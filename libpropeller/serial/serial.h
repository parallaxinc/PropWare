#ifndef LIBPROPELLER_SERIAL_H_
#define LIBPROPELLER_SERIAL_H_

#include <cstdarg>
#include <propeller.h>
#include "libpropeller/numbers/numbers.h"

#include "libpropeller/streaminterface/streaminterface.h"

#include "libpropeller/printstream/printstream.h"

extern char _load_start_serial_cog[];

/** A high speed and high accuracy serial driver.
 * Based on Fast Full-Duplex Serial 1 (FFDS1) version 0.9 by Jonathan Dummer (lonesock). C++ Port done by SRLM.
 * 
 * Serial provides a fast and stable serial interface using a single cog.
 * 
 * Max baudrate = clkfreq / (86 * 2)
 * 
 *    Clock  | MaxBaud | Standard
 *    -------+---------+---------
 *    96 MHz | 558_139 | 500_000    - 6MHz XTAL at 16x PLL
 *    80 MHz | 465_116 | 460_800    - 5MHz XTAL at 16x PLL (most common)
 *    12 MHz |  69_767 |  57_600    - approx RCFAST
 *    20 kHz |     116 | hah hah    - approx RCSLOW
 * 
 * Bit period is calculated to the nearest 2 clocks. So, the bit period should be within 1 clock, or 12.5 ns at 80 MHz.
 * 
 * @author SRLM (srlm@srlmproductions.com)
 * 
 */
class Serial : public InputStream<Serial>, public OutputStream<Serial>{
//class Serial : public StreamInterface{
//class Serial{
public:

    /** Size of the RX buffer in bytes. No restrictions on size (well, Hub RAM 
     * limits [8^) Does not need to be a power of 2. There is no TX buffer, 
     * Serial sends directly from Hub RAM.
     * 
     * For some reason, 117 seems to be the minimum buffer size before the tests
     * start to lock up. Originally, it was 512. Seems safe to change, as long
     * as it's >= 117.
     */
    static const int kBufferLength = 512;

    ~Serial() {
        Stop();
    }

    /** Start Serial driver in a new cog.
     * 
     * Set any pin to -1 to disable it. No pin can match any other pin.
     * 
     * @param rxpin the pin [0..31] for incoming data.
     * @param txpin the pin [0..31] for outgoing data.
     * @param rate  the initial baud rate in bits per second.
     * @param ctspin  cts is an input for control of flow from the tx pin. If high, it disables transmissions.
     * @return  Returns true if the cog started OK, false otherwise
     */
    bool Start(const int rxpin, const int txpin, const int rate, const int ctspin = -1) {
        
        // Prevent garbage collection of the ASM code
        volatile void * asm_driver_reference = NULL;
        __asm__ volatile ("mov %[asm_driver_reference], #Fds_entry \n\t"
                : [asm_driver_reference] "+r" (asm_driver_reference));

        Stop();

        extern char * Masktx asm("Masktx");
        extern char * Maskrx asm("Maskrx");
        extern char * Ctra_val asm("Ctra_val");
        extern char * Ctrb_val asm("Ctrb_val");
        extern char * Period_ptr asm("Period_ptr");
        extern char * Rx_head_ptr asm("Rx_head_ptr");
        extern char * Rx_end_ptr asm("Rx_end_ptr");
        extern char * Update_head_ptr asm("Update_head_ptr");
        extern char * Maskcts asm("Maskcts");


        SetDriverLong(&Masktx, 0);
        SetDriverLong(&Ctra_val, 0);
        if (txpin >= 0) {
            DIRA |= 1 << txpin;
            SetDriverLong(&Masktx, 1 << txpin);
            SetDriverLong(&Ctra_val, 0x10000000 | txpin);
        }
        SetDriverLong(&Maskrx, 0);
        SetDriverLong(&Ctrb_val, 0);
        if (rxpin >= 0) {
            DIRA &= ~(1 << rxpin);

            SetDriverLong(&Maskrx, 1 << rxpin);
            SetDriverLong(&Ctrb_val, 0x54000000 | rxpin);
        }


        SetDriverLong(&Maskcts, 0);
        if (ctspin >= 0) {
            //Set CTS pin to input:
            DIRA &= ~(1 << ctspin);

            SetDriverLong(&Maskcts, 1 << ctspin);
        }


        SetBaud(rate);

        SetDriverLong(&Period_ptr, (int) &half_bit_period_);
        memset((void *) &rx_buffer_, 0, kBufferLength);

        SetDriverLong(&Rx_head_ptr, (int) &rx_buffer_);
        SetDriverLong(&Rx_end_ptr, (int) &rx_buffer_ + kBufferLength);

        rx_head_ = 0;
        rx_tail_ = 0;

        SetDriverLong(&Update_head_ptr, (int) &rx_head_);
        write_buf_ptr_ = 1;
        cog_ = 1 + cognew((int) (&(*(int *) &_load_start_serial_cog[0])), (int) (&write_buf_ptr_));
        if (cog_) {
            WaitForTransmissionCompletion();
            return true;
        }
        return false;
    }

    /** Stops the Serial PASM engine, if it is running. Frees a cog.
     */
    void Stop(void) {
        WaitForTransmissionCompletion();
        if (cog_) {
            cogstop(cog_ - 1);
            cog_ = 0;
        }
    }

    /** Does a live update of the baud rate in the Serial engine.
     * 
     * @param rate desired baud rate of the Serial engine.
     * @return     true if the baud rate was achieved
     */
    bool SetBaud(const int rate) {
        return SetBaudClock(rate, CLKFREQ);
    }

    /** Does a live update of the baud rate in the Serial engine.
     * 
     * Examples:
     *          got_desired_rate = SetBaudClock( 9600, CLKFREQ )
     *          got_desired_rate = SetBaudClock( 9600, actual_sys_clock_freq )
     * 
     * @param rate     desired baud rate of the Serial engine.
     * @param sysclock the system clock (use CLKFREQ), or provide your own
     *                  (useful when using RCFAST).
     * @return        true if the baud rate was achieved
     */
    bool SetBaudClock(const unsigned int rate, const unsigned int sysclock) {
        WaitForTransmissionCompletion();

        // how many clocks per 1/2 bit (pre-round to the nearest integer)
        int got_rate = ((sysclock >> 1) + (rate >> 1)) / rate;

        // clamp the period to the allowable range
        half_bit_period_ = got_rate > kMinimumHalfPeriod ? got_rate : kMinimumHalfPeriod;

        // return true if the requested period was >= the allowable limit
        return got_rate >= kMinimumHalfPeriod;
    }

    /** Transmit a single character.
     * 
     * @param character the byte to send.
     */
    void Put(const char character) {
        WaitForTransmissionCompletion();
        send_temp_ = character;
        write_buf_ptr_ = (int) (&send_temp_);
    }

    /** Transmit a set of bytes.
     * 
     * @param buffer_ptr The array of bytes to send.
     * @param count      The number of bytes to send.
     * @return           The number of bytes actually sent. An error has
     *                          occurred if this is less than strlen(buffer_ptr)
     */
    int Put(const char * buffer_ptr, const int count) {
        WaitForTransmissionCompletion();
        for (int i = 0; i < count; i++) {
            Put(buffer_ptr[i]);
        }
        return count;
    }

    /** Transmit a C string.
     * 
     * @param buffer_ptr The null terminated string to put. Note: does not 
     *                          transmit the trailing null.
     * @return The number of bytes actually sent. An error has occurred if this 
     *                          is less than strlen(buffer_ptr)
     */
    int Put(const char * buffer_ptr) {
        return Put(buffer_ptr, strlen(buffer_ptr));
    }

    

    /** Receive a byte (wait) or timeout.
     * 
     * @warning This function may block indefinitely if timeout is set to a
     *                  negative value, and no data is received.
     * 
     * The timeout duration is a minimum, not a precise number. Experimental 
     * results indicate that a timeout of 10ms results in a total function time 
     * of 10.047ms.
     * 
     * @param   timeout the number of milliseconds to wait. Defaults to forever 
     *                          wait (negative timeout). Maximum value is 53000 
     *                          milliseconds (no bounds checking).
     * @return          -1 if no byte received, 0x00..0xFF if byte
     */
    int Get(const int timeout = -1) {
        int rxbyte = 0;

        if (timeout <= -1) { //No timeout, wait forever

            while ((rxbyte = CheckBuffer()) < 0);
            return (char) rxbyte;
        } else {
            unsigned int total_cycles = (CLKFREQ / 1000) * timeout;
            unsigned int elapsed_cycles = 0;

            int previous_cnt = CNT;
            do {
                rxbyte = CheckBuffer();
                int current_cnt = CNT;
                elapsed_cycles += current_cnt - previous_cnt;
                previous_cnt = current_cnt;
            } while (rxbyte < 0 && elapsed_cycles < total_cycles);
            return rxbyte;
        }
    }

    /** Get a buffer of characters
     * 
     * @param buffer  The buffer to store the received characters in.
     * @param length  The maximum number of characters to read (may be less 
     *                  than @a length if @a timeout is enabled.
     * @param timeout The maximum amount of time to wait for _each_ character, 
     *                  _not_ the total time of the function.
     * @returns the number of characters read. If less than @a length, then a
     *                  timeout occurred.
     */
    int Get(char * const buffer, const int length, const int timeout = -1) {
        int character_count;
        for (character_count = 0; character_count < length; ++character_count) {
            int character = Get(timeout);
            if (character <= -1) {
                return character_count;
            }
            buffer[character_count] = (char) character;
        }
        return character_count;
    }

    /** Get a terminated string of characters
     * 
     * @param buffer     The buffer to store the received characters in. Note:
     *                          an extra '\0' is always added to the end (null 
     *                          terminate).
     * @param terminator The last character to put into the buffer. On the 
     *                          first instance of this character, the function 
     *                          returns.
     * @returns          The number of characters read, including terminator. 
     *                          Does not include null terminator.
     */
    int Get(char * const buffer, const char terminator = '\n') {
        int character_count;
        char received_character = terminator + 1; // guarantee that they differ the first time.
        for (character_count = 0; received_character != terminator; ++character_count) {
            received_character = (char) Get();
            buffer[character_count] = received_character;
        }
        buffer[character_count] = '\0';
        return character_count;
    }

    /** Flushes receive buffer.
     */
    void GetFlush(void) {
        rx_tail_ = rx_head_;
    }

    /** Get the number of bytes in the receive buffer.
     */
    int GetCount(void) const {
        const int tail = rx_tail_;
        const int head = rx_head_;
        if (head >= tail) {
            return head - tail;
        } else {
            return kBufferLength - tail + head;
        }
    }
    
    int PutFormatted(const char * formatString, ...){
        PrintStream<Serial> ps(this);
        
        va_list list;
        va_start(list, formatString);
        int result = ps.Format(formatString, list);
        va_end(list);
        return result;
    }


private:
    /** Half period must be at least this, otherwise the cog will sleep for a whole counter cycle (2^32 / clkfreq seconds).
     * 
     * 86 @80MHz
     */
    static const int kMinimumHalfPeriod = 86;

    volatile int write_buf_ptr_;
    volatile int send_temp_;
    volatile int half_bit_period_;
    volatile short rx_head_;
    volatile short rx_tail_;
    volatile char rx_buffer_[kBufferLength];
    int cog_;

    /** Checks if byte is waiting in the buffer, but doesn't wait.
     * @return -1 if no byte received, 0x00..0xFF if byte
     */
    int CheckBuffer(void) {
        int rxbyte = -1;
        if (rx_tail_ != rx_head_) {
            rxbyte = rx_buffer_[rx_tail_];
            rx_buffer_[rx_tail_] = 0;
            rx_tail_ = ((rx_tail_ + 1) % kBufferLength);
        }
        return rxbyte;
    }

    void WaitForTransmissionCompletion(void) {
        while (write_buf_ptr_) {
        };
    }

    void SetDriverLong(const int index, const int value) {
        ((int *) &_load_start_serial_cog[index])[0] = value;
    }

    /* Warning: SetDriverLong(char **, int) must be declared second, otherwise it calls itself! */
    void SetDriverLong(char ** index, const int value) {
        SetDriverLong((int) index, value);
    }
};




//
// The Put(buffer) function doesn't seem to work in CMM mode. In the tests, I 
// get a -1 for the matching Get(), instead of the character sent. The same code
// can pass in LMM mode.

// Update: now, it doesn't work at all.

//int Serial::PutBuffer(char * buffer_ptr, const bool wait, int buffer_bytes, const char terminator)
//{
//	volatile char * volatile temp_ptr = buffer_ptr;
//	if(buffer_bytes == -1){
//		if(terminator == '\0'){
//			buffer_bytes = strlen(buffer_ptr);
//			
//		}else{
//			for(buffer_bytes = 0; buffer_ptr[buffer_bytes] != terminator; buffer_bytes++){}
//		}
//			
//	}

//	buffer_bytes = 5;

//	if (buffer_bytes > 0 && buffer_ptr != NULL) {
//	
//		send_temp_ = (int)(buffer_ptr);
//    	write_buf_ptr_ = (send_temp_ | ((buffer_bytes - 1) << 16));
//  	}
//	
//	if(wait){
//		while (write_buf_ptr_){}
//	}
//	return buffer_bytes;
//}

#endif // LIBPROPELLER_SERIAL_H_
