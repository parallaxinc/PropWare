/**
 * @file    spi.h
 */
/**
 * @brief   Provides a library for the propeller, running in the current cog,
 *          for SPI communication
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

#ifndef SPI_H_
#define SPI_H_

/**
 * @defgroup _propware_spi          SPI Serial Communications
 * @{
 */

/**
 * @publicsection @{
 */

#include <propeller.h>
#include <stdlib.h>
#include <PropWare.h>

/** @name   SPI Extra Code Options
 * @{ */
/**
 * Debugging features similar to exceptions; Errors will be caught the program
 * will enter an infinite loop
 * <p>
 * DEFAULT: Off
 */
#define SPI_OPTION_DEBUG
// This allows Doxygen to document the macro without permanently enabling it
#undef SPI_OPTION_DEBUG
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

/**
 * @brief   Descriptor for SPI signal as defined by Motorola modes
 *
 * @detailed    CPOL 0 refers to a low polarity (where the clock idles in the
 *              low state) and CPOL 1 is for high polarity.
 *              TODO: Describe phase
 * <table><tr><td>SPI Mode</td><td>CPOL</td><td>CPHA</td></tr><tr><td>0</td>
 * <td>0</td><td>0</td></tr><tr><td>1</td><td>0</td><td>1</td></tr><tr><td>2
 * </td><td>1</td><td>0</td></tr><tr><td>3</td><td>1</td><td>1</td></tr></table>
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
    /** Mode 0 */SPI_MODE_0,
    /** Mode 1 */SPI_MODE_1,
    /** Mode 2 */SPI_MODE_2,
    /** Mode 3 */SPI_MODE_3,
    /** Number of SPI modes */SPI_MODES
} spimode_t;

/**
 * @brief   Determine if data is communicated with the LSB or MSB sent/received
 *          first
 *
 * @note    Initial value is SPI_MODES + 1 making them easily distinguishable
 */
typedef enum {
    // Start the enumeration where spimode_t left off; this ensures no overlap
    SPI_LSB_FIRST = SPI_MODES,
    SPI_MSB_FIRST,
    SPI_BIT_MODES
} spibitmode_t;

// (Default: CLKFREQ/10) Wait 0.1 seconds before throwing a timeout error
#define SPI_WR_TIMEOUT_VAL          CLKFREQ/10
#define SPI_RD_TIMEOUT_VAL          CLKFREQ/10
#define SPI_MAX_PAR_BITS            31
#define SPI_MAX_CLOCK               (CLKFREQ >> 2)

/** Number of allocated error codes for SPI */
#define SPI_ERRORS_LIMIT            16

/**
 * Error codes - Proceeded by nothing
 */
typedef enum {
    /** SPI Error  0 */SPI_INVALID_PIN = 0,
    /** SPI Error  1 */SPI_INVALID_CLOCK_INIT,
    /** SPI Error  2 */SPI_INVALID_MODE,
    /** SPI Error  3 */SPI_INVALID_PIN_MASK,
    /** SPI Error  4 */SPI_TOO_MANY_BITS,
    /** SPI Error  5 */SPI_TIMEOUT,
    /** SPI Error  6 */SPI_TIMEOUT_RD,
    /** SPI Error  7 */SPI_EXCESSIVE_PAR_SZ,
    /** SPI Error  8 */SPI_COG_NOT_STARTED,
    /** SPI Error  9 */SPI_MODULE_NOT_RUNNING,
    /** SPI Error 10 */SPI_INVALID_FREQ,
    /** SPI Error 11 */SPI_INVALID_BYTE_SIZE,
    /** SPI Error 12 */SPI_ADDR_MISALIGN,
    /** SPI Error 13 */SPI_INVALID_BITMODE
} spi_error_code_t;

/**
 * @brief       Initialize an SPI module by starting a new cog
 *
 * @param[in]   mosi        Pin mask for MOSI
 * @param[in]   miso        Pin mask for MISO
 * @param[in]   sclk        Pin mask for SCLK
 * @param[in]   frequency   Frequency, in Hz, to run the SPI clock; Must be less
 *                          than CLKFREQ/4
 * @param[in]   polarity    Polarity of the clock - idle low or high; must be
 *                          one of SPI_POLARITY_LOW or SPI_POLARITY_HIGH
 *
 * @return      Returns 0 upon success, otherwise error code
 */
uint8_t SPIStart (const uint32_t mosi, const uint32_t miso, const uint32_t sclk,
        const uint32_t frequency, const spimode_t mode,
        const spibitmode_t bitmode);

/**
 * @brief   Stop a running SPI cog
 *
 * @return  Returns 0 upon success, otherwise error code (will return
 *          SPI_COG_NOT_STARTED if no cog has previously been started)
 */
uint8_t SPIStop (void);

/**
 * @brief    Determine if the SPI cog has already been initialized
 *
 * @return       Returns 1 if the SPI cog is up and running, 0 otherwise
 */
inline int8_t SPIIsRunning (void);

/**
 * @brief   Wait for the SPI cog to signal that it is in the idle state
 *
 * @return  May return non-zero error code when a timeout occurs
 */
inline uint8_t SPIWait (void);

/**
 * @brief       Set the mode of SPI communication
 *
 * @param[in]   mode    Sets the SPI mode to one SPI_MODE_0, SPI_MODE_1,
 *                      SPI_MODE_2, or SPI_MODE_3
 *
 * @return      Can return non-zero in the case of a timeout
 */
uint8_t SPISetMode (const spimode_t mode);

/**
 * @brief       Set the bitmode of SPI communication
 *
 * @param[in]   mode    Select one of SPI_LSB_FIRST or SPI_MSB_FIRST to choose
 *                      which bit will be shifted out first
 *
 * @return      Can return non-zero in the case of a timeout
 */
uint8_t SPISetBitMode (const spibitmode_t bitmode);

/**
 * @brief       Change the SPI module's clock frequency
 *
 * @param[in]   frequency   Frequency, in Hz, to run the SPI clock; Must be less
 *                          than CLKFREQ/4 (for 80 MHz, 1.9 MHz is the fastest
 *                          I've tested successfully)
 *
 * @return      Returns 0 upon success, otherwise error code
 */
uint8_t SPISetClock (const uint32_t frequency);

/**
 * @brief       Retrieve the SPI module's clock frequency
 *
 * @param[out]  *frequency  Frequency, in Hz, that the SPI object is running
 *
 * @return      Returns 0 upon success, otherwise error code
 */
uint8_t SPIGetClock (uint32_t *frequency);

/**
 * @brief       Send a value out to a peripheral device
 *
 * @detailed    Pass a value and mode into the assembly cog to be sent to the
 *              peripheral; NOTE: this function is non-blocking and chip-select
 *              should not be set inactive immediately after the return (you
 *              should call SPIWait() before setting chip-select inactive)
 *
 * @param[in]   bits        Number of bits to be shifted out
 * @param[in]   value       The value to be shifted out
 *
 * @return      Returns 0 upon success, otherwise error code
 */
uint8_t SPIShiftOut (uint8_t bits, uint32_t value);

/**
 * @brief       Receive a value in from a peripheral device
 *
 * @param[in]   bits        Number of bits to be shifted in
 * @param[out]  *data       Received data will be stored at this address
 * @param[in]   bytes       Number of bytes allocated to *data; Example:
 *                              int newVal;
 *                              SPIShiftIn(8, &newVal, sizeof(newVal));
 *                          Or if using a pointer:
 *                              int *newVal;
 *                              SPIShiftIn(8, newVal, sizeof(*newVal));
 *
 * @return      Returns 0 upon success, otherwise error code
 */
uint8_t SPIShiftIn (const uint8_t bits, void *data, const size_t size);

#ifdef SPI_OPTION_FAST
/**
 * @brief       Send a value out to a peripheral device
 *
 * @detailed    Pass a value and mode into the assembly cog to be sent to the
 *              peripheral; NOTE: this function is non-blocking and chip-select
 *              should not be set inactive immediately after the return (you
 *              should call SPIWait() before setting chip-select inactive);
 *              Optimized for fastest possible clock speed; No error checking is
 *              performed; 'Timeout' event will never be thrown and possible
 *              infinite loop can happen
 *
 * @param[in]   bits        Number of bits to be shifted out
 * @param[in]   value       The value to be shifted out
 *
 * @return      Returns 0 upon success, otherwise error code
 */
void SPIShiftOut_fast (uint8_t bits, uint32_t value);

/**
 * @brief       Receive a value in from a peripheral device; Optimized for
 *              fastest possible clock speed; No error checking is performed;
 *              'Timeout' event will never be thrown and possible infinite loop
 *              can happen
 *
 * @param[in]   bits    Number of bits to be shifted in
 * @param[out]  *data   Received data will be stored at this address
 * @param[in]   bytes   Number of bytes allocated to *data; Example:
 *                          int newVal;
 *                          SPIShiftIn_fast(8, &newVal, sizeof(newVal));
 *                      Or if using a pointer:
 *                          int *newVal;
 *                          SPIShiftIn_fast(8, newVal, sizeof(*newVal));
 */
void SPIShiftIn_fast (const uint8_t bits, void *data, const uint8_t bytes);

/**
 * @brief       Read an entire sector of data in from an SD card
 *
 * @param[out]  *addr       First hub address where the data should be written
 * @param[in]   blocking    When set to non-zero, function will not return until
 *                          the data transfer is complete
 */
int8_t SPIShiftIn_sector (const uint8_t addr[], const uint8_t blocking);
#endif

/**@}*/

/********************************************
 *** Private definitions and Declarations ***
 ********************************************/
/**
 * @privatesection @{
 */
#define SPI_TIMEOUT_WIGGLE_ROOM     400
#define SPI_FUNC_SEND               0
#define SPI_FUNC_READ               1
#define SPI_FUNC_SEND_FAST          2
#define SPI_FUNC_READ_FAST          3
#define SPI_FUNC_READ_SECTOR        4
#define SPI_FUNC_SET_MODE           5
#define SPI_FUNC_SET_BITMODE        6
#define SPI_FUNC_SET_FREQ           7
#define SPI_FUNC_GET_FREQ           8

#define SPI_BITS_OFFSET             8

#define SPI_PHASE_BIT               BIT_0
#define SPI_POLARITY_BIT            BIT_1 // Idle high == HIGH; Idle low == LOW
#define SPI_BITMODE_BIT             BIT_2 // MSB_FIRST == HIGH; LSB_FIRST == LOW
/**
 * @brief       Read the value that the SPI cog just shifted in
 *
 * @param[out]  *par    Address to store the parameter
 * @param[in]   bytes   Number of bytes allocated to *data; Example:
 *                          int newVal;
 *                          SPIReadPar(&newVal, sizeof(newVal));
 *                      Or if using a pointer:
 *                          int *newVal;
 *                          SPIReadPar(newVal, sizeof(*newVal));
 *
 * @return      Returns 0 upon success, error code otherwise
 */
static inline uint8_t SPIReadPar (void *par, const size_t size);

/**
 * @brief       Count the number of set bits in a variable
 *
 * @param[in]   par     Variable to count the bits in
 *
 * @return      Number of bits in the parameter par (no error checking)
 */
static uint8_t SPICountBits (uint32_t par);

/**
 * @brief       Retrieve the pin number from a pin mask; i.e., if pinMask is
 *              0x01, return 0; if pinMask is 0x40, return 6
 *
 * @pre         Only 1 bit is set in pinMask (if more than one is set, the
 *              return value will be related to the least significant set bit)
 *
 * @param[in]   pinMask     The bit number of the set bit in this variable will
 *                          be returned
 *
 * @return      Returns the pin number of pinMask (no error checking)
 */
static uint8_t SPIGetPinNum (const uint32_t pinMask);

/**@}*/

/**@}*/

#endif /* SPI_H_ */
