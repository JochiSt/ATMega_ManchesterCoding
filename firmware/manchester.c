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

    // initialize the RX variables
    man_RXbitcnt = 0;
    man_RXbitphase = 0;
    man_RX_synced = 0;
    man_RX_sync_cnt = 0;
    man_RX_bit0 = 0;
    man_RX_bit1 = 0;
    man_RX_buffer = 0;

    // initialize the TX variables
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

    // first half of the RX bit
    if(!man_RXbitphase){
        man_RXbitphase = 1;
        man_RX_bit0 = IS_SET(MAN_RX_PIN);
        // evaluate the received pattern
        // 1
        if(!man_RX_bit0 && man_RX_bit1) {
            man_RX_sync_cnt = 0;


            man_RX_buffer = (man_RX_buffer >> 1);
            man_RX_buffer|= (1) << 7;
            man_RXbitcnt++;

            SET(MAN_DBG_PIN_RX);

        // 0
        } else if(man_RX_bit0 && !man_RX_bit1) {
            man_RX_sync_cnt ++;

            man_RX_buffer = (man_RX_buffer >> 1);
            man_RX_buffer|= (0) << 7;
            man_RXbitcnt++;

            RESET(MAN_DBG_PIN_RX);
        }

        // force FAKE sync
        man_RX_synced = 1;

        // limit RX bit counter to 0-7
        if( man_RXbitcnt > 7) {
            man_RXbitcnt = 0;
        }
/*
        // do we have detected the start pattern
        if (man_RX_buffer == MAN_START_PATTERN){
            SET(MAN_DBG_PIN_RX);
        }else{
            RESET(MAN_DBG_PIN_RX);
        }
*/
        /*
        if (man_RX_synced){
            // adjust our bit counter to match the start pattern
            if (man_RX_buffer == MAN_START_PATTERN){
                man_RXbitcnt = 8;
            }
            if( man_RXbitcnt > 8){
                usart_write_char(man_RX_buffer);
                man_RXbitcnt = 0;
            }
        }
        */
    // second half of the RX bit
    }else{
        man_RXbitphase = 0;
        man_RX_bit1 = IS_SET(MAN_RX_PIN);
    }

    ////////////////////////////////////////////////////////////////////////////
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
        }
    }
}
