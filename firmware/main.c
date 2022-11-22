/**
 * Test implementation of software Manchester encoding and decoding
 * @author JochiSt
 *
 */

#include "usart.h"
#include "config.h"
#include "manchester.h"
#include "io_manipulation.h"

#include <avr/interrupt.h>
#include <util/delay.h>

void setup(void){
    SET_OUTPUT(LED);
    SET(LED);

    SET_OUTPUT(MAN_DBG_PIN_TRG);
    RESET(MAN_DBG_PIN_TRG);

    usart_init(BAUDRATE);
    usart_write("Compiliert at "__DATE__" - "__TIME__"\r\n");
	usart_write("Compiliert with GCC Version "__VERSION__"\r\n");

    _delay_ms(500);

    manchester_init(0);

    // enable global interrupts
    sei();
}

int main(void){
	setup();
    _delay_ms(50);  // wait some time for getting everything setup

    // wait until we are synced
    while(!man_RX_synced){
        _delay_us(500);
    }
    // turn off the LED once we are synced
    RESET(LED);
    usart_write("Manchester Synced\n");

	// Main program loop
	while(1){
        SET(MAN_DBG_PIN_TRG);
        manchester_write_char(MAN_START_PATTERN);
        manchester_write_char('H'); //  0
        manchester_write_char('E'); //  1
        manchester_write_char('L'); //  2
        manchester_write_char('L'); //  3
        manchester_write_char('O'); //  4
        manchester_write_char(' '); //  5
        manchester_write_char('W'); //  6
        manchester_write_char('O'); //  7
        manchester_write_char('R'); //  8
        manchester_write_char('L'); //  9
        manchester_write_char('D'); // 10
        RESET(MAN_DBG_PIN_TRG);

        //usart_write_str(man_RX_buffer);
        usart_write_str(man_RX_buffer);
        usart_write_str("\n");

        //_delay_us(500);
        _delay_ms(5000);
	}
}
