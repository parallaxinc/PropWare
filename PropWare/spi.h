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
#include <PropWare/printer.h>

/** @name   SPI Extra Code Options
 * @{ */
/**
 * Parameter checking within each function call. I recommend you leave this
 * option enabled unless speed is critical
 * <p>
 * DEFAULT: On
 */
#define SPI_OPTION_DEBUG_PARAMS
/**
 * Allows for fast send and receive routines without error checking or timing
 * delays; Normal routines still available when enabled
 *
 * TODO: Use the counter module instead of "xor clkPin, clkPin"
 * <p>
 * DEFAULT: On
 */
#define SPI_OPTION_FAST
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
 * @brief       SPI serial communications library; Core functionality comes from
 *              a dedicated assembly cog
 *
 * Generally, multiple instances of the SPI class are not desired. To avoid
 * the programmer from accidentally creating multiple instances, this class is
 * set up as a singleton. A static instance can be retrieved with
 * PropWare::SPI::get_instance(). If multiple instances of PropWare::SPI are
 * desired, the PropWare library (and your project) should be built from
 * source with PROPWARE_NO_SAFE_SPI defined
 */
class SPI {
#define check_errors_w_str(x, y) \
        if ((err = x)) {strcpy(this->m_errorInMethod, y);return err;}

    public:
        /**
         * @brief   Descriptor for SPI signal as defined by Motorola modes
         *
         * CPOL 0 refers to a low polarity (where the clock idles in the low
         * state) and CPOL 1 is for high polarity.
         * TODO: Describe phase
         * <table><tr><td>SPI Mode</td><td>CPOL</td><td>CPHA</td></tr><tr>
         * <td>0</td><td>0</td><td>0</td></tr><tr><td>1</td><td>0</td><td>1</td>
         * </tr><tr><td>2</td><td>1</td><td>0</td></tr><tr><td>3</td><td>1</td>
         * <td>1</td></tr></table>
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
            /** Mode 3 */MODE_3,
        } Mode;

        /**
         * @brief   Determine if data is communicated with the LSB or MSB
         *          sent/received first
         *
         * @note    Initial value is SPI_MODES + 1 making them easily
         *          distinguishable
         */
        typedef enum {
            /**
             * Start the enumeration where SPI::Mode left off; this ensures no
             * overlap
             */
            LSB_FIRST = SPI::MODE_3 + 1,
            MSB_FIRST
        } BitMode;

        /**
         * Error codes - Proceeded by nothing
         */
        typedef enum {
            /** No error */NO_ERROR             = 0,
            /** First SPI error */BEG_ERROR,
            /** SPI Error  0 */INVALID_PIN      = BEG_ERROR,
            /** SPI Error  1 */INVALID_CLOCK_INIT,
            /** SPI Error  2 */INVALID_MODE,
            /** SPI Error  3 */INVALID_PIN_MASK,
            /** SPI Error  4 */TOO_MANY_BITS,
            /** SPI Error  5 */TIMEOUT,
            /** SPI Error  6 */TIMEOUT_RD,
            /** SPI Error  7 */EXCESSIVE_PAR_SZ,
            /** SPI Error  8 */COG_NOT_STARTED,
            /** SPI Error  9 */MODULE_NOT_RUNNING,
            /** SPI Error 10 */INVALID_FREQ,
            /** SPI Error 11 */INVALID_BYTE_SIZE,
            /** SPI Error 12 */ADDR_MISALIGN,
            /** SPI Error 13 */INVALID_BITMODE,
            /** Last SPI error code */END_ERROR = SPI::INVALID_BITMODE
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
         * @brief   Create a new instance of SPI which will, upon calling
         *          SPI::start(), start a new assembly cog. Creating multiple
         *          instances of PropWare::SPI allows the user to have multiple,
         *          independent SPI modules for simultaneous communication
         */
        SPI () {
            this->m_mailbox = -1;
            this->m_cog = -1;
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
         * @param[in]   frequency   Frequency, in Hz, to run the SPI clock; Must
         *                          be less than CLKFREQ/4
         * @param[in]   mode        Select one of the 4 Motorola SPI modes to
         *                          choose your polarity and clock phase
         * @param[in]   bitmode     One of MSB-first or LSB-first to determine
         *                          the order that data is sent and received
         *
         * @return      Returns 0 upon success, otherwise error code
         */
        PropWare::ErrorCode start (const PropWare::Port::Mask mosi,
                const PropWare::Port::Mask miso,
                const PropWare::Port::Mask sclk, const int32_t frequency,
                const SPI::Mode mode, const SPI::BitMode bitmode) {
            PropWare::ErrorCode err;
            const char str[] = "start";

#ifdef SPI_OPTION_DEBUG_PARAMS
            // Check clock frequency
            if (SPI::MAX_CLOCK <= frequency)
                return SPI::INVALID_FREQ;
#endif

            // If cog already started, do not start another
            if (!this->is_running()) {

                // Start GAS cog
                // Set the mailbox to 0 (anything other than -1) so that we know
                // when the SPI cog has started
                this->m_mailbox = 0;
                this->m_cog = PropWare::_SPIStartCog((void *) &this->m_mailbox);
                if (!this->is_running())
                    return SPI::COG_NOT_STARTED;

                // Pass in all parameters
                check_errors_w_str(this->wait(), str);
                this->m_mailbox = mosi;
                check_errors_w_str(this->wait(), str);
                this->m_mailbox = PropWare::Pin::convert(mosi);
                check_errors_w_str(this->wait(), str);
                this->m_mailbox = miso;
                check_errors_w_str(this->wait(), str);
                this->m_mailbox = PropWare::Pin::convert(miso);
                check_errors_w_str(this->wait(), str);
                this->m_mailbox = sclk;
            }

            check_errors_w_str(this->set_mode(mode), str);
            check_errors_w_str(this->set_bit_mode(bitmode), str);
            check_errors_w_str(this->set_clock(frequency), str);

            return SPI::NO_ERROR;
        }

        /**
         * @brief   Stop a running SPI cog
         *
         * @return  Returns 0 upon success, otherwise error code (will return
         *          SPI::COG_NOT_STARTED if no cog has previously been started)
         */
        PropWare::ErrorCode stop () {
            if (!this->is_running())
                return SPI::NO_ERROR;

            cogstop(this->m_cog);
            this->m_cog = -1;
            this->m_mailbox = -1;

            return SPI::NO_ERROR;
        }

        /**
         * @brief    Determine if the SPI cog has already been initialized
         *
         * @return   Returns 1 if the SPI cog is up and running, 0 otherwise
         */
        bool is_running () {
            return -1 != this->m_cog;
        }

        /**
         * @brief   Wait for the SPI cog to signal that it is in the idle state
         *
         * @return  May return non-zero error code when a timeout occurs
         */
        PropWare::ErrorCode wait () {
            const uint32_t timeoutCnt = SPI::TIMEOUT_VAL + CNT;

            // Wait for GAS cog to read in value and write -1
            while (-1 != this->m_mailbox)
                if (abs(timeoutCnt - CNT) < SPI::TIMEOUT_WIGGLE_ROOM)
                    return SPI::TIMEOUT;

            return SPI::NO_ERROR;
        }

        /**
         * @brief   Wait for a specific value from the assembly cog
         *
         * @param[in]   value   The value being waited on
         *
         * @return  May return non-zero error code when a timeout occurs
         */
        PropWare::ErrorCode wait_specific (const uint32_t value) {
            const uint32_t timeoutCnt = SPI::TIMEOUT_VAL + CNT;

            // Wait for GAS cog to read in value and write -1
            while (value == (uint32_t) this->m_mailbox)
                if (abs(timeoutCnt - CNT) < SPI::TIMEOUT_WIGGLE_ROOM)
                    // Always use return instead of spi_error() for private
                    // f0unctions
                    return SPI::TIMEOUT;

            return SPI::NO_ERROR;
        }

        /**
         * @brief       Set the mode of SPI communication
         *
         * @param[in]   mode    Sets the SPI mode to one SPI::MODE_0,
         *                      SPI::MODE_1, SPI::MODE_2, or SPI::MODE_3
         *
         * @return      Can return non-zero in the case of a timeout
         */
        PropWare::ErrorCode set_mode (const SPI::Mode mode) {
            PropWare::ErrorCode err;
            char str[] = "set_mode";

            if (!this->is_running())
                return SPI::MODULE_NOT_RUNNING;

            // Wait for SPI cog to go idle
            check_errors_w_str(this->wait(), str);

            this->m_mailbox = SPI::SET_MODE;
            check_errors_w_str(this->wait(), str);
            this->m_mailbox = mode;

            return SPI::NO_ERROR;
        }

        /**
         * @brief       Set the bitmode of SPI communication
         *
         * @param[in]   bitmode     Select one of SPI::LSB_FIRST or
         *                          SPI::MSB_FIRST to choose which bit will be
         *                          shifted out first
         *
         * @return      Can return non-zero in the case of a timeout
         */
        PropWare::ErrorCode set_bit_mode (const SPI::BitMode bitmode) {
            PropWare::ErrorCode err;
            char str[] = "set_bit_mode";

            if (!this->is_running())
                return SPI::MODULE_NOT_RUNNING;
#ifdef SPI_OPTION_DEBUG_PARAMS
            if (SPI::LSB_FIRST != bitmode && SPI::MSB_FIRST != bitmode)
                return SPI::INVALID_BITMODE;
#endif

            check_errors_w_str(this->wait(), str);
            this->m_mailbox = SPI::SET_BITMODE;
            check_errors_w_str(this->wait(), str);
            this->m_mailbox = bitmode;

            return SPI::NO_ERROR;
        }

        /**
         * @brief       Change the SPI module's clock frequency
         *
         * @param[in]   frequency   Frequency, in Hz, to run the SPI clock; Must
         *                          be less than CLKFREQ/4 (for 80 MHz, 900 kHz
         *                          is the fastest I've tested successfully)
         *
         * @return      Returns 0 upon success, otherwise error code
         */
        PropWare::ErrorCode set_clock (const int32_t frequency) {
            PropWare::ErrorCode err;
            char str[] = "set_clock";

            if (!this->is_running())
                return SPI::MODULE_NOT_RUNNING;
#ifdef SPI_OPTION_DEBUG_PARAMS
            if (SPI::MAX_CLOCK <= frequency || 0 > frequency)
                return SPI::INVALID_FREQ;
#endif

            // Wait for SPI cog to go idle
            check_errors_w_str(this->wait(), str);
            // Prepare cog for clock frequency change
            this->m_mailbox = SPI::SET_FREQ;
            // Wait for the ready command
            check_errors_w_str(this->wait_specific(SPI::SET_FREQ), str);
            // Send new frequency
            this->m_mailbox = (CLKFREQ / frequency) >> 1;

            return SPI::NO_ERROR;
        }

        /**
         * @brief       Retrieve the SPI module's clock frequency
         *
         * @param[out]  *frequency  Frequency, in Hz, that the SPI module is
         *                          running
         *
         * @return      Returns 0 upon success, otherwise error code
         */
        PropWare::ErrorCode get_clock (int32_t *frequency) {
            PropWare::ErrorCode err;
            char str[] = "get_clock";

#ifdef SPI_OPTION_DEBUG_PARAMS
            // Check for errors
            if (!this->is_running())
                return SPI::MODULE_NOT_RUNNING;
#endif

            // Wait to ensure the SPI cog is in its idle state
            check_errors_w_str(this->wait(), str);

            // Call GAS function
            this->m_mailbox = SPI::GET_FREQ;
            check_errors_w_str(this->wait_specific(SPI::GET_FREQ), str);

            this->read_par(frequency, sizeof(*frequency));
            *frequency = (int32_t) (CLKFREQ / (*frequency << 1));

            return SPI::NO_ERROR;
        }

        /**
         * @brief       Send a value out to a peripheral device
         *
         * Pass a value and mode into the assembly cog to be sent to the
         * peripheral; NOTE: this function is non-blocking and chip-select
         * should not be set inactive immediately after the return (you should
         * call spi_wait() before setting chip-select inactive)
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
                return SPI::MODULE_NOT_RUNNING;
            if (SPI::MAX_PAR_BITS < bits)
                return SPI::TOO_MANY_BITS;
#endif

            // Wait to ensure the SPI cog is in its idle state
            check_errors_w_str(this->wait(), str);

            // Call GAS function
            this->m_mailbox = SPI::SEND | (bits << SPI::BITS_OFFSET);
            check_errors_w_str(
                    this->wait_specific(
                            SPI::SEND | (bits << SPI::BITS_OFFSET)), str);

            // Pass parameter in; Bit 31 is cleared to indicate data is being
            // sent. Without this limitation, who's to say the value being
            // passed is not -1?
            this->m_mailbox = (atomic_t) (value & (~BIT_31));

            return SPI::NO_ERROR;
        }

        /**
         * @brief       Receive a value in from a peripheral device
         *
         * @param[in]   bits    Number of bits to be shifted in
         * @param[out]  *data   Received data will be stored at this address
         * @param[in]   size    Number of bytes allocated to *data; Example:
         *                        int newVal;
         *                        spi.shift_in(8, &newVal, sizeof(newVal));
         *                      Or if using a pointer:
         *                        int *newVal;
         *                        spi.shift_in(8, newVal, sizeof(*newVal));
         *
         * @return      Returns 0 upon success, otherwise error code
         */
        PropWare::ErrorCode shift_in (const uint8_t bits, void *data,
                const size_t size) {
            PropWare::ErrorCode err;
            const char str[] = "shift_in";

            // Check for errors
#ifdef SPI_OPTION_DEBUG_PARAMS
            if (!this->is_running())
                return SPI::MODULE_NOT_RUNNING;
            if (SPI::MAX_PAR_BITS < bits)
                return SPI::TOO_MANY_BITS;
            if ((4 == size && ((uint32_t) data) % 4)
                    || (2 == size && ((uint32_t) data) % 2))
                return SPI::ADDR_MISALIGN;
#endif

            // Ensure SPI module is not busy
            check_errors_w_str(this->wait(), str);

            // Call GAS function
            this->m_mailbox = SPI::READ | (bits << SPI::BITS_OFFSET);

            // Read in parameter
            check_errors_w_str(this->read_par(data, size), str);

            return SPI::NO_ERROR;
        }

#ifdef SPI_OPTION_FAST
        /**
         * @brief       Send a value out to a peripheral device
         *
         * Pass a value and mode into the assembly cog to be sent to the
         * peripheral; NOTE: this function is non-blocking and chip-select
         * should not be set inactive immediately after the return (you should
         * call spi_wait() before setting chip-select inactive); Optimized for
         * fastest possible clock speed; No error checking is performed;
         * 'Timeout' event will never be thrown and possible infinite loop can
         * happen
         *
         * @param[in]   bits        Number of bits to be shifted out
         * @param[in]   value       The value to be shifted out
         *
         * @return      Returns 0 upon success, otherwise error code
         */
        PropWare::ErrorCode shift_out_fast (uint8_t bits, uint32_t value) {
            // NOTE: No debugging within this function to allow for fastest
            // possible execution time
            // Wait to ensure the SPI cog is in its idle state
            this->wait();

            // Call GAS function
            this->m_mailbox = PropWare::SPI::SEND_FAST
                    | (bits << PropWare::SPI::BITS_OFFSET);
            this->wait();

            // Pass parameter in; Bit 31 is cleared to indicate data is being
            // sent. Without this limitation, who's to say the value being
            // passed is not -1?
            this->m_mailbox = (atomic_t) (value & (~BIT_31));

            return PropWare::SPI::NO_ERROR;
        }

        /**
         * @brief       Quickly receive a value in from a peripheral device
         *
         * Optimized for fastest possible clock speed; No error checking is
         * performed; 'Timeout' event will never be thrown and possible infinite
         * loop can happen
         *
         * @param[in]   bits    Number of bits to be shifted in
         * @param[out]  *data   Received data will be stored at this address
         * @param[in]   bytes   Number of bytes allocated to *data; Example:
         *                        int newVal;
         *                        spi.shift_in_fast(8, &newVal, sizeof(newVal));
         *                      Or if using a pointer:
         *                        int *newVal;
         *                        spi.shift_in_fast(8, newVal, sizeof(*newVal));
         */
        PropWare::ErrorCode shift_in_fast (const uint8_t bits, void *data,
                const uint8_t bytes) {
            uint8_t *par8;
            uint16_t *par16;
            uint32_t *par32;

            // Wait until idle state, then send function and mode bits
            this->wait();
            this->m_mailbox = PropWare::SPI::READ_FAST
                    | (bits << PropWare::SPI::BITS_OFFSET);

            // Wait for a value to be written
            while (-1 == this->m_mailbox)
                waitcnt(PropWare::SPI::TIMEOUT_WIGGLE_ROOM + CNT);

            // Determine if output variable is char, short or long and write
            // data to that location
            switch (bytes) {
                case sizeof(uint8_t):
                    par8 = (uint8_t *) data;
                    *par8 = (uint8_t) this->m_mailbox;
                    break;
                case sizeof(uint16_t):
                    par16 = (uint16_t *) data;
                    *par16 = (uint16_t) this->m_mailbox;
                    break;
                case sizeof(uint32_t):
                    par32 = (uint32_t *) data;
                    *par32 = (uint32_t) this->m_mailbox;
                    break;
                default:
                    return PropWare::SPI::INVALID_BYTE_SIZE;
            }

            // Signal that value is saved and GAS cog can continue execution
            this->m_mailbox = -1;

            return PropWare::SPI::NO_ERROR;
        }

        /**
         * @brief       Read an entire sector of data in from an SD card
         *
         * @param[out]  *addr       First hub address where the data should be
         *                          written
         * @param[in]   blocking    When set to non-zero, function will not
         *                          return until the data transfer is complete
         */
        PropWare::ErrorCode shift_in_sector (const uint8_t addr[],
                const uint8_t blocking) {
            this->wait();
            this->m_mailbox = PropWare::SPI::READ_SECTOR;
            this->wait();
            this->m_mailbox = (atomic_t) addr;
            if (blocking)
                return this->wait();
            else
                return PropWare::SPI::NO_ERROR;
        }
#endif
        /**
         * @brief       Print through UART an error string followed by entering
         *              an infinite loop
         *
         * @param[in]   *printer    Object used for printing error string
         * @param[in]   err         Error number used to determine error string
         */
        void print_error_str (const Printer *printer,
                const SPI::ErrorCode err) const {
            char str[] = "SPI Error %u: %s" CRLF;

            switch (err) {
                case PropWare::SPI::INVALID_PIN:
                    printer->printf(str, (err - PropWare::SPI::BEG_ERROR),
                            "Invalid pin");
                    break;
                case PropWare::SPI::INVALID_MODE:
                    printer->printf(str, (err - PropWare::SPI::BEG_ERROR),
                            "Invalid mode");
                    break;
                case PropWare::SPI::INVALID_PIN_MASK:
                    printer->printf(str, (err - PropWare::SPI::BEG_ERROR),
                            "Invalid pin mask");
                    break;
                case PropWare::SPI::TOO_MANY_BITS:
                    printer->printf(str, (err - PropWare::SPI::BEG_ERROR),
                            "Incapable of handling so many bits in an "
                                    "argument");
                    break;
                case PropWare::SPI::TIMEOUT:
                    printer->printf("SPI Error %u: %s\n\tCalling function was "
                            "SPI::%s()" CRLF, (err - PropWare::SPI::BEG_ERROR),
                            "Timed out during parameter passing",
                            this->m_errorInMethod);
                    break;
                case PropWare::SPI::TIMEOUT_RD:
                    printer->printf(str, (err - PropWare::SPI::BEG_ERROR),
                            "Timed out during parameter read");
                    break;
                case PropWare::SPI::COG_NOT_STARTED:
                    printer->printf(str, (err - PropWare::SPI::BEG_ERROR),
                            "SPI's GAS cog was not started");
                    break;
                case PropWare::SPI::MODULE_NOT_RUNNING:
                    printer->printf(str, (err - PropWare::SPI::BEG_ERROR),
                            "SPI GAS cog not running");
                    break;
                case PropWare::SPI::INVALID_FREQ:
                    printer->printf(str, (err - PropWare::SPI::BEG_ERROR),
                            "Frequency set too high");
                    break;
                case PropWare::SPI::ADDR_MISALIGN:
                    printer->printf(str, (err - PropWare::SPI::BEG_ERROR),
                            "Passed in address is miss aligned");
                    break;
                default:
                    // Is the error an SPI error?
                    if (err > PropWare::SPI::BEG_ERROR
                            && err
                                    < (PropWare::SPI::BEG_ERROR
                                            + PropWare::SPI::END_ERROR))
                        printer->printf("Unknown SPI error %u\n",
                                (err - PropWare::SPI::BEG_ERROR));
                    else
                        printer->printf("Unknown error %u\n", (err));
                    break;
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
        PropWare::ErrorCode read_par (void *par, const size_t size) {
            uint8_t *par8;
            uint16_t *par16;
            uint32_t *par32;
            const uint32_t timeoutCnt = PropWare::SPI::TIMEOUT_VAL + CNT;

            // Wait for a value to be written
            while (-1 == this->m_mailbox)
                if (abs(timeoutCnt - CNT) < PropWare::SPI::TIMEOUT_WIGGLE_ROOM)
                    return PropWare::SPI::TIMEOUT_RD;

            // Determine if output variable is char, short or long and write
            // data to that location
            switch (size) {
                case sizeof(uint8_t):
                    par8 = (uint8_t *) par;
                    *par8 = (uint8_t) this->m_mailbox;
                    break;
                case sizeof(uint16_t):
                    par16 = (uint16_t *) par;
                    *par16 = (uint16_t) this->m_mailbox;
                    break;
                case sizeof(uint32_t):
                    par32 = (uint32_t *) par;
                    *par32 = (uint32_t) this->m_mailbox;
                    break;
                default:
                    return PropWare::SPI::INVALID_BYTE_SIZE;
                    break;
            }

            // Signal that value is saved and GAS cog can continue execution
            this->m_mailbox = -1;

            return SPI::NO_ERROR;
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
