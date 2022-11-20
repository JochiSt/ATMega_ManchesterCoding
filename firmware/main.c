/**
 * Test implementation of software Manchester encoding and decoding
 * @author JochiSt
 *
 */

#include "usart.h"
#include "config.h"

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

	}
}
