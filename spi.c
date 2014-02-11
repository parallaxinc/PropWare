/**
 * @file    spi.c
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
#include <spi.h>

#ifdef SPI_OPTION_DEBUG
#include <stdio.h>
#include <stdarg.h>
/**
 * @brief   Print through UART an error string followed by entering an infinite
 *          loop
 *
 * @param   err     Error number used to determine error string
 */
static void SPIError (const uint8_t err, ...);
#else
// Exit calling function by returning 'err'
#define SPIError(err, ...)          return err
#endif
#define PROPWARE_SPI_SAFETY_CHECK(x) if ((err = x)) SPIError(err)
#define PROPWARE_SPI_SAFETY_CHECK_STR(x, y) if ((err = x)) SPIError(err, y)

// Global variables
extern uint32_t _SPIStartCog(void *arg);
volatile static uint32_t g_mailbox = -1;
static int8_t g_spiCog = -1;

// Function definitions
uint8_t spi_start (const uint32_t mosi, const uint32_t miso, const uint32_t sclk,
        const uint32_t frequency, const SPI_Mode mode,
        const SPI_BitMode bitmode) {
    uint8_t err;
    const char str[11] = "spi_start()";

#ifdef SPI_OPTION_DEBUG_PARAMS
    // Ensure all pin-mask parameters have exactly 1 set bit
    if (1 != propware_count_bits(mosi))
        SPIError(SPI_INVALID_PIN_MASK);
    if (1 != propware_count_bits(miso))
        SPIError(SPI_INVALID_PIN_MASK);
    if (1 != propware_count_bits(sclk))
        SPIError(SPI_INVALID_PIN_MASK);

    // Check clock frequency
    if (SPI_MAX_CLOCK <= frequency)
        SPIError(SPI_INVALID_FREQ);

    if (SPI_MODES <= mode)
        SPIError(SPI_INVALID_MODE);
    if (SPI_LSB_FIRST != bitmode && SPI_MSB_FIRST != bitmode)
        SPIError(SPI_INVALID_BITMODE);
#endif

    // If cog already started, do not start another
    if (!spi_is_running()) {

        // Start GAS cog
        // Set the mailbox to 0 (anything other than -1) so that we know when
        // the SPI cog has started
        g_mailbox = 0;
        g_spiCog = _SPIStartCog((void *) &g_mailbox);
        if (!spi_is_running())
            SPIError(SPI_COG_NOT_STARTED);

        // Pass in all parameters
        PROPWARE_SPI_SAFETY_CHECK_STR(spi_wait(), str);
        g_mailbox = mosi;
        PROPWARE_SPI_SAFETY_CHECK_STR(spi_wait(), str);
        g_mailbox = propware_get_pin_num(mosi);
        PROPWARE_SPI_SAFETY_CHECK_STR(spi_wait(), str);
        g_mailbox = miso;
        PROPWARE_SPI_SAFETY_CHECK_STR(spi_wait(), str);
        g_mailbox = propware_get_pin_num(miso);
        PROPWARE_SPI_SAFETY_CHECK_STR(spi_wait(), str);
        g_mailbox = sclk;
    }

    PROPWARE_SPI_SAFETY_CHECK_STR(spi_set_mode(mode), str);
    PROPWARE_SPI_SAFETY_CHECK_STR(spi_set_bit_mode(bitmode), str);
    PROPWARE_SPI_SAFETY_CHECK_STR(spi_set_clock(frequency), str);

    return 0;
}

uint8_t spi_stop (void) {
    if (!spi_is_running())
        return 0;

    cogstop(g_spiCog);
    g_spiCog = -1;
    g_mailbox = -1;

    return 0;
}

inline int8_t spi_is_running (void) {
    return !(((int8_t) -1) == g_spiCog);
}

inline uint8_t spi_wait (void) {
    const uint32_t timeoutCnt = SPI_WR_TIMEOUT_VAL + CNT;

    // Wait for GAS cog to read in value and write -1
    while ((uint32_t) -1 != g_mailbox)
        if (abs(timeoutCnt - CNT) < SPI_TIMEOUT_WIGGLE_ROOM)
            // Always use return instead of SPIError() for private functions
            return SPI_TIMEOUT;

    return 0;
}

inline uint8_t spi_wait_specific (const uint32_t value) {
    const uint32_t timeoutCnt = SPI_WR_TIMEOUT_VAL + CNT;

    // Wait for GAS cog to read in value and write -1
    while (value == g_mailbox)
        if (abs(timeoutCnt - CNT) < SPI_TIMEOUT_WIGGLE_ROOM)
            // Always use return instead of SPIError() for private functions
            return SPI_TIMEOUT;

    return 0;
}

uint8_t spi_set_mode (const SPI_Mode mode) {
    uint8_t err;
    char str[15] = "spi_set_mode()";

    if (!spi_is_running())
        SPIError(SPI_MODULE_NOT_RUNNING);
#ifdef SPI_OPTION_DEBUG_PARAMS
    if (SPI_MODES <= mode)
        SPIError(SPI_INVALID_MODE);
#endif

    // Wait for SPI cog to go idle
    PROPWARE_SPI_SAFETY_CHECK_STR(spi_wait(), str);

    g_mailbox = SPI_FUNC_SET_MODE;
    PROPWARE_SPI_SAFETY_CHECK_STR(spi_wait(), str);
    g_mailbox = mode;

    return 0;
}

uint8_t spi_set_bit_mode (const SPI_BitMode bitmode) {
    uint8_t err;
    char str[19] = "spi_set_bit_mode()";

    if (!spi_is_running())
        SPIError(SPI_MODULE_NOT_RUNNING);
#ifdef SPI_OPTION_DEBUG_PARAMS
    if (SPI_LSB_FIRST != bitmode && SPI_MSB_FIRST != bitmode)
        SPIError(SPI_INVALID_BITMODE);
#endif

    PROPWARE_SPI_SAFETY_CHECK_STR(spi_wait(), str);
    g_mailbox = SPI_FUNC_SET_BITMODE;
    PROPWARE_SPI_SAFETY_CHECK_STR(spi_wait(), str);
    g_mailbox = bitmode;

    return 0;
}

uint8_t spi_set_clock (const uint32_t frequency) {
    uint8_t err;
    char str[16] = "spi_set_clock()";

    if (!spi_is_running())
        SPIError(SPI_MODULE_NOT_RUNNING);
#ifdef SPI_OPTION_DEBUG_PARAMS
    if (SPI_MAX_CLOCK <= frequency)
        SPIError(SPI_INVALID_FREQ);
#endif

    // Wait for SPI cog to go idle
    PROPWARE_SPI_SAFETY_CHECK_STR(spi_wait(), str);
    // Prepare cog for clock frequency change
    g_mailbox = SPI_FUNC_SET_FREQ;
    // Wait for the ready command
    PROPWARE_SPI_SAFETY_CHECK_STR(spi_wait_specific(SPI_FUNC_SET_FREQ), str);
    // Send new frequency
    g_mailbox = CLKFREQ / frequency;

    return 0;
}

uint8_t spi_get_clock (uint32_t *frequency) {
    uint8_t err;
    char str[16] = "spi_get_clock()";

#ifdef SPI_OPTION_DEBUG_PARAMS
    // Check for errors
    if (!spi_is_running())
        SPIError(SPI_MODULE_NOT_RUNNING);
#endif

    // Wait to ensure the SPI cog is in its idle state
    PROPWARE_SPI_SAFETY_CHECK_STR(spi_wait(), str);

    // Call GAS function
    g_mailbox = SPI_FUNC_GET_FREQ;
    PROPWARE_SPI_SAFETY_CHECK_STR(spi_wait_specific(SPI_FUNC_GET_FREQ), str);

    spi_read_par(frequency, sizeof(*frequency));
    *frequency = CLKFREQ / *frequency;

    return 0;
}

uint8_t spi_shift_out (uint8_t bits, uint32_t value) {
    uint8_t err;
    char str[16] = "spi_shift_out()";

#ifdef SPI_OPTION_DEBUG_PARAMS
    // Check for errors
    if (!spi_is_running())
        SPIError(SPI_MODULE_NOT_RUNNING);
    if (SPI_MAX_PAR_BITS < bits)
        SPIError(SPI_TOO_MANY_BITS);
#endif

    // Wait to ensure the SPI cog is in its idle state
    PROPWARE_SPI_SAFETY_CHECK_STR(spi_wait(), str);

    // Call GAS function
    g_mailbox = SPI_FUNC_SEND | (bits << SPI_BITS_OFFSET);
    PROPWARE_SPI_SAFETY_CHECK_STR(
            spi_wait_specific(SPI_FUNC_SEND | (bits << SPI_BITS_OFFSET)), str);

    // Pass parameter in; Bit 31 is cleared to indicate data is being sent.
    // Without this limitation, who's to say the value being passed is not -1?
    g_mailbox = value & (~BIT_31);

    return 0;
}

uint8_t spi_shift_in (const uint8_t bits, void *data, const size_t bytes) {
    uint8_t err;
    const char str[16] = "spi_shift_in()";

    // Check for errors
#ifdef SPI_OPTION_DEBUG_PARAMS
    if (!spi_is_running())
        SPIError(SPI_MODULE_NOT_RUNNING);
    if (SPI_MAX_PAR_BITS < bits)
        SPIError(SPI_TOO_MANY_BITS);
    if ((4 == bytes && ((uint32_t) data) % 4)
            || (2 == bytes && ((uint32_t) data) % 2))
        SPIError(SPI_ADDR_MISALIGN);
#endif

    // Ensure SPI module is not busy
    PROPWARE_SPI_SAFETY_CHECK_STR(spi_wait(), str);

    // Call GAS function
    g_mailbox = SPI_FUNC_READ | (bits << SPI_BITS_OFFSET);

    // Read in parameter
    PROPWARE_SPI_SAFETY_CHECK_STR(spi_read_par(data, bytes), str);

    return 0;
}

#ifdef SPI_OPTION_FAST
void spi_shift_out_fast (uint8_t bits, uint32_t value) {
    // NOTE: No debugging within this function to allow for fastest possible
    // execution time
    // Wait to ensure the SPI cog is in its idle state
    spi_wait();

    // Call GAS function
    g_mailbox = SPI_FUNC_SEND_FAST | (bits << SPI_BITS_OFFSET);
    spi_wait();

    // Pass parameter in; Bit 31 is cleared to indicate data is being sent.
    // Without this limitation, who's to say the value being passed is not -1?
    g_mailbox = value & (~BIT_31);
}

void spi_shift_in_fast (const uint8_t bits, void *data, const uint8_t bytes) {
    uint8_t *par8;
    uint16_t *par16;
    uint32_t *par32;

    // Wait until idle state, then send function and mode bits
    spi_wait();
    g_mailbox = SPI_FUNC_READ_FAST | (bits << SPI_BITS_OFFSET);

    // Wait for a value to be written
    while ((uint32_t) -1 == g_mailbox)
        waitcnt(SPI_TIMEOUT_WIGGLE_ROOM + CNT);

    // Determine if output variable is char, short or long and write data to
    // that location
    switch (bytes) {
        case 1:
            par8 = data;
            *par8 = g_mailbox;
            break;
        case 2:
            par16 = data;
            *par16 = g_mailbox;
            break;
        case 4:
            par32 = data;
            *par32 = g_mailbox;
            break;
        default:
#ifdef SPI_OPTION_DEBUG
            SPIError(SPI_INVALID_BYTE_SIZE);
#else
            return;
#endif
            break;
    }

    // Signal that value is saved and GAS cog can continue execution
    g_mailbox = -1;
}

int8_t spi_shift_in_sector (const uint8_t addr[], const uint8_t blocking) {
    int8_t err;

    spi_wait();
    g_mailbox = SPI_FUNC_READ_SECTOR;
    spi_wait();
    g_mailbox = (uint32_t) addr;
    if (blocking)
        PROPWARE_SPI_SAFETY_CHECK(spi_wait());

    return 0;
}
#endif

static inline uint8_t spi_read_par (void *par, const size_t bytes) {
    uint8_t *par8;
    uint16_t *par16;
    uint32_t *par32;
    const uint32_t timeoutCnt = SPI_WR_TIMEOUT_VAL + CNT;

    // Wait for a value to be written
    while ((uint32_t) -1 == g_mailbox)
        if (abs(timeoutCnt - CNT) < SPI_TIMEOUT_WIGGLE_ROOM)
            return SPI_TIMEOUT_RD;

    // Determine if output variable is char, short or long and write data to
    // that location
    switch (bytes) {
        case sizeof(uint8_t):
            par8 = par;
            *par8 = g_mailbox;
            break;
        case sizeof(uint16_t):
            par16 = par;
            *par16 = g_mailbox;
            break;
        case sizeof(uint32_t):
            par32 = par;
            *par32 = g_mailbox;
            break;
        default:
            SPIError(SPI_INVALID_BYTE_SIZE);
            break;
    }

    // Signal that value is saved and GAS cog can continue execution
    g_mailbox = -1;

    return 0;
}

#ifdef SPI_OPTION_DEBUG
void SPIError (const uint8_t err, ...) {
    va_list list;
    char str[18] = "SPI Error %u: %s\n";

    switch (err) {
        case SPI_INVALID_PIN:
        __simple_printf(str, (err - SPI_ERRORS_BASE), "Invalid pin");
        break;
        case SPI_INVALID_MODE:
        __simple_printf(str, (err - SPI_ERRORS_BASE), "Invalid mode");
        break;
        case SPI_INVALID_PIN_MASK:
        __simple_printf(str, (err - SPI_ERRORS_BASE), "Invalid pin mask");
        break;
        case SPI_TOO_MANY_BITS:
        __simple_printf(str, (err - SPI_ERRORS_BASE),
                "Incapable of handling so many bits in an argument");
        break;
        case SPI_TIMEOUT:
        va_start(list, err);
        __simple_printf("SPI Error %u: %s\n\tCalling function was %s\n",
                (err - SPI_ERRORS_BASE),
                "Timed out during parameter passing", va_arg(list, char *));
        va_end(list);
        break;
        case SPI_TIMEOUT_RD:
        __simple_printf(str, (err - SPI_ERRORS_BASE),
                "Timed out during parameter read");
        break;
        case SPI_COG_NOT_STARTED:
        __simple_printf(str, (err - SPI_ERRORS_BASE),
                "SPI's GAS cog was not started");
        break;
        case SPI_MODULE_NOT_RUNNING:
        __simple_printf(str, (err - SPI_ERRORS_BASE),
                "SPI GAS cog not running");
        break;
        case SPI_INVALID_FREQ:
        __simple_printf(str, (err - SPI_ERRORS_BASE),
                "Frequency set too high");
        break;
        case SPI_ADDR_MISALIGN:
        __simple_printf(str, (err - SPI_ERRORS_BASE),
                "Passed in address is miss aligned");
        break;
        default:
        // Is the error an SPI error?
        if (err > SPI_ERRORS_BASE
                && err < (SPI_ERRORS_BASE + SPI_ERRORS_LIMIT))
        __simple_printf("Unknown SPI error %u\n",
                (err - SPI_ERRORS_BASE));
        else
        __simple_printf("Unknown error %u\n", (err));
        break;
    }
    while (1)
    ;
}
#endif
