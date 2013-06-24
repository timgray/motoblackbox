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

#define USART_BAUDRATE 16064
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

/**
 * Transmission only.
 */
void initUSART (void)
{
        UCSR1B |= (1 << TXEN1) | (1 << RXEN1);
        UCSR1C |= (1 << UCSZ11) | (1 << UCSZ10);
        UBRR1H = (BAUD_PRESCALE >> 8);
        UBRR1L = BAUD_PRESCALE;
}

void sendByteEcu (uint8_t byte)
{
        while ((UCSR1A & (1 << UDRE1)) == 0)
                ; // Do nothing until UDR is ready for more data to be written to it

        UDR1 = byte;
}

/**
 *
 */
int main (void)
{
        initUSART ();
        DDRB = 1 << PB0;

        while (1) {
                sendByteEcu(0x01);
                _delay_ms (2);
                sendByteEcu(0x56);
                _delay_ms (1);
                sendByteEcu(0x05);
                _delay_ms (1);
                sendByteEcu(0x00);
                _delay_ms (1);
                sendByteEcu(0x77);
                _delay_ms (1);
                sendByteEcu(0xd2);
                _delay_ms (8);
                PORTB ^= 1 << PB0;
        }
}
