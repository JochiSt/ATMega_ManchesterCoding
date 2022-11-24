/**
 * @brief Test implementation of software Manchester encoding and decoding
 * @author JochiSt
 */

#include "usart.h"
#include "config.h"
#include "manchester.h"
#include "io_manipulation.h"

#include <avr/interrupt.h>
#include <util/delay.h>

/// Pin, which indicates a new transmission
#define MAN_DBG_PIN_TRG  D,7

void setup(void){
    SET_OUTPUT(LED);
    SET(LED);

    SET_OUTPUT(MAN_DBG_PIN_TRG);
    RESET(MAN_DBG_PIN_TRG);

    usart_init(BAUDRATE);
    usart_write("Compiliert at "__DATE__" - "__TIME__"\n");
	usart_write("Compiliert with GCC Version "__VERSION__"\n");

    //manchester_init(100UL);
    manchester_init(100000UL);

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
        manchester_TX_str("HELLO WORLD");
        RESET(MAN_DBG_PIN_TRG);

        if(man_RX_ready){
            man_RX_ready = 0;
            usart_write_str(man_RX_buffer);
            usart_write_str("\n");
        }

        _delay_us(500);
	}
}
