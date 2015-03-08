/**
 * @file        spi.h
 *
 * @author      David Zemon
 *
 * @copyright
 * The MIT License (MIT)<br>
 * <br>Copyright (c) 2013 David Zemon<br>
 * <br>Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:<br>
 * <br>The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.<br>
 * <br>THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/thread.h>
#include <PropWare/PropWare.h>
#include <PropWare/pin.h>
#include <PropWare/printer/printer.h>
#include <PropWare/scanner.h>

/**
 * @name   SPI Extra Code Options
 * @{
 */
/**
 * Parameter checking within each function call. I recommend you leave this option enabled unless speed is critical
 * <p>
 * DEFAULT: On
 */
#define SPI_OPTION_DEBUG_PARAMS
/**
 * TODO: Figure out why this doesn't work... :(
 * <p>
 * DEFAULT: Off
 */
#define SPI_OPTION_FAST_SECTOR
// This allows Doxygen to document the macro without permanently enabling it
#undef SPI_OPTION_FAST_SECTOR
/** @} */

namespace PropWare {

// Symbol for assembly instructions to start a new SPI cog
extern "C" {
extern uint32_t _SPIStartCog (void *arg);
}

/**
 * @brief   SPI serial communications library; Core functionality comes from a dedicated assembly cog
 *
 * Generally, multiple instances of the SPI class are not desired. To avoid the programmer from accidentally creating
 * multiple instances, this class is set up as a singleton. A static instance can be retrieved with
 * get_instance(). If multiple instances of PropWare::SPI are desired, the PropWare library (and your
 * project) should be built from source with PROPWARE_NO_SAFE_SPI defined
 */
class SPI : public PrintCapable,
            public ScanCapable {
#define check_errors_w_str(x, y) \
        if ((err = x)) {strcpy(this->m_errorInMethod, y);return err;}

    public:
        /**
         * @brief   Descriptor for SPI signal as defined by Motorola modes
         *
         * CPOL 0 refers to a low polarity (where the clock idles in the low state) and CPOL 1 is for high polarity.
         *
         * TODO: Describe phase
         * <table><tr><td>SPI Mode</td><td>CPOL</td><td>CPHA</td></tr><tr><td>0</td><td>0</td><td>0</td></tr><tr>
         * <td>1</td><td>0</td><td>1</td></tr><tr><td>2</td><td>1</td><td>0</td></tr><tr><td>3</td><td>1</td><td>1</td>
         * </tr></table>
         */
        /* Raw text version of the above HTML table
         *
         * SPI Mode     CPOL    CPHA
         * 0            0       0
         * 1            0       1
         * 2            1       0
         * 3            1       1
         */
        typedef enum {
            /** Mode 0 */MODE_0,
            /** Mode 1 */MODE_1,
            /** Mode 2 */MODE_2,
            /** Mode 3 */MODE_3
        } Mode;

        /**
         * @brief   Determine if data is communicated with the LSB or MSB sent/received first
         *
         * @note    Initial value is SPI_MODES + 1 making them easily distinguishable
         */
        typedef enum {
            /**
             * Start the enumeration where Mode left off; this ensures no overlap
             */
            LSB_FIRST = MODE_3 + 1,
            MSB_FIRST
        } BitMode;

        /**
         * Error codes - Proceeded by nothing
         */
        typedef enum {
            /** No error */           NO_ERROR    = 0,
            /** First SPI error */    BEG_ERROR,
            /** SPI Error  0 */       INVALID_PIN = BEG_ERROR,
            /** SPI Error  1 */       INVALID_CLOCK_INIT,
            /** SPI Error  2 */       INVALID_MODE,
            /** SPI Error  3 */       INVALID_PIN_MASK,
            /** SPI Error  4 */       TOO_MANY_BITS,
            /** SPI Error  5 */       TIMEOUT,
            /** SPI Error  6 */       TIMEOUT_RD,
            /** SPI Error  7 */       EXCESSIVE_PAR_SZ,
            /** SPI Error  8 */       COG_NOT_STARTED,
            /** SPI Error  9 */       MODULE_NOT_RUNNING,
            /** SPI Error 10 */       INVALID_FREQ,
            /** SPI Error 11 */       INVALID_BYTE_SIZE,
            /** SPI Error 12 */       ADDR_MISALIGN,
            /** SPI Error 13 */       INVALID_BITMODE,
            /** Last SPI error code */END_ERROR   = INVALID_BITMODE
        } ErrorCode;

    public:
        // (Default: CLKFREQ/10) Wait 0.1 seconds before throwing a timeout
        // error
        static const uint32_t TIMEOUT_VAL;
        static const uint8_t  MAX_PAR_BITS = 31;
        static const int32_t  MAX_CLOCK;

#ifndef PROPWARE_NO_SAFE_SPI
    private:
#else
    public:
#endif
        /**
         * @brief   Create a new instance of SPI which will, upon calling start(), start a new assembly cog.
         *          Creating multiple instances of PropWare::SPI allows the user to have multiple, independent SPI
         *          modules for simultaneous communication
         */
        SPI () : m_mailbox(-1),
                 m_cog(-1) {
        }

    public:
        /**
         * @brief   Retrieve an instance of the SPI module
         *
         * @return  Address of an SPI module
         */
        static SPI* get_instance () {
            // TODO: Add a lock to make this thread-safe
            static SPI instance;
            return &instance;
        }

        /**
         * @brief       Initialize an SPI module by starting a new cog
         *
         * @param[in]   mosi        PinNum mask for MOSI
         * @param[in]   miso        PinNum mask for MISO
         * @param[in]   sclk        PinNum mask for SCLK
         * @param[in]   frequency   Frequency, in Hz, to run the SPI clock; Must be less than CLKFREQ/4
         * @param[in]   mode        Select one of the 4 Motorola SPI modes to choose your polarity and clock phase
         * @param[in]   bitmode     One of MSB-first or LSB-first to determine the order that data is sent and received
         *
         * @return      Returns 0 upon success, otherwise error code
         */
        PropWare::ErrorCode start (const Port::Mask mosi, const Port::Mask miso, const Port::Mask sclk,
                                   const int32_t frequency, const Mode mode, const BitMode bitmode) {
            PropWare::ErrorCode err;
            const char str[] = "start";

#ifdef SPI_OPTION_DEBUG_PARAMS
            // Check clock frequency
            if (MAX_CLOCK <= frequency)
                return INVALID_FREQ;
#endif

            // If cog already started, do not start another
            if (!this->is_running()) {

                // Start GAS cog
                // Set the mailbox to 0 (anything other than -1) so that we know
                // when the SPI cog has started
                this->m_mailbox = 0;
                this->m_cog = PropWare::_SPIStartCog((void *) &this->m_mailbox);
                if (!this->is_running())
                    return COG_NOT_STARTED;

                // Pass in all parameters
                check_errors_w_str(this->wait(), str);
                this->m_mailbox = mosi;
                check_errors_w_str(this->wait(), str);
                this->m_mailbox = Pin::convert(mosi);
                check_errors_w_str(this->wait(), str);
                this->m_mailbox = miso;
                check_errors_w_str(this->wait(), str);
                this->m_mailbox = Pin::convert(miso);
                check_errors_w_str(this->wait(), str);
                this->m_mailbox = sclk;
            }

            check_errors_w_str(this->set_mode(mode), str);
            check_errors_w_str(this->set_bit_mode(bitmode), str);
            check_errors_w_str(this->set_clock(frequency), str);

            return NO_ERROR;
        }

        /**
         * @brief   Stop a running SPI cog
         *
         * @return  Returns 0 upon success, otherwise error code (will return
         *          COG_NOT_STARTED if no cog has previously been started)
         */
        PropWare::ErrorCode stop () {
            if (!this->is_running())
                return NO_ERROR;

            cogstop(this->m_cog);
            this->m_cog = -1;
            this->m_mailbox = -1;

            return NO_ERROR;
        }

        /**
         * @brief    Determine if the SPI cog has already been initialized
         *
         * @return   Returns 1 if the SPI cog is up and running, 0 otherwise
         */
        bool is_running () const {
            return -1 != this->m_cog;
        }

        /**
         * @brief   Wait for the SPI cog to signal that it is in the idle state
         *
         * @return  May return non-zero error code when a timeout occurs
         */
        PropWare::ErrorCode wait () const {
            const uint32_t timeoutCnt = TIMEOUT_VAL + CNT;

            // Wait for GAS cog to read in value and write -1
            while (-1 != this->m_mailbox)
                if (abs(timeoutCnt - CNT) < TIMEOUT_WIGGLE_ROOM)
                    return TIMEOUT;

            return NO_ERROR;
        }

        /**
         * @brief       Set the mode of SPI communication
         *
         * @param[in]   mode    Sets the SPI mode to one MODE_0, MODE_1, MODE_2, or MODE_3
         *
         * @return      Can return non-zero in the case of a timeout
         */
        PropWare::ErrorCode set_mode (const Mode mode) {
            PropWare::ErrorCode err;
            char str[] = "set_mode";

            if (!this->is_running())
                return MODULE_NOT_RUNNING;

            // Wait for SPI cog to go idle
            check_errors_w_str(this->wait(), str);
            this->m_mailbox = SET_MODE;
            check_errors_w_str(this->wait(), str);
            this->m_mailbox = mode;

            return NO_ERROR;
        }

        /**
         * @brief       Set the bitmode of SPI communication
         *
         * @param[in]   bitmode     Select one of LSB_FIRST or MSB_FIRST to choose which bit will be shifted
         *                          out first
         *
         * @return      Can return non-zero in the case of a timeout
         */
        PropWare::ErrorCode set_bit_mode (const BitMode bitmode) {
            PropWare::ErrorCode err;
            char str[] = "set_bit_mode";

            if (!this->is_running())
                return MODULE_NOT_RUNNING;
#ifdef SPI_OPTION_DEBUG_PARAMS
            if (LSB_FIRST != bitmode && MSB_FIRST != bitmode)
                return INVALID_BITMODE;
#endif

            check_errors_w_str(this->wait(), str);
            this->m_mailbox = SET_BITMODE;
            check_errors_w_str(this->wait(), str);
            this->m_mailbox = bitmode;

            return NO_ERROR;
        }

        /**
         * @brief       Change the SPI module's clock frequency
         *
         * @param[in]   frequency   Frequency, in Hz, to run the SPI clock; Must be less than CLKFREQ/4 (for 80 MHz,
         *                          900 kHz is the fastest I've tested successfully)
         *
         * @return      Returns 0 upon success, otherwise error code
         */
        PropWare::ErrorCode set_clock (const int32_t frequency) {
            PropWare::ErrorCode err;
            char str[] = "set_clock";

            if (!this->is_running())
                return MODULE_NOT_RUNNING;
#ifdef SPI_OPTION_DEBUG_PARAMS
            if (MAX_CLOCK <= frequency || 0 > frequency)
                return INVALID_FREQ;
#endif

            // Wait for SPI cog to go idle
            check_errors_w_str(this->wait(), str);
            // Prepare cog for clock frequency change
            this->m_mailbox = SET_FREQ;
            // Wait for the ready command
            check_errors_w_str(this->wait_specific(SET_FREQ), str);
            // Send new frequency
            this->m_mailbox = (CLKFREQ / frequency) >> 1;

            return NO_ERROR;
        }

        /**
         * @brief       Retrieve the SPI module's clock frequency
         *
         * @param[out]  *frequency  Frequency, in Hz, that the SPI module is running
         *
         * @return      Returns 0 upon success, otherwise error code
         */
        PropWare::ErrorCode get_clock (int32_t *frequency) {
            PropWare::ErrorCode err;
            char str[] = "get_clock";

#ifdef SPI_OPTION_DEBUG_PARAMS
            // Check for errors
            if (!this->is_running())
                return MODULE_NOT_RUNNING;
#endif

            // Wait to ensure the SPI cog is in its idle state
            check_errors_w_str(this->wait(), str);

            // Call GAS function
            this->m_mailbox = GET_FREQ;
            check_errors_w_str(this->wait_specific(GET_FREQ), str);

            this->read_par(frequency);
            *frequency = CLKFREQ / (*frequency << 1);

            return NO_ERROR;
        }

        /**
         * @brief       Send a value out to a peripheral device
         *
         * Pass a value and mode into the assembly cog to be sent to the peripheral; NOTE: this function is non-blocking
         * and chip-select should not be set inactive immediately after the return (you should call spi_wait() before
         * setting chip-select inactive)
         *
         * @param[in]   bits        Number of bits to be shifted out
         * @param[in]   value       The value to be shifted out
         *
         * @return      Returns 0 upon success, otherwise error code
         */
        PropWare::ErrorCode shift_out (uint8_t bits, uint32_t value) {
            PropWare::ErrorCode err;
            char str[] = "shift_out";

#ifdef SPI_OPTION_DEBUG_PARAMS
            // Check for errors
            if (!this->is_running())
                return MODULE_NOT_RUNNING;
            if (MAX_PAR_BITS < bits)
                return TOO_MANY_BITS;
#endif

            // Wait to ensure the SPI cog is in its idle state
            check_errors_w_str(this->wait(), str);

            // Call GAS function
            this->m_mailbox = SEND | (bits << BITS_OFFSET);
            check_errors_w_str(
                    this->wait_specific(
                            SEND | (bits << BITS_OFFSET)), str);

            // Pass parameter in; Bit 31 is cleared to indicate data is being sent. Without this limitation, who's to
            // say the value being passed is not -1?
            this->m_mailbox = (atomic_t) (value & (~BIT_31));

            return NO_ERROR;
        }

        /**
         * @brief       Receive a value in from a peripheral device
         *
         * @param[in]   bits    Number of bits to be shifted in
         * @param[out]  *data   Received data will be stored at this address
         *
         * @return      Returns 0 upon success, otherwise error code
         */
        template<typename T>
        PropWare::ErrorCode shift_in (const uint8_t bits, T *data) {
            PropWare::ErrorCode err;
            const char str[] = "shift_in";

            // Check for errors
#ifdef SPI_OPTION_DEBUG_PARAMS
            if (!this->is_running())
                return MODULE_NOT_RUNNING;
            if (MAX_PAR_BITS < bits)
                return TOO_MANY_BITS;
#endif

            // Ensure SPI module is not busy
            check_errors_w_str(this->wait(), str);

            // Call GAS function
            this->m_mailbox = READ | (bits << BITS_OFFSET);

            // Read in parameter
            check_errors_w_str(this->read_par(data), str);

            return NO_ERROR;
        }

        /**
         * @brief       Send a value out to a peripheral device
         *
         * Pass a value and mode into the assembly cog to be sent to the peripheral; NOTE: this function is non-blocking
         * and chip-select should not be set inactive immediately after the return (you should call spi_wait()
         * before setting chip-select inactive); Optimized for fastest possible clock speed; No error checking is
         * performed; 'Timeout' event will never be thrown and possible infinite loop can happen
         *
         * @param[in]   bits        Number of bits to be shifted out
         * @param[in]   value       The value to be shifted out
         *
         * @return      Returns 0 upon success, otherwise error code
         */
        PropWare::ErrorCode shift_out_fast (uint8_t bits, uint32_t value) {
            // NOTE: No debugging within this function to allow for fastest possible execution time
            // Wait to ensure the SPI cog is in its idle state
            this->wait();

            // Call GAS function
            this->m_mailbox = SEND_FAST | (bits << BITS_OFFSET);
            this->wait();

            // Pass parameter in; Bit 31 is cleared to indicate data is being sent. Without this limitation, who's to
            // say the value being passed is not -1?
            this->m_mailbox = (atomic_t) (value & (~BIT_31));

            return NO_ERROR;
        }

        /**
         * @brief       Quickly receive a value in from a peripheral device
         *
         * Optimized for fastest possible clock speed; No error checking is performed; 'Timeout' event will never be
         * thrown and possible infinite loop can happen
         *
         * @param[in]   bits    Number of bits to be shifted in
         * @param[out]  *data   Received data will be stored at this address
         */
        template<typename T>
        PropWare::ErrorCode shift_in_fast (const uint8_t bits, T *data) {
            // Wait until idle state, then send function and mode bits
            this->wait();
            this->m_mailbox = READ_FAST | (bits << BITS_OFFSET);

            // Wait for a value to be written
            while (-1 == this->m_mailbox)
                waitcnt(TIMEOUT_WIGGLE_ROOM + CNT);

            *data = this->m_mailbox;

            // Signal that value is saved and GAS cog can continue execution
            this->m_mailbox = -1;

            return NO_ERROR;
        }

        /**
         * @brief       Read an entire sector of data in from an SD card
         *
         * @param[out]  *addr       First hub address where the data should be written
         * @param[in]   blocking    When set to non-zero, function will not return until the data transfer is complete
         */
        PropWare::ErrorCode shift_in_sector (const uint8_t addr[], const uint8_t blocking) {
            this->wait();
            this->m_mailbox = READ_SECTOR;
            this->wait();
            this->m_mailbox = (atomic_t) addr;
            if (blocking)
                return this->wait();
            else
                return NO_ERROR;
        }

        void put_char (const char c) {
            this->shift_out(8, (uint32_t) c);
        }

        void puts (const char string[]) {
            char *sPtr = (char *) string;
            while (sPtr)
                this->put_char(*sPtr++);
        }

        char get_char () {
            char c = 0;
            this->shift_in(8, &c);
            return c;
        }

        /**
         * @brief       Print through UART an error string followed by entering an infinite loop
         *
         * @param[in]   *printer    Object used for printing error string
         * @param[in]   err         Error number used to determine error string
         */
        void print_error_str (const Printer *printer, const ErrorCode err) const {
            const char str[] = "SPI Error ";
            const int relativeErr = err - BEG_ERROR;

            switch (err) {
                case INVALID_PIN:
                    *printer << str << relativeErr << ": Invalid pin\n";
                    break;
                case INVALID_MODE:
                    *printer << str << relativeErr << ": Invalid mode\n";
                    break;
                case INVALID_PIN_MASK:
                    *printer << str << relativeErr << ": Invalid pin mask\n";
                    break;
                case TOO_MANY_BITS:
                    *printer << str << relativeErr << ": Incapable of handling so many bits in an argument\n";
                    break;
                case TIMEOUT:
                    *printer << str << relativeErr << ": Timed out during parameter passing\n";
                    *printer << "\tCalling function was " << this->m_errorInMethod << "()\n";
                    break;
                case TIMEOUT_RD:
                    *printer << str << relativeErr << ": Timed out during parameter read";
                    break;
                case COG_NOT_STARTED:
                    *printer << str << relativeErr << ": SPI's GAS cog was not started";
                    break;
                case MODULE_NOT_RUNNING:
                    *printer << str << relativeErr << ": SPI GAS cog not running";
                    break;
                case INVALID_FREQ:
                    *printer << str << relativeErr << ": Frequency set too high";
                    break;
                case ADDR_MISALIGN:
                    *printer << str << relativeErr << ": Passed in address is miss aligned";
                    break;
                default:
                    // Is the error an SPI error?
                    if (err > BEG_ERROR && err < (BEG_ERROR + END_ERROR))
                        *printer << "Unknown SPI error " << relativeErr << '\n';
                    else
                        *printer << "Unknown error " << err << '\n';
            }
        }

    protected:
        /*************************************
         *** Protected Method Declarations ***
         *************************************/
        static const uint16_t TIMEOUT_WIGGLE_ROOM = 600;

        typedef enum {
            SEND,
            READ,
            SEND_FAST,
            READ_FAST,
            READ_SECTOR,
            SET_MODE,
            SET_BITMODE,
            SET_FREQ,
            GET_FREQ
        } FunctionConstant;

        static const uint8_t BITS_OFFSET = 8;

    protected:
        /*************************************
         *** Protected Method Declarations ***
         *************************************/
        /**
         * @brief       Read the value that the SPI cog just shifted in
         *
         * @param[out]  *par    Address to store the parameter
         * @param[in]   size    Number of bytes allocated to *data; Example:
         *                        int newVal;
         *                        spi.read_par(&newVal, sizeof(newVal));
         *                      Or if using a pointer:
         *                        int *newVal;
         *                        spi.read_par(newVal, sizeof(*newVal));
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        template<typename T>
        PropWare::ErrorCode read_par (T *par) {
            const uint32_t timeoutCnt = TIMEOUT_VAL + CNT;

            // Wait for a value to be written
            while (-1 == this->m_mailbox)
                if (abs(timeoutCnt - CNT) < TIMEOUT_WIGGLE_ROOM)
                    return TIMEOUT_RD;

            *par = this->m_mailbox;

            this->m_mailbox = -1;

            return NO_ERROR;
        }

        /**
         * @brief   Wait for a specific value from the assembly cog
         *
         * @param[in]   value   The value being waited on
         *
         * @return  May return non-zero error code when a timeout occurs
         */
        PropWare::ErrorCode wait_specific (const uint32_t value) const {
            const uint32_t timeoutCnt = TIMEOUT_VAL + CNT;

            // Wait for GAS cog to read in value and write -1
            while (value == (uint32_t) this->m_mailbox)
                if (abs(timeoutCnt - CNT) < TIMEOUT_WIGGLE_ROOM)
                    // Always use return instead of spi_error() for private functions
                    return TIMEOUT;

            return NO_ERROR;
        }

    protected:
        /***************************************
         *** Protected Variable Declarations ***
         ***************************************/
        volatile atomic_t m_mailbox;
        int8_t            m_cog;
        char              m_errorInMethod[16];
    };

}
