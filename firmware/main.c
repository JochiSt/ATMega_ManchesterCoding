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
        usart_write("test\r\n");
        manchester_write_char('U');
    	_delay_ms(1000);
	}
}
