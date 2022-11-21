/**
 * Manchester En- and decoding in firmware using an ATmega
 */
#include "manchester.h"
#include "io_manipulation.h"
#include <avr/interrupt.h>

/**
 * Initialize everything, that we can run the Manchester Encoding and Decoding
 */
void manchester_init(unsigned long datarate){

    // initialize the output pin
    SET_OUTPUT(MAN_TX_PIN);
    RESET(MAN_TX_PIN);      // switch pin OFF

    // initialize the input pin
    SET_INPUT(MAN_RX_PIN);

    // debug outputs
    SET_OUTPUT(MAN_DBG_PIN_CLK);
    SET(MAN_DBG_PIN_CLK);      // switch pin ON

    SET_OUTPUT(MAN_DBG_PIN_TRG);
    RESET(MAN_DBG_PIN_TRG);

    TCCR2A = (1<<WGM21);  // Wave Form Generation Mode 2: CTC, OC2A disconnected
    TCCR2B = (1<<CS20) ;  // prescaler = 1
    TIMSK2 = (1<<OCIE2A); // interrupt when Compare Match with OCR2A
    OCR2A = 160;
    // this should result in something of about 100kHz

    man_TXbitphase = 0;
    man_TXbitcnt = 0;
    man_TXbyte = 0;
    man_TXbusy = 0;        // we are not busy sending a byte
}

/**
 * write a single character via the Manchester encoding output
 */
void manchester_write_char(char c){
    // wait until the last byte got sent out
    while(man_TXbusy){
    }
    man_TXbyte = c;        // put the new byte into the TX chain
    man_TXbusy = 1;        // indicate that we want to transmit a new byte
    SET(MAN_DBG_PIN_TRG);
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

ISR(TIMER2_COMPA_vect){
    // for the Manchester coding for each single bit in the TX character
    // two bits needs to be send out (1b2b encoding)

    TOGGLE(MAN_DBG_PIN_CLK);

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
        // set the proper output
        manchester_setOutputPin(man_TXbit, man_TXbitphase);

    // second half of the TX bit
    }else{
        man_TXbitphase = 0;
        // set the proper output
        manchester_setOutputPin(man_TXbit, man_TXbitphase);

        // if we have all bits transmitted, we are done and ready for the next
        // byte
        if(man_TXbitcnt > 7){
            man_TXbusy = 0;     // we are done with transmission
            man_TXbitcnt = 0;   // reset the bit counter
            RESET(MAN_DBG_PIN_TRG);
        }
    }
}
