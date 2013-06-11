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

#define USART_BAUDRATE 16064
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

/// Double buffered. Filled with debug data.
volatile uint8_t buffer[2][BUFLEN] = { {0x01, 0x02, 0x03, 0x04, 0x05, 0x06}, {0x11, 0x12, 0x13, 0x14, 0x15, 0x16} };

/**
 * Buffer number we are currently wrinting the data to. The other buffer contains previous complete snapshot,
 * thus it is always ready to send over SPI to the host. Current buffer may be incomplete at this time, but
 * once it's filled, currentBuffer is swapped making it available for sending.
 */
volatile uint8_t currentBuffer = 0;

/**
 *
 */
volatile uint8_t currentByte = 0;


volatile uint8_t transmissionInProgress = 0;

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

        SPCR = (1 << SPIE)      // interrupt enabled
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

void swapBuffers (void)
{
        // If transmission is happening right now, do not swap buffers.
        while (transmissionInProgress);

        ATOMIC_BLOCK (ATOMIC_RESTORESTATE)
        {
                currentBuffer ^= 1;
                SPDR = buffer[currentBuffer ^ 1][0];
                currentByte = 1;
        }
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

        buffer[currentBuffer][BUF_AIR_TEMP_LSB] = tempBuffer[0];
        buffer[currentBuffer][BUF_AIR_TEMP_MSB] = tempBuffer[1];
        buffer[currentBuffer ^ 1][BUF_AIR_TEMP_LSB] = tempBuffer[0];
        buffer[currentBuffer ^ 1][BUF_AIR_TEMP_MSB] = tempBuffer[1];
}

/**
 * Data acquisition here. This function will block until it collects
 * entire buffer and store it in the 'currentBuffer'. At the end it swaps
 * currentBuffer value thus fresh buffer is available to send.
 */
void fillCurrentBuffer (void)
{
        static uint8_t fakeTemp = 0x00;
        buffer[currentBuffer][BUF_GPIO] = fakeTemp++;
        _delay_ms (50);
//        storeTempInBuffer ();


        while ((UCSR1A & (1 << RXC1)) == 0)
                ;

        uint8_t data = UDR1;
        buffer[currentBuffer][BUF_ENGINE_TEMP] = data;

        swapBuffers ();
}

/**
 *
 */
int main (void)
{
        initSpi ();
        initUSART ();
        storeTempInBuffer ();
        sei ();

        while (1) {
                fillCurrentBuffer ();
        }


        return 0;
}

/**
 * Called when byte from master is received (and since this is full duplex transmission)
 * AND buffer[currentBuffer ^ 1] transmitted to the master. I set the next byte to transmit.
 */
ISR (SPI_STC_vect, ISR_BLOCK)
{
        transmissionInProgress = 1;

        if (currentByte >= BUFLEN) {
                SPDR = buffer[currentBuffer ^ 1][0];
                currentByte = 1;
                transmissionInProgress = 0;
        }
        else {
                SPDR = buffer[currentBuffer ^ 1][currentByte++];
        }
}
