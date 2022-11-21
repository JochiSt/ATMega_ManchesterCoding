/**
 * Test implementation of software Manchester encoding and decoding
 * @author JochiSt
 *
 */

#include "usart.h"
#include "config.h"
#include "manchester.h"

#include <avr/interrupt.h>
#include <util/delay.h>

void setup(){
    usart_init(BAUDRATE);
    manchester_init();

    // enable global interrupts
    sei();
}

int main(){
	setup();

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
