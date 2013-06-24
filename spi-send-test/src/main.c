/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/crc16.h>
//#include <util/setbaud.h>
//#include <avr/uart.h>

#include "dallas_one_wire.h"

#define SPI_SS_PIN PB0
#define SPI_SCK_PIN PB1
#define SPI_MOSI_PIN PB2
#define SPI_MISO_PIN PB3

#define BUF_VELOCITY 0
#define BUF_RPM 1
#define BUF_ENGINE_TEMP 2
#define BUF_GPIO 3
#define  GPIO_LEFT_TURN 0
#define  GPIO_RIGHT_TURN 1
#define  GPIO_FRONT_BRAKE 2
#define  GPIO_REAR_BRAKE 3
#define  GPIO_PARKING_LIGHT 4
#define BUF_AIR_TEMP_LSB 4
#define BUF_AIR_TEMP_MSB 5

#define BUFLEN 6
#define ECU_FRAME_SIZE 6
#define ECU_BYTE_DELAY_CANCEL_MS 6
#define TEMP_READ_TOMEOUT_MS 60000

#define ECU_RPM 1
#define ECU_VELOCITY 2
#define ECU_ENGINE_TEMP 4

#define USART_BAUDRATE 16064
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

/// Filled with debug data.
volatile uint8_t buffer[BUFLEN] = {0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF};

///**
// * Buffer number we are currently wrinting the data to. The other buffer contains previous complete snapshot,
// * thus it is always ready to send over SPI to the host. Current buffer may be incomplete at this time, but
// * once it's filled, currentBuffer is swapped making it available for sending.
// */
//volatile uint8_t currentBuffer = 0;

volatile uint16_t currentTime = 0;
volatile uint16_t lastTempReadTime = (uint16_t)-TEMP_READ_TOMEOUT_MS;

/**
 *
 */
void initSpi (void)
{
        // Configure port B for SPI slave.
        DDRB &= ~(1 << SPI_MOSI_PIN);   // input
        DDRB |= (1 << SPI_MISO_PIN);    // output
        DDRB &= ~(1 << SPI_SS_PIN);     // input
        DDRB &= ~(1 << SPI_SCK_PIN);    // input

        SPCR = (0 << SPIE)      // interrupt enabled / disapbled
             | (1 << SPE)       // enable SPI
             | (0 << DORD)      // MSB
             | (0 << MSTR)      // Slave
             | (0 << CPOL)      // clock timing mode CPOL. Set to 0 - mode 0
             | (0 << CPHA)      // clock timing mode CPHA. Set to 0 - mode 0
             | (0 << SPR1)      // SPR1 and SPR0 set to 0 means SCK frequency == fosc/4,
             | (0 << SPR0);     // but since we are SPI slave, this takes no effect.
}

/**
 * Transmission only.
 */
void initUSART (void)
{
        UCSR1B |= (1 << RXEN1);
        UCSR1C |= (1 << UCSZ11) | (1 << UCSZ10);
        UBRR1H = (BAUD_PRESCALE >> 8);
        UBRR1L = BAUD_PRESCALE;
}

/**
 * Timer at full speed, increases halfMs variable every half ms.
 */
void initTimer (void)
{
        TCCR1B |= (1 << WGM12) ; // Configure timer 1 for CTC mode
        TIMSK1 |= (1 << OCIE1A) ; // Enable CTC interrupt
        OCR1A = F_CPU / 1000 - 1; // 1 ms CTC.
        TCCR1B |= (1 << CS10); // Counter running at FCPU speed.
}

/**
 *
 */
void storeTempInBuffer (void)
{
        uint8_t crc, i;
        uint8_t tempBuffer[9];

        dallas_skip_rom ();
        dallas_write_byte (0x44);
        dallas_drive_bus();
        _delay_ms(800);

        do {
                dallas_skip_rom ();
                dallas_write_byte(0xBE);
                dallas_read_buffer((uint8_t *) &tempBuffer, 9);
                dallas_reset();

                // Checking the CRC...

                crc = 0x00;

                for (i = 0; i < 9; i++) {
                        crc = _crc_ibutton_update(crc, tempBuffer[i]);
                }

        } while (crc != 0x00);

        buffer[BUF_AIR_TEMP_LSB] = tempBuffer[0];
        buffer[BUF_AIR_TEMP_MSB] = tempBuffer[1];
}

/**
 * Poll for 6 bytes from the ECU.
 */
void checkECU (void)
{
        /// Data from the ECU
        uint8_t ecuFrame[ECU_FRAME_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t frameComplete = 0;
        while (!frameComplete) {

                uint8_t i;
                for (i = 0; i < ECU_FRAME_SIZE; ++i) {
                        uint16_t timestampMs = currentTime;

                        while ((UCSR1A & (1 << RXC1)) == 0)
                                ;

                        // If reading took too long, skip this frame. It meant, we started from the middle of the frame.
                        if ((uint16_t)(currentTime - timestampMs) >= ECU_BYTE_DELAY_CANCEL_MS && i > 0) {
                                break;
                        }

                        ecuFrame[i] = UDR1;
                }

                if (i >= ECU_FRAME_SIZE) {
                        frameComplete = 1;
                }
        }

        buffer[BUF_RPM] = ecuFrame[ECU_RPM];
        buffer[BUF_VELOCITY] = ecuFrame[ECU_VELOCITY];
        buffer[BUF_ENGINE_TEMP] = ecuFrame[ECU_ENGINE_TEMP];
}

/**
 *
 */
void checkTemp (void)
{
        // Every minute
        if ((uint16_t)(currentTime - lastTempReadTime) >= TEMP_READ_TOMEOUT_MS) {
                storeTempInBuffer ();
                lastTempReadTime = currentTime;
        }
}

/**
 *
 */
void checkGPIO (void)
{

}

/**
 * Send buffer to SPI master (raspberry).
 */
void sendToHost (void)
{
        SPDR = 0x00; // Marker byte.

        for (uint8_t i = 0; i < BUFLEN; ++i) {
                while (!(SPSR & (1 << SPIF)))
                        ;

                SPDR = buffer[i];
        }

        while (!(SPSR & (1 << SPIF)))
                ;
}

/**
 *
 */
int main (void)
{
        initSpi ();
        initUSART ();
        initTimer ();
        sei () ;

        while (1) {
//                fillCurrentBuffer ();
                checkECU ();
                checkTemp ();
                checkGPIO ();
                sendToHost ();
        }


        return 0;
}

/**
 * Called when byte from master is received (and since this is full duplex transmission)
 *  AND buffer[currentBuffer ^ 1] transmitted to the master. I set the next byte to transmit.
 */
//ISR (SPI_STC_vect, ISR_BLOCK)
//{
//        transmissionInProgress = 1;
//
//        if (currentByte >= BUFLEN) {
//                SPDR = buffer[0];
//                currentByte = 1;
//                transmissionInProgress = 0;
//        }
//        else {
//                SPDR = buffer[currentByte++];
//        }
//}

ISR (TIMER1_COMPA_vect, ISR_BLOCK)
{
        ++currentTime;
}
