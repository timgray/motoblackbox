/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

/*
 * Flash with:
 * avrdude -p m2560 -c stk500v2 -P /dev/ttyACM0 -b 115200 -F -U flash:w:moto-shield.hex
 * TODO :
 * - #command byte
 * - #checksum
 * - #GPIO
 * - #?temperature with ADC
 * - #velocity sum.
 */
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/crc16.h>

#define BUF_VELOCITY_MSB 0
#define BUF_VELOCITY_LSB 1
#define BUF_RPM 2
#define BUF_ENGINE_TEMP 3
#define BUF_GPIO 4
#define  GPIO_LEFT_TURN_BIT 0
#define  GPIO_RIGHT_TURN_BIT 1
#define  GPIO_FRONT_BRAKE_BIT 2
#define  GPIO_REAR_BRAKE_BIT 3
#define  GPIO_PARKING_LIGHT_BIT 4
#define BUF_AIR_TEMP 5

#define DDR_GPIO DDRA
#define PIN_GPIO PINA
#define GPIO_LEFT_TURN_PIN PA0
#define GPIO_RIGHT_TURN_PIN PA1
#define GPIO_FRONT_BRAKE_PIN PA2
#define GPIO_REAR_BRAKE_PIN PA3
#define GPIO_PARKING_LIGHT_PIN PA4

#define BUFLEN 6
#define ECU_FRAME_SIZE 6
#define ECU_BYTE_DELAY_CANCEL 5
#define VELOCITY_TIMEOUT 125
#define ECU_COMMAND_BYTE 0x01
#define SHIELD_COMMAND_BYTE 0x01

typedef uint8_t EcuFrameBuffer[ECU_FRAME_SIZE];

struct EcuFrame_t {
        EcuFrameBuffer buffer;
        uint8_t start;
        uint8_t size;
};

typedef struct EcuFrame_t EcuFrame;

#define ECU_RPM 1
#define ECU_VELOCITY 2
#define ECU_ENGINE_TEMP 4

#define USART_BAUDRATE_ECU 16064
#define BAUD_PRESCALE_ECU (((F_CPU / (USART_BAUDRATE_ECU * 16UL))) - 1)

#define USART_BAUDRATE_RASPI 38400
#define BAUD_PRESCALE_RASPI (((F_CPU / (USART_BAUDRATE_RASPI * 16UL))) - 1)

/// Filled with debug data.
volatile uint8_t buffer[BUFLEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
volatile uint16_t currentTime = 0;
volatile uint16_t lastEcuReadTime = 0;
volatile uint8_t waitForEcu = 1;
volatile uint16_t velocityTime = 0;
volatile uint16_t velocitySum = 0;

/**
 * Receive only.
 */
void initUSARTRaspi (void)
{
        UCSR2B |= (1 << TXEN2); // only transmission
        UCSR2C |= (1 << UCSZ21) | (1 << UCSZ20); // standard config : 8bits
        UBRR2H = (BAUD_PRESCALE_RASPI >> 8);
        UBRR2L = BAUD_PRESCALE_RASPI;
}


/**
 * Transmission only.
 */
void initUSARTEcu (void)
{
//        UCSR1B |= (1 << RXEN1) | (1 << RXCIE1); // Enable data reception and interrupt.
        UCSR1B |= (1 << RXEN1); // Enable data reception
        UCSR1C |= (1 << UCSZ11) | (1 << UCSZ10);
        UBRR1H = (BAUD_PRESCALE_ECU >> 8);
        UBRR1L = BAUD_PRESCALE_ECU;
}

/**
 * Timer at full speed, CTC mode.
 */
void initTimer (void)
{
        TCCR3B |= (1 << WGM32) ; // Configure timer 1 for CTC mode
        TIMSK3 |= (1 << OCIE3A) ; // Enable CTC interrupt
        OCR3A = F_CPU / 250 - 1; // (250 times per second) = 4 ms when @16MHz, CTC.
        TCCR3B |= (1 << CS30); // Counter running at FCPU speed.
}

void initInitGPIO (void)
{
        // All as input
        DDR_GPIO = 0x00;

        // Debug outputs for analyzer.
        DDRE = (1 << PE4) | (1 << PE5);
}

/**
 *
 */
void initADC (void)
{
        // Free running mode - this line is of cource not necessary, but i put it for the sake of completness.
        // No MUX values needed to be changed to use ADC0
        ADCSRB &= ~(1 << ADTS2) | ~(1 << ADTS1) | ~(1 << ADTS0);

        // Reference Selection : AVCC with external capacitor at AREF pin
        ADMUX |= (1 << REFS0);
        ADMUX |= (1 << ADLAR); // Left adjust ADC result to allow easy 8 bit reading

        ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // ADC clock signal prescaler. Divide by 128.
        ADCSRA |= (1 << ADEN); // Enable ADC
        ADCSRA |= (1 << ADSC); // Start A2D Conversions
}

/**
 * Wait for 1 byte from UART1.
 */
uint8_t poll1 (void)
{
        // Debug output.
        PORTE |= 1 << PE4;

        ATOMIC_BLOCK (ATOMIC_RESTORESTATE) {
                lastEcuReadTime = currentTime;
                waitForEcu = 1;
        }

        while ((UCSR1A & (1 << RXC1)) == 0 && waitForEcu)
                ;

        // Debug output.
        PORTE &= ~(1 << PE4);

        if (!waitForEcu) {
                return 0x00;
        }

        return UDR1;
}

/**
 * Ecu frames from mitsubishi ECU computer have a checksum at the end, and controll byte at the
 * beginnig (0x01 in my case). Thus checking them for validity is to check first byte and if
 * it's correct, check the checksum of entire frame.
 */
uint8_t validateECUFrame (EcuFrame *ecuFrame)
{
        uint8_t start = ecuFrame->start;
        PORTE &= 1 << PE5;

        if (ecuFrame->buffer[start] != ECU_COMMAND_BYTE) {
                return 0;
        }

        PORTE ^= 1 << PE5;
        uint8_t checksum = ecuFrame->buffer[(start + 1) % ECU_FRAME_SIZE] +
                           ecuFrame->buffer[(start + 2) % ECU_FRAME_SIZE] +
                           ecuFrame->buffer[(start + 3) % ECU_FRAME_SIZE] +
                           ecuFrame->buffer[(start + 4) % ECU_FRAME_SIZE];

        if (ecuFrame->buffer[(start + 5) % ECU_FRAME_SIZE] != checksum) {
                return 0;
        }

        return 1;
}

/**
 * Wait for ECU_COMMAND_BYTE, and then fill frame with 5 following consecutive bytes thus fillig
 * the buffer completely.
 */
void fillFrame (EcuFrame *ecuFrame)
{
        // Wit for command byte to appear (value 0x01).
        while (poll1 () != ECU_COMMAND_BYTE && waitForEcu)
                ;

        if (!waitForEcu) {
                return;
        }

        // Store it in the buffer. It may be some other 0x01 than the command.
        ecuFrame->buffer[0] = ECU_COMMAND_BYTE;

        for (uint8_t i = 1; i < ECU_FRAME_SIZE && waitForEcu; ++i) {
                ecuFrame->buffer[i] = poll1 ();
        }

        if (!waitForEcu) {
                return;
        }

        ecuFrame->size = ECU_FRAME_SIZE;
        ecuFrame->start = 0;
}

/**
 * Read next byte (only one) and rotate the buffer.
 */
void rotateFrame1 (EcuFrame *ecuFrame)
{
//        PORTE &= 1 << PE5;
        if (ecuFrame->size < ECU_FRAME_SIZE) {
                return;
        }

        // Overwrite the oldest byte (starting byte)
        ecuFrame->buffer[ecuFrame->start] = poll1 ();
        ecuFrame->start = (ecuFrame->start + 1) %  ECU_FRAME_SIZE;
//        PORTE ^= 1 << PE5;
}

/**
 * Store received ecu frame in the output buffer.
 */
void commitEcuData (EcuFrame *ecuFrame)
{
        velocitySum += ecuFrame->buffer[(ECU_VELOCITY + ecuFrame->start) % ECU_FRAME_SIZE];
        buffer[BUF_RPM] = ecuFrame->buffer[(ECU_RPM + ecuFrame->start) % ECU_FRAME_SIZE];
        buffer[BUF_ENGINE_TEMP] = ecuFrame->buffer[(ECU_ENGINE_TEMP + ecuFrame->start) % ECU_FRAME_SIZE];
}

/**
 * One frame.
 */
void receiveECUFrame (void)
{
        EcuFrame ecuFrame;
        fillFrame (&ecuFrame);

        if (!waitForEcu) {
                return;
        }

        if (validateECUFrame (&ecuFrame)) {
                commitEcuData (&ecuFrame);
                return;
        }

        do {
                rotateFrame1 (&ecuFrame);
        } while (!validateECUFrame (&ecuFrame) && waitForEcu);

        commitEcuData (&ecuFrame);
}

/**
 * Transmit a byte thru USART2.
 */
void transmit2 (uint8_t byte)
{
        while ((UCSR2A & (1 << UDRE2)) == 0)
                ; // Do nothing until UDR is ready for more data to be written to it

        UDR2 = byte;
}

/**
 * Send buffer to SPI master (raspberry).
 */
void sendToHost (void)
{
        // Send head byte
        transmit2 (SHIELD_COMMAND_BYTE);

        for (uint8_t i = 0; i < BUFLEN; ++i) {
                transmit2(buffer[i]);
        }

        // Comune and send the checksum
        uint8_t checksum = 0;
        for (uint8_t i = 0; i < BUFLEN; ++i) {
                checksum += buffer[i];
        }

        transmit2 (checksum);
}

/**
 *
 */
void checkTemp (void)
{
        buffer[BUF_AIR_TEMP] = ADCH;
}

/**
 *
 */
void checkGPIO (void)
{
        uint8_t state = 0x00;

        state |= (PIN_GPIO & (1 << GPIO_FRONT_BRAKE_PIN)) << GPIO_FRONT_BRAKE_BIT;
        state |= (PIN_GPIO & (1 << GPIO_REAR_BRAKE_PIN)) << GPIO_REAR_BRAKE_BIT;
        state |= (PIN_GPIO & (1 << GPIO_RIGHT_TURN_PIN)) << GPIO_RIGHT_TURN_BIT;
        state |= (PIN_GPIO & (1 << GPIO_LEFT_TURN_PIN)) << GPIO_LEFT_TURN_BIT;
        state |= (PIN_GPIO & (1 << GPIO_PARKING_LIGHT_PIN)) << GPIO_PARKING_LIGHT_BIT;

        buffer[BUF_GPIO] = state;
}

/**
 *
 */
int main (void)
{
        initUSARTRaspi ();
        initUSARTEcu ();
        initTimer ();
        initInitGPIO ();

        sei () ;

        while (1) {
//                PORTE ^= (1 << PE5);
                receiveECUFrame ();
                checkTemp ();
                checkGPIO ();
                sendToHost ();
//                _delay_ms (50);
        }


        return 0;
}

/**
 *
 */
ISR (TIMER3_COMPA_vect, ISR_BLOCK)
{
        ++currentTime;

        uint16_t interval = (uint16_t)(currentTime - lastEcuReadTime);

        if (interval >= ECU_BYTE_DELAY_CANCEL) {
                waitForEcu = 0;
        }

        if (++velocityTime > VELOCITY_TIMEOUT) {
                buffer[BUF_VELOCITY_MSB] = velocitySum >> 8;
                buffer[BUF_VELOCITY_LSB] = velocitySum;
                velocityTime = velocitySum = 0;
        }
}
