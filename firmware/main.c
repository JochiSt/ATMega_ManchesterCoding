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

    manchester_init();

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

	// Main program loop
	while(1){
        //usart_write("test\r\n");
        SET(MAN_DBG_PIN_TRG);
        manchester_write_char(MAN_START_PATTERN);
        //manchester_write_char('U');
        //manchester_write_char('F');
        RESET(MAN_DBG_PIN_TRG);

        //_delay_us(500);
        _delay_ms(5000);
	}
}
