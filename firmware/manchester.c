/**
 * Manchester En- and decoding in firmware using an ATmega
 */

#include "manchester.h"
#include "io_manipulation.h"
#include <avr/interrupt.h>

/**
 * Initialize everything, that we can run the Manchester Encoding and Decoding
 * @param datarate Rate of the 1B2B datastream
 * TODO implement calculation of timer settings from function parameter
 */
void manchester_init(unsigned long datarate){

    // initialize the output pin
    SET_OUTPUT(MAN_TX_PIN);
    RESET(MAN_TX_PIN);      // switch pin OFF

    // initialize the input pin
    SET_INPUT(MAN_RX_PIN);

    // init the debug clock output
    SET_OUTPUT(MAN_DBG_PIN_CLK);
    RESET(MAN_DBG_PIN_CLK);

    uint8_t prescaler_cnt = 0;
    uint8_t prescalers[7] = {1, 8, 32, 64, 128, 256, 1024};

    uint32_t f_cpu = F_CPU;

    while( f_cpu / datarate > 250){
        prescaler_cnt++;
        f_cpu = F_CPU / prescalers[prescaler_cnt];
    }

    // setting up the timer interrupt
    TCCR2A = (1<<WGM21);  // Wave Form Generation Mode 2: CTC, OC2A disconnected
    if(prescalers[prescaler_cnt] == 1){
        TCCR2B = (0<<CS22)|(0<<CS21)|(1<<CS20);
    }else if(prescalers[prescaler_cnt] == 8){
        TCCR2B = (0<<CS22)|(1<<CS21)|(0<<CS20);
    }else if(prescalers[prescaler_cnt] == 32){
        TCCR2B = (0<<CS22)|(1<<CS21)|(1<<CS20);
    }else if(prescalers[prescaler_cnt] == 64){
        TCCR2B = (1<<CS22)|(0<<CS21)|(0<<CS20);
    }else if(prescalers[prescaler_cnt] == 128){
        TCCR2B = (1<<CS22)|(1<<CS21)|(1<<CS20);
    }else if(prescalers[prescaler_cnt] == 256){
        TCCR2B = (1<<CS22)|(1<<CS21)|(0<<CS20);
    }else if(prescalers[prescaler_cnt] == 1024){
        TCCR2B = (1<<CS22)|(1<<CS21)|(1<<CS20);
    }
    OCR2A = f_cpu/datarate;

    TIMSK2 = (1<<OCIE2A); // interrupt when Compare Match with OCR2A


    // initialize the RX variables
    man_RXbitcnt = 0;
    man_RXbitphase = 0;
    man_RX_synced = 0;
    man_RX_sync_cnt = 0;
    man_RX_bit0 = 0;
    man_RX_bit1 = 0;
    man_RX_bitbuffer = 0;
    man_RX_buffercounter = 0;
    man_RX_ready = 0;

    // initialize the TX variables
    man_TXbitphase = 0;
    man_TXbitcnt = 0;
    man_TXbyte = 0;
    man_TXbusy = 0;        // we are not busy sending a byte
}

/**
 * write a single character via the Manchester encoding output
 * @param c character to be written via the encoding
 */
void manchester_write_char(char c){
    // wait until the last byte got sent out
    while(man_TXbusy){
    }
    man_TXbyte = c;        // put the new byte into the TX chain
    man_TXbusy = 1;        // indicate that we want to transmit a new byte
}

/**
 * write a string via Manchester Coding
 * @param str string, which should be sent
 */
void manchester_write_str(char *str){
	while (*str){
		manchester_write_char(*str++);
	}
}

void manchester_TX_str(char* str){
    manchester_write_char(MAN_START_PATTERN);
    manchester_write_str(str);
    manchester_write_char(MAN_STOP_PATTERN);
}

/**
 * reset the synchronization and by this force resynchronization
 */
void manchester_resetSynchronization(void){
    man_RX_sync_cnt = 0;
    man_RX_synced = 0;
}

/**
 * set the pin output. In this function the 0 and 1 of the txbit are translated
 * to 01 and 10 at the output.
 * @param txbit value of the bit to be transmitted (0/1)
 * @param phase phase of output transmission (0/1)
 *
 */
void manchester_setOutputPin(unsigned char txbit, unsigned char phase) {
    if(txbit) { // txbit = 1
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

/******************************************************************************/
    // first half of the RX bit
    if(!man_RXbitphase){
        man_RXbitphase = 1;
        man_RX_bit0 = IS_SET(MAN_RX_PIN);

        ////////////////////////////////////////////////////////////////////////
        // evaluate the received pattern
        // a new bit has being received
        man_RX_bitbuffer = (man_RX_bitbuffer >> 1);
        man_RXbitcnt ++;

        // RX bit = 1 (01)
        if(!man_RX_bit0 && man_RX_bit1) {
            // if we have received a one, it's no longer part of the sync
            // sequence -> the sync counter is reset
            man_RX_sync_cnt = 0;

            man_RX_bitbuffer |= (1 << 7);
        // RX bit = 0 (10)
        } else if(man_RX_bit0 && !man_RX_bit1) {
            man_RX_bitbuffer |= (0 << 7);
            man_RX_sync_cnt++;
        }

        ////////////////////////////////////////////////////////////////////////
        // evaluate the synchronization counter
        if(man_RX_sync_cnt > 10){
            man_RX_synced = 1;
        }

        ////////////////////////////////////////////////////////////////////////
        // limit RX bit counter to 0-7
        if( man_RXbitcnt > 7) {
            man_RXbitcnt = 0;

            // we have received a byte / char
            if (man_RX_buffercounter < MAN_RX_BUFFER_SIZE - 1){
                man_RX_buffer[man_RX_buffercounter++] = man_RX_bitbuffer;
            }
            if( man_RX_bitbuffer == MAN_STOP_PATTERN){
                man_RX_ready = 1;
            }
        }

        ////////////////////////////////////////////////////////////////////////
        // do we have detected the start pattern
        // if so, we reset the bit counter to be in phase with the received data
        if (man_RX_bitbuffer == MAN_START_PATTERN){
            man_RXbitcnt = 0;
            man_RX_buffercounter = 0;
        }

    // second half of the RX bit
    }else{
        man_RXbitphase = 0;
        man_RX_bit1 = IS_SET(MAN_RX_PIN);
    }

/******************************************************************************/
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
