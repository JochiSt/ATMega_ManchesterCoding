/**
 * Manchester En- and decoding in firmware using an ATmega
 */
#include "manchester.h"
#include "io_manipulation.h"
#include <avr/interrupt.h>

/**
 * Initialise everything, that we can run the Manchester Encoding and Decoding
 */
void manchester_init(unsigned long datarate){

    // initialise the output pin
    SET_OUTPUT(MAN_TX_PIN);
    RESET(MAN_TX_PIN);      // switch pin OFF

    // configure the timer
    TCCRA =  (1 << WGM01); // CTC mode
    TCCRB |= (1 << CS01);  // prescaler (here 8)
    OCRA  = 125-1;         // set the overflow value

    // enable timer interrupt
    IRMSK |= (1<<OCIE0A);

    man_TXbitphase = 0;
    man_TXbitcnt = 0;
    man_TXbyte = 0;
    man_TXbusy = 0;        // we are not busy sending a byte
}

void manchester_write_char(char c){
    // wait until the last byte got sent out
    while(man_TXbusy){
    }
    man_TXbyte = c;        // put the new byte into the TX chain
    man_TXbusy = 1;
}

/**
 * set the pin output. In this function the 0 and 1 of the txbit are translated
 * to 01 and 10 at the output.
 * @param txbit value of the bit to be transmitted (0/1)
 * @param phase phase of output transmission (0/1)
 *
 */
void manchester_setOutputPin(unsigned char txbit, unsigned char phase) {
    if(txbit) { // txbit = 0
        if(phase){
            // output 1
            SET(MAN_TX_PIN);
        }else{
            // output 0
            RESET(MAN_TX_PIN);
        }
    } else {    // txbit = 0
        if(phase){
            // output 0
            RESET(MAN_TX_PIN);
        }else{
            // output 1
            SET(MAN_TX_PIN);
        }
    }
}

ISR(TIMER0_COMPA_vect){
    // for the Manchester coding for each single bit in the TX character
    // two bits needs to be send out (1b2b encoding)

    // first half of the TX bit
    if(!man_TXbitphase) {
        man_TXbitphase = 1;

        // get a new bit for transmission
        if(man_TXbusy) {
            man_TXbit = ( man_TXbyte & (1<< man_TXbitcnt) ) >> man_TXbitcnt;
            man_TXbitcnt ++;
        } else {
            // if we are no longer busy, continuously transmit zeros
            man_TXbit = 0;
        }
        manchester_setOutputPin(man_TXbit, man_TXbitphase);

    // second half of the TX bit
    }else{
        man_TXbitphase = 0;
        manchester_setOutputPin(man_TXbit, man_TXbitphase);

        // if we have all bits transmitted, we are done and ready for the next
        // byte
        if(man_TXbitcnt > 7){
            man_TXbusy = 0;
            man_TXbitcnt = 0;
        }
    }
}
