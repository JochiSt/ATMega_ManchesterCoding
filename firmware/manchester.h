/**
 * Manchester Encoding Software UART-like (TX and RX)
 * Goal is to provide functions to send and receive data with a given data rate
 * like and UART. Since the input and output data is Manchester encoded (1b2b)
 * the data rate will not follow the 'classical' baudrates.
 *
 * This code is based and inspired by:
 * - ATMEL APPLICATION NOTE 9164 Manchester Coding Basics
 */

#ifndef MANCHESTER_H
#define MANCHESTER_H

    #if defined (__AVR_ATmega328__) || defined (__AVR_ATmega328P__) || defined (__AVR_ATmega88__) || defined (__AVR_ATmega48__)
        #define TIMER_vect  TIMER0_COMPA_vect
        #define TCCRA       TCCR0A
        #define TCCRB       TCCR0B
        #define OCRA        OCR0A
        #define IRMSK       TIMSK0
    #endif

/// Pin for sending the Manchester Codes
#define MAN_TX_PIN      B,1
/// Pin for Receiving the Manchester Codes
#define MAN_RX_PIN      B,2

#define MAN_DBG_PIN_CLK  B,0

/**
 * @addtogroup RXvars Variables needed for receiving
 * @{
 */
/// Number of required sync '0's before any bit can start
#define REQ_SYNC_CNT        10
// the start pattern must be 8 bit, because the write char function is used
#define MAN_START_PATTERN   0b01010000
#define HEADER              0b1010
#define HEADER_LEN          4

volatile unsigned char man_RX_sync_cnt;   ///< counter of the sync bits
volatile unsigned char man_RX_synced;     ///< are we synced

volatile unsigned char man_RX_bitbuffer;  ///< buffering the single bits

#define MAN_RX_BUFFER_SIZE 16             ///< buffer size of the byte buffer
char man_RX_buffer[MAN_RX_BUFFER_SIZE];   ///< RX byte buffer
volatile unsigned char man_RX_buffercounter;

volatile unsigned char man_RXbitphase;    ///< the bit phase
volatile unsigned char man_RXbitcnt;      ///< bit counter

volatile unsigned char man_RX_bit0;       ///< RX bit of cycle 0
volatile unsigned char man_RX_bit1;       ///< RX bit of cycle 1
/** @} */
/**
 * @addtogroup TXvars Variables needed for transmission
 * @{
 */
volatile unsigned char man_TXbusy;        ///< are we still busy sending a byte
char man_TXbyte;                          ///< byte, which should be transmitted
volatile char man_TXbit;                  ///< bit, which is currently being sent
volatile unsigned char man_TXbitphase;    ///< indicating the TX bit phase
volatile unsigned char man_TXbitcnt;      ///< counter for counting the TX bits
/** @} */

void manchester_init(unsigned long datarate);
void manchester_resetSynchronization(void);
void manchester_write_char(char c);
void manchester_write_str(char *str);

/**
 * @addtogroup IntFunc internal functions
 * @{
 */
void manchester_setOutputPin(unsigned char txbit, unsigned char phase);
/** @} */

#endif
