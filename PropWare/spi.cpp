/**
 * @file    spi.cpp
 *
 * @project PropWare
 *
 * @author  David Zemon
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

// Includes
#include <PropWare/spi.h>

// Symbol for assembly instructions to start a new SPI cog
extern "C" {
extern uint32_t _SPIStartCog (void *arg);
}

// Constants that use CLKFREQ cannot be initialized in the header
const uint32_t PropWare::SPI::WR_TIMEOUT_VAL = CLKFREQ / 10;
const uint32_t PropWare::SPI::RD_TIMEOUT_VAL = CLKFREQ / 10;
const int32_t PropWare::SPI::MAX_CLOCK = CLKFREQ >> 3;

#define check_errors_w_str(x, y) \
    if ((err = x)) { \
        strcpy(this->m_errorInMethod, y); \
        return err;}

#ifndef PROPWARE_NO_SAFE_SPI
PropWare::SPI* PropWare::SPI::getInstance () {
    // TODO: Add a lock to make this thread-safe
    static SPI instance;
    return &instance;
}
#endif

PropWare::SPI::SPI () {
    this->m_mailbox = -1;
    this->m_cog = -1;
}

// Function definitions
PropWare::ErrorCode PropWare::SPI::start (const PropWare::Pin::Mask mosi,
        const PropWare::Pin::Mask miso, const PropWare::Pin::Mask sclk,
        const int32_t frequency, const SPI::Mode mode,
        const SPI::BitMode bitmode) {
    PropWare::ErrorCode err;
    const char str[] = "start";

#ifdef SPI_OPTION_DEBUG_PARAMS
    // Check clock frequency
    if (SPI::MAX_CLOCK <= frequency)
        return SPI::INVALID_FREQ;

    if (SPI::MODES <= mode)
        return SPI::INVALID_MODE;
    if (SPI::LSB_FIRST != bitmode && SPI::MSB_FIRST != bitmode)
        return SPI::INVALID_BITMODE;
#endif

    // If cog already started, do not start another
    if (!this->is_running()) {

        // Start GAS cog
        // Set the mailbox to 0 (anything other than -1) so that we know when
        // the SPI cog has started
        this->m_mailbox = 0;
        this->m_cog = _SPIStartCog((void *) &this->m_mailbox);
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

PropWare::ErrorCode PropWare::SPI::stop (void) {
    if (!this->is_running())
        return SPI::NO_ERROR;

    cogstop(this->m_cog);
    this->m_cog = -1;
    this->m_mailbox = -1;

    return SPI::NO_ERROR;
}

bool PropWare::SPI::is_running (void) {
    return -1 != this->m_cog;
}

PropWare::ErrorCode PropWare::SPI::wait (void) {
    const uint32_t timeoutCnt = SPI::WR_TIMEOUT_VAL + CNT;

    // Wait for GAS cog to read in value and write -1
    while ((uint32_t) -1 != this->m_mailbox)
        if (abs(timeoutCnt - CNT) < SPI::TIMEOUT_WIGGLE_ROOM)
            return SPI::TIMEOUT;

    return SPI::NO_ERROR;
}

PropWare::ErrorCode PropWare::SPI::wait_specific (const uint32_t value) {
    const uint32_t timeoutCnt = SPI::WR_TIMEOUT_VAL + CNT;

    // Wait for GAS cog to read in value and write -1
    while (value == this->m_mailbox)
        if (abs(timeoutCnt - CNT) < SPI::TIMEOUT_WIGGLE_ROOM)
            // Always use return instead of spi_error() for private functions
            return SPI::TIMEOUT;

    return SPI::NO_ERROR;
}

PropWare::ErrorCode PropWare::SPI::set_mode (const SPI::Mode mode) {
    PropWare::ErrorCode err;
    char str[] = "set_mode";

    if (!this->is_running())
        return SPI::MODULE_NOT_RUNNING;
#ifdef SPI_OPTION_DEBUG_PARAMS
    if (SPI::MODES <= mode)
        return SPI::INVALID_MODE;
#endif

    // Wait for SPI cog to go idle
    check_errors_w_str(this->wait(), str);

    this->m_mailbox = SPI::FUNC_SET_MODE;
    check_errors_w_str(this->wait(), str);
    this->m_mailbox = mode;

    return SPI::NO_ERROR;
}

PropWare::ErrorCode PropWare::SPI::set_bit_mode (const SPI::BitMode bitmode) {
    PropWare::ErrorCode err;
    char str[] = "set_bit_mode";

    if (!this->is_running())
        return SPI::MODULE_NOT_RUNNING;
#ifdef SPI_OPTION_DEBUG_PARAMS
    if (SPI::LSB_FIRST != bitmode && SPI::MSB_FIRST != bitmode)
        return SPI::INVALID_BITMODE;
#endif

    check_errors_w_str(this->wait(), str);
    this->m_mailbox = SPI::FUNC_SET_BITMODE;
    check_errors_w_str(this->wait(), str);
    this->m_mailbox = bitmode;

    return SPI::NO_ERROR;
}

PropWare::ErrorCode PropWare::SPI::set_clock (const int32_t frequency) {
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
    this->m_mailbox = SPI::FUNC_SET_FREQ;
    // Wait for the ready command
    check_errors_w_str(this->wait_specific(SPI::FUNC_SET_FREQ), str);
    // Send new frequency
    this->m_mailbox = (CLKFREQ / frequency) >> 1;

    return SPI::NO_ERROR;
}

PropWare::ErrorCode PropWare::SPI::get_clock (int32_t *frequency) {
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
    this->m_mailbox = SPI::FUNC_GET_FREQ;
    check_errors_w_str(this->wait_specific(SPI::FUNC_GET_FREQ), str);

    this->read_par(frequency, sizeof(*frequency));
    *frequency = CLKFREQ / (*frequency << 1);

    return SPI::NO_ERROR;
}

PropWare::ErrorCode PropWare::SPI::shift_out (uint8_t bits, uint32_t value) {
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
    this->m_mailbox = SPI::FUNC_SEND | (bits << SPI::BITS_OFFSET);
    check_errors_w_str(
            this->wait_specific(SPI::FUNC_SEND | (bits << SPI::BITS_OFFSET)),
            str);

    // Pass parameter in; Bit 31 is cleared to indicate data is being sent.
    // Without this limitation, who's to say the value being passed is not -1?
    this->m_mailbox = value & (~BIT_31);

    return SPI::NO_ERROR;
}

PropWare::ErrorCode PropWare::SPI::shift_in (const uint8_t bits, void *data, const size_t bytes) {
    PropWare::ErrorCode err;
    const char str[] = "shift_in";

    // Check for errors
#ifdef SPI_OPTION_DEBUG_PARAMS
    if (!this->is_running())
        return SPI::MODULE_NOT_RUNNING;
    if (SPI::MAX_PAR_BITS < bits)
        return SPI::TOO_MANY_BITS;
    if ((4 == bytes && ((uint32_t) data) % 4)
            || (2 == bytes && ((uint32_t) data) % 2))
        return SPI::ADDR_MISALIGN;
#endif

    // Ensure SPI module is not busy
    check_errors_w_str(this->wait(), str);

    // Call GAS function
    this->m_mailbox = SPI::FUNC_READ | (bits << SPI::BITS_OFFSET);

    // Read in parameter
    check_errors_w_str(this->read_par(data, bytes), str);

    return SPI::NO_ERROR;
}

#ifdef SPI_OPTION_FAST
PropWare::ErrorCode PropWare::SPI::shift_out_fast (uint8_t bits, uint32_t value) {
    // NOTE: No debugging within this function to allow for fastest possible
    // execution time
    // Wait to ensure the SPI cog is in its idle state
    this->wait();

    // Call GAS function
    this->m_mailbox = PropWare::SPI::FUNC_SEND_FAST
            | (bits << PropWare::SPI::BITS_OFFSET);
    this->wait();

    // Pass parameter in; Bit 31 is cleared to indicate data is being sent.
    // Without this limitation, who's to say the value being passed is not -1?
    this->m_mailbox = value & (~BIT_31);

    return PropWare::SPI::NO_ERROR;
}

PropWare::ErrorCode PropWare::SPI::shift_in_fast (const uint8_t bits,
        void *data, const uint8_t bytes) {
    uint8_t *par8;
    uint16_t *par16;
    uint32_t *par32;

    // Wait until idle state, then send function and mode bits
    this->wait();
    this->m_mailbox = PropWare::SPI::FUNC_READ_FAST
            | (bits << PropWare::SPI::BITS_OFFSET);

    // Wait for a value to be written
    while ((uint32_t) -1 == this->m_mailbox)
        waitcnt(PropWare::SPI::TIMEOUT_WIGGLE_ROOM + CNT);

    // Determine if output variable is char, short or long and write data to
    // that location
    switch (bytes) {
        case sizeof(uint8_t):
            par8 = (uint8_t *) data;
            *par8 = this->m_mailbox;
            break;
        case sizeof(uint16_t):
            par16 = (uint16_t *) data;
            *par16 = this->m_mailbox;
            break;
        case sizeof(uint32_t):
            par32 = (uint32_t *) data;
            *par32 = this->m_mailbox;
            break;
        default:
            return PropWare::SPI::INVALID_BYTE_SIZE;
    }

    // Signal that value is saved and GAS cog can continue execution
    this->m_mailbox = -1;

    return PropWare::SPI::NO_ERROR;
}

PropWare::ErrorCode PropWare::SPI::shift_in_sector (const uint8_t addr[],
        const uint8_t blocking) {
    this->wait();
    this->m_mailbox = PropWare::SPI::FUNC_READ_SECTOR;
    this->wait();
    this->m_mailbox = (uint32_t) addr;
    if (blocking)
        return this->wait();
    else
        return PropWare::SPI::NO_ERROR;
}
#endif

PropWare::ErrorCode PropWare::SPI::read_par (void *par, const size_t bytes) {
    uint8_t *par8;
    uint16_t *par16;
    uint32_t *par32;
    const uint32_t timeoutCnt = PropWare::SPI::WR_TIMEOUT_VAL + CNT;

    // Wait for a value to be written
    while ((uint32_t) -1 == this->m_mailbox)
        if (abs(timeoutCnt - CNT) < PropWare::SPI::TIMEOUT_WIGGLE_ROOM)
            return PropWare::SPI::TIMEOUT_RD;

    // Determine if output variable is char, short or long and write data to
    // that location
    switch (bytes) {
        case sizeof(uint8_t):
            par8 = (uint8_t *) par;
            *par8 = this->m_mailbox;
            break;
        case sizeof(uint16_t):
            par16 = (uint16_t *) par;
            *par16 = this->m_mailbox;
            break;
        case sizeof(uint32_t):
            par32 = (uint32_t *) par;
            *par32 = this->m_mailbox;
            break;
        default:
            return PropWare::SPI::INVALID_BYTE_SIZE;
            break;
    }

    // Signal that value is saved and GAS cog can continue execution
    this->m_mailbox = -1;

    return SPI::NO_ERROR;
}

void PropWare::SPI::print_error_str (const PropWare::SPI::ErrorCode err) const {
    char str[] = "SPI Error %u: %s\n";

    switch (err) {
        case PropWare::SPI::INVALID_PIN:
            printf(str, (err - PropWare::SPI::BEG_ERROR), "Invalid pin");
            break;
        case PropWare::SPI::INVALID_MODE:
            printf(str, (err - PropWare::SPI::BEG_ERROR), "Invalid mode");
            break;
        case PropWare::SPI::INVALID_PIN_MASK:
            printf(str, (err - PropWare::SPI::BEG_ERROR), "Invalid pin mask");
            break;
        case PropWare::SPI::TOO_MANY_BITS:
            printf(str, (err - PropWare::SPI::BEG_ERROR),
                    "Incapable of handling so many bits in an argument");
            break;
        case PropWare::SPI::TIMEOUT:
            printf("SPI Error %u: %s\n\tCalling function was SPI::%s()\n",
                    (err - PropWare::SPI::BEG_ERROR),
                    "Timed out during parameter passing",
                    this->m_errorInMethod);
            break;
        case PropWare::SPI::TIMEOUT_RD:
            printf(str, (err - PropWare::SPI::BEG_ERROR),
                    "Timed out during parameter read");
            break;
        case PropWare::SPI::COG_NOT_STARTED:
            printf(str, (err - PropWare::SPI::BEG_ERROR),
                    "SPI's GAS cog was not started");
            break;
        case PropWare::SPI::MODULE_NOT_RUNNING:
            printf(str, (err - PropWare::SPI::BEG_ERROR),
                    "SPI GAS cog not running");
            break;
        case PropWare::SPI::INVALID_FREQ:
            printf(str, (err - PropWare::SPI::BEG_ERROR),
                    "Frequency set too high");
            break;
        case PropWare::SPI::ADDR_MISALIGN:
            printf(str, (err - PropWare::SPI::BEG_ERROR),
                    "Passed in address is miss aligned");
            break;
        default:
            // Is the error an SPI error?
            if (err > PropWare::SPI::BEG_ERROR
                    && err <
                    (PropWare::SPI::BEG_ERROR + PropWare::SPI::END_ERROR))
                printf("Unknown SPI error %u\n",
                        (err - PropWare::SPI::BEG_ERROR));
            else
                printf("Unknown error %u\n", (err));
            break;
    }
}

