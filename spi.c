/**
 * @file    spi.c
 *
 * @author  David Zemon
 */

// Includes
#include <spi.h>

#ifdef SPI_DEBUG
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
#define SPIError(err, ...)				return err
#endif
#define PROPWARE_SPI_SAFETY_CHECK(x) if ((err = x)) SPIError(err)
#define PROPWARE_SPI_SAFETY_CHECK_STR(x, y) if ((err = x)) SPIError(err, y)

// Global variables
extern uint32_t _load_start_spi_as_cog[];
volatile static uint32_t g_mailbox = -1;
static int8_t g_spiCog = -1;

// Function definitions
uint8_t SPIStart (const uint32_t mosi, const uint32_t miso, const uint32_t sclk,
        const uint32_t frequency, const spimode_t mode,
        const spibitmode_t bitmode) {
    uint8_t err;
    const char str[11] = "SPIStart()";

#ifdef SPI_DEBUG_PARAMS
    // Ensure all pin-mask parameters have exactly 1 set bit
    if (1 != PropWareCountBits(mosi))
        SPIError(SPI_INVALID_PIN_MASK);
    if (1 != PropWareCountBits(miso))
        SPIError(SPI_INVALID_PIN_MASK);
    if (1 != PropWareCountBits(sclk))
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
    if (!SPIIsRunning()) {

        // Start GAS cog
        // Set the mailbox to 0 (anything other than -1) so that we know when
        // the SPI cog has started
        g_mailbox = 0;
        g_spiCog = cognew(_load_start_spi_as_cog, &g_mailbox);
        if (!SPIIsRunning())
            SPIError(SPI_COG_NOT_STARTED);

        // Pass in all parameters
        PROPWARE_SPI_SAFETY_CHECK_STR(SPIWait(), str);
        g_mailbox = mosi;
        PROPWARE_SPI_SAFETY_CHECK_STR(SPIWait(), str);
        g_mailbox = PropWareGetPinNum(mosi);
        PROPWARE_SPI_SAFETY_CHECK_STR(SPIWait(), str);
        g_mailbox = miso;
        PROPWARE_SPI_SAFETY_CHECK_STR(SPIWait(), str);
        g_mailbox = PropWareGetPinNum(miso);
        PROPWARE_SPI_SAFETY_CHECK_STR(SPIWait(), str);
        g_mailbox = sclk;
    }

    PROPWARE_SPI_SAFETY_CHECK_STR(SPISetMode(mode), str);
    PROPWARE_SPI_SAFETY_CHECK_STR(SPISetBitMode(bitmode), str);
    PROPWARE_SPI_SAFETY_CHECK_STR(SPISetClock(frequency), str);

    return 0;
}

uint8_t SPIStop (void) {
    if (!SPIIsRunning())
        return 0;

    cogstop(g_spiCog);
    g_spiCog = -1;
    g_mailbox = -1;

    return 0;
}

inline int8_t SPIIsRunning (void) {
    return !(((int8_t) -1) == g_spiCog);
}

inline uint8_t SPIWait (void) {
    const uint32_t timeoutCnt = SPI_WR_TIMEOUT_VAL + CNT;

    while ((uint32_t) -1 != g_mailbox) // Wait for GAS cog to read in value and write -1
        if (abs(timeoutCnt - CNT) < SPI_TIMEOUT_WIGGLE_ROOM)
            return SPI_TIMEOUT; // Always use return instead of SPIError() for private functions

    return 0;
}

inline uint8_t SPIWaitSpecific (const uint32_t value) {
    const uint32_t timeoutCnt = SPI_WR_TIMEOUT_VAL + CNT;

    while (value == g_mailbox) // Wait for GAS cog to read in value and write -1
        if (abs(timeoutCnt - CNT) < SPI_TIMEOUT_WIGGLE_ROOM)
            return SPI_TIMEOUT; // Always use return instead of SPIError() for private functions

    return 0;
}

uint8_t SPISetMode (const uint8_t mode) {
    uint8_t err;
    char str[14] = "SPISetMode()";

    if (!SPIIsRunning())
        SPIError(SPI_MODULE_NOT_RUNNING);
#ifdef SPI_DEBUG_PARAMS
    if (SPI_MODES <= mode)
        SPIError(SPI_INVALID_MODE);
#endif

    // Wait for SPI cog to go idle
    PROPWARE_SPI_SAFETY_CHECK_STR(SPIWait(), str);
    g_mailbox = SPI_FUNC_SET_MODE;
    PROPWARE_SPI_SAFETY_CHECK_STR(SPIWait(), str);
    g_mailbox = mode;

    return 0;
}

uint8_t SPISetBitMode (const uint8_t bitmode) {
    uint8_t err;
    char str[16] = "SPISetBitMode()";

    if (!SPIIsRunning())
        SPIError(SPI_MODULE_NOT_RUNNING);
#ifdef SPI_DEBUG_PARAMS
    if (SPI_LSB_FIRST != bitmode && SPI_MSB_FIRST != bitmode)
        SPIError(SPI_INVALID_BITMODE);
#endif

    PROPWARE_SPI_SAFETY_CHECK_STR(SPIWait(), str);
    g_mailbox = SPI_FUNC_SET_BITMODE;
    PROPWARE_SPI_SAFETY_CHECK_STR(SPIWait(), str);
    g_mailbox = bitmode;

    return 0;
}

uint8_t SPISetClock (const uint32_t frequency) {
    uint8_t err;
    char str[14] = "SPISetClock()";

    if (!SPIIsRunning())
        SPIError(SPI_MODULE_NOT_RUNNING);
#ifdef SPI_DEBUG_PARAMS
    if (SPI_MAX_CLOCK <= frequency)
        SPIError(SPI_INVALID_FREQ);
#endif

    // Wait for SPI cog to go idle
    PROPWARE_SPI_SAFETY_CHECK_STR(SPIWait(), str);
    // Prepare cog for clock frequency change
    g_mailbox = SPI_FUNC_SET_FREQ;
    // Wait for the ready command
    PROPWARE_SPI_SAFETY_CHECK_STR(SPIWaitSpecific(SPI_FUNC_SET_FREQ), str);
    // Send new frequency
    g_mailbox = CLKFREQ / frequency;

    return 0;
}

uint8_t SPIGetClock (uint32_t *frequency) {
    uint8_t err;
    char str[14] = "SPIGetClock()";

#ifdef SPI_DEBUG_PARAMS
    // Check for errors
    if (!SPIIsRunning())
        SPIError(SPI_MODULE_NOT_RUNNING);
#endif

    // Wait to ensure the SPI cog is in its idle state
    PROPWARE_SPI_SAFETY_CHECK_STR(SPIWait(), str);

    // Call GAS function
    g_mailbox = SPI_FUNC_GET_FREQ;
    PROPWARE_SPI_SAFETY_CHECK_STR(SPIWaitSpecific(SPI_FUNC_GET_FREQ), str);

    SPIReadPar(frequency, sizeof(*frequency));
    *frequency = CLKFREQ / *frequency;

    return 0;
}

uint8_t SPIShiftOut (uint8_t bits, uint32_t value) {
    uint8_t err;
    char str[14] = "SPIShiftOut()";

#ifdef SPI_DEBUG_PARAMS
    // Check for errors
    if (!SPIIsRunning())
        SPIError(SPI_MODULE_NOT_RUNNING);
    if (SPI_MAX_PAR_BITS < bits)
        SPIError(SPI_TOO_MANY_BITS);
#endif

    // Wait to ensure the SPI cog is in its idle state
    PROPWARE_SPI_SAFETY_CHECK_STR(SPIWait(), str);

    // Call GAS function
    g_mailbox = SPI_FUNC_SEND | (bits << SPI_BITS_OFFSET);
    PROPWARE_SPI_SAFETY_CHECK_STR(
            SPIWaitSpecific(SPI_FUNC_SEND | (bits << SPI_BITS_OFFSET)), str);

    // Pass parameter in; Bit 31 is cleared to indicate data is being sent. Without this limitation, who's to say the value being passed is not -1?
    g_mailbox = value & (~BIT_31);

    return 0;
}

uint8_t SPIShiftIn (const uint8_t bits, void *data, const size_t bytes) {
    uint8_t err;
    const char str[13] = "SPIShiftIn()";

    // Check for errors
#ifdef SPI_DEBUG_PARAMS
    if (!SPIIsRunning())
        SPIError(SPI_MODULE_NOT_RUNNING);
    if (SPI_MAX_PAR_BITS < bits)
        SPIError(SPI_TOO_MANY_BITS);
    if ((4 == bytes && ((uint32_t) data) % 4)
            || (2 == bytes && ((uint32_t) data) % 2))
        SPIError(SPI_ADDR_MISALIGN);
#endif

    // Ensure SPI module is not busy
    PROPWARE_SPI_SAFETY_CHECK_STR(SPIWait(), str);

    // Call GAS function
    g_mailbox = SPI_FUNC_READ | (bits << SPI_BITS_OFFSET);

    // Read in parameter
    PROPWARE_SPI_SAFETY_CHECK_STR(SPIReadPar(data, bytes), str);

    return 0;
}

#ifdef SPI_FAST
void SPIShiftOut_fast (uint8_t bits, uint32_t value) {
    // NOTE: No debugging within this function to allow for fastest possible
    // execution time
    // Wait to ensure the SPI cog is in its idle state
    SPIWait();

    // Call GAS function
    g_mailbox = SPI_FUNC_SEND_FAST | (bits << SPI_BITS_OFFSET);
    SPIWait();

    // Pass parameter in; Bit 31 is cleared to indicate data is being sent. Without this limitation, who's to say the value being passed is not -1?
    g_mailbox = value & (~BIT_31);
}

void SPIShiftIn_fast (const uint8_t bits, void *data, const uint8_t bytes) {
    uint8_t *par8;
    uint16_t *par16;
    uint32_t *par32;

    // Wait until idle state, then send function and mode bits
    SPIWait();
    g_mailbox = SPI_FUNC_READ_FAST | (bits << SPI_BITS_OFFSET);

    // Wait for a value to be written
    while ((uint32_t) -1 == g_mailbox)
        waitcnt(SPI_TIMEOUT_WIGGLE_ROOM + CNT);

    // Determine if output variable is char, short or long and write data to that location
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
#ifdef SPI_DEBUG
            SPIError(SPI_INVALID_BYTE_SIZE);
#else
            return;
#endif
            break;
    }

    // Signal that value is saved and GAS cog can continue execution
    g_mailbox = -1;
}

void SPIShiftIn_sector (const uint8_t addr[], const uint8_t blocking) {
    SPIWait();
    g_mailbox = SPI_FUNC_READ_SECTOR;
    SPIWait();
    g_mailbox = (uint32_t) addr;
    if (blocking)
        SPIWait();
}
#endif

static inline uint8_t SPIReadPar (void *par, const size_t bytes) {
    uint8_t *par8;
    uint16_t *par16;
    uint32_t *par32;
    const uint32_t timeoutCnt = SPI_WR_TIMEOUT_VAL + CNT;

    // Wait for a value to be written
    while ((uint32_t) -1 == g_mailbox)
        if (abs(timeoutCnt - CNT) < SPI_TIMEOUT_WIGGLE_ROOM)
            return SPI_TIMEOUT_RD;

    // Determine if output variable is char, short or long and write data to that location
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

#ifdef SPI_DEBUG
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
