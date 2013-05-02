/* File:    spi.h
 * Author:  David Zemon
 * 
 * Description: Provides a library for the propeller, running in the current cog,
 * 				for SPI communication. Inspired by OBEX #433.
 */

#ifndef SPI_H_
#define SPI_H_

#include <propeller.h>
#include <gpio.h>
#include <bit_expansion.h>
#include <types.h>

/**
 * \brief	Extra code options - Uncomment definitions to enable features
 *
 * \param	SPI_DEBUG			Debugging features similar to exceptions; Errors will be
 * 								caught the program will enter an infinite loop
 * 								DEFAULT: OFF
 * \param	SPI_DEBUG_PARAMS	Parameter checking within each function call. I recommend
 * 								you leave this option enabled unless speed is critical
 * 								DEFAULT: ON
 * \param	SPI_FAST			Allows for fast send and receive routines without error
 * 								checking or timing delays; Normal routines still available
 * 								when enabled
 * 								DEFAULT: ON
 * 								TODO: Use the counter module instead of "xor clkPin, clkPin"
 * \param	SPI_FAST_SECTOR		TODO: Figure out why this doesn't work... :(
 * 								DEFAULT: OFF
 */
//#define SPI_DEBUG
//#define SPI_DEBUG_PARAMS
#define SPI_FAST
//#define SPI_FAST_SECTOR

#define	SPI_POLARITY_LOW			0
#define SPI_POLARITY_HIGH			1

#define SPI_MSB_PRE					0
#define SPI_LSB_PRE					1
#define SPI_MSB_POST				2
#define SPI_LSB_POST				3
#define SPI_LSB_FIRST				4
#define	SPI_MSB_FIRST				5

// (Default: CLKFREQ/10) Wait 0.1 seconds before throwing a timeout error
#define SPI_WR_TIMEOUT_VAL			CLKFREQ/10
#define SPI_RD_TIMEOUT_VAL			CLKFREQ/10
#define SPI_MAX_PAR_BITS			31

// Errors
#define SPI_ERRORS_BASE				1
#define SPI_ERRORS_LIMIT			16
#define SPI_INVALID_PIN				SPI_ERRORS_BASE + 0
#define	SPI_INVALID_CLOCK_INIT		SPI_ERRORS_BASE + 1
#define	SPI_INVALID_MODE			SPI_ERRORS_BASE + 2
#define SPI_INVALID_PIN_MASK		SPI_ERRORS_BASE + 3
#define SPI_TOO_MANY_BITS			SPI_ERRORS_BASE + 4
#define SPI_TIMEOUT					SPI_ERRORS_BASE + 5
#define SPI_TIMEOUT_RD				SPI_ERRORS_BASE + 6
#define SPI_EXCESSIVE_PAR_SZ		SPI_ERRORS_BASE + 7
#define SPI_COG_NOT_STARTED			SPI_ERRORS_BASE + 8
#define SPI_MODULE_NOT_RUNNING		SPI_ERRORS_BASE + 9
#define SPI_INVALID_FREQ			SPI_ERRORS_BASE + 10
#define SPI_INVALID_BYTE_SIZE		SPI_ERRORS_BASE + 11
#define SPI_ADDR_MISALIGN			SPI_ERRORS_BASE + 12

/**
 * \brief	Initialize an SPI module by starting a new cog
 *
 * \param	mosi		Pin mask for MOSI
 * \param	miso		Pin mask for MISO
 * \param	sclk		Pin mask for SCLK
 * \param	frequency	Frequency, in Hz, to run the SPI clock; Must be less than CLKFREQ/4
 * \param	polarity	Polarity of the clock - idle low or high; must be one of
 * 						SPI_POLARITY_LOW or SPI_POLARITY_HIGH
 *
 * \return		Returns 0 upon success, otherwise error code
 */
uint8 SPIStart (const uint32 mosi, const uint32 miso, const uint32 sclk,
		const uint32 frequency, const uint8 polarity);

/**
 * \brief	Stop a running SPI cog
 *
 * \return		Returns 0 upon success, otherwise error code (will return SPI_COG_NOT_STARTED
 * 				if no cog has previously been started)
 */
uint8 SPIStop (void);

/**
 * \brief	Wait for the SPI cog to signal that it is in the idle state
 *
 * \return		May return non-zero error code when a timeout occurs
 */
inline uint8 SPIWait (void);

/**
 * \brief		Send a value out to a peripheral device
 *
 * \detailed	Send pass a value and mode into the assembly cog to be sent out to the
 * 				peripheral; NOTE: this function is non-blocking and chip-select should
 * 				not be set inactive immediately after the return (you should call SPIWait()
 * 				before setting chip-select inactive)
 *
 * \param	bits		Number of bits to be shifted out
 * \param	value		The value to be shifted out
 * \param	mode		Controls whether the MSB or LSB is sent first; Must be one of
 * 						SPI_LSB_FIRST or SPI_MSB_FIRST
 *
 * \return		Returns 0 upon success, otherwise error code
 */
uint8 SPIShiftOut (uint8 bits, uint32 value, const uint8 mode);

/**
 * \brief	Receive a value in from a peripheral device
 *
 * \param	bits		Number of bits to be shifted in
 * \param	mode		Controls whether the MSB or LSB is sent first and whether data is
 * 						valid before or after the clock pulse; Must be one of SPI_MSB_PRE,
 * 						SPI_LSB_PRE, SPI_MSB_POST, or SPI_LSB_POST
 * \param	*data		Received data will be stored at this address
 * \param	bytes		Byte-width of the *data variable type; Must be one of 1, 2, or 4
 * 						(is *data a pointer	to char, short or int?)
 *
 * \return		Returns 0 upon success, otherwise error code
 */
uint8 SPIShiftIn (const uint8 bits, const uint8 mode, void *data, const uint8 bytes);

#ifdef SPI_FAST
/**
 * \brief	Receive a value in from a peripheral device; Optimized for fastest possible
 * 			clock speed; No error checking is performed; 'Timeout' event will never be
 * 			thrown and possible infinite loop can happen
 *
 * \param	bits		Number of bits to be shifted in
 * \param	mode		Controls whether the MSB or LSB is sent first and whether data is
 * 						valid before or after the clock pulse; Must be one of SPI_MSB_PRE,
 * 						SPI_LSB_PRE, SPI_MSB_POST, or SPI_LSB_POST
 * \param	*data		Received data will be stored at this address
 * \param	bytes		Byte-width of the *data variable type; Must be one of 1, 2, or 4
 * 						(is *data a pointer	to char, short or int?)
 */
void SPIShiftIn_fast (const uint8 bits, const uint8 mode, void *data, const uint8 bytes);

/**
 * \brief	Read an entire sector of data in from an SD card
 *
 * \param	*addr		First hub address where the data should be written
 * \param	blocking	When set to non-zero, function will not return until the data
 * 						transfer is complete
 */
void SPIShiftIn_sector (const uint8 addr[], const uint8 blocking);
#endif

/**
 * \brief	Change the SPI module's clock frequency
 *
 * \param	frequency	Frequency, in Hz, to run the SPI clock; Must be less than CLKFREQ/4
 *
 * \return		Returns 0 upon success, otherwise error code
 */
uint8 SPISetClock (const uint32 frequency);

#ifdef SPI_DEBUG
#include <stdio.h>
#include <stdarg.h>
/**
 * \brief	Print through UART an error string followed by entering an infinite loop
 *
 * \param	err		Error number used to determine error string
 */
void SPIError (const uint8 err, ...);
#else
// Exit calling function by returning 'err'
#define SPIError(err, ...)				return err
#endif

/*** Private definitions and Declarations ***/
// Defintions
#define	SPI_TIMEOUT_WIGGLE_ROOM		300
#define SPI_FUNC_SEND				0
#define	SPI_FUNC_READ				1
#define SPI_FUNC_CLK				2
#define SPI_FUNC_SEND_FAST			3
#define SPI_FUNC_READ_FAST			4
#define SPI_FUNC_READ_SECTOR		5
#define SPI_BITS_OFFSET				8
#define SPI_MODE_OFFSET				16

// Function prototypes
/**
 * \brief	Read the value that the SPI cog just shifted in
 *
 * \param	*par	Address to store the parameter
 * \param	bytes	Byte-width of the desired value
 *
 * \return		Returns 0 upon success, error code otherwise
 */
static inline uint8 SPIReadPar (void *par, const uint8 bytes);

/**
 * \brief	Count the number of set bits in a variable
 *
 * \param	par		Variable to count the bits in
 *
 * \return		Number of bits in the parameter par (no error checking)
 */
static uint8 SPICountBits (uint32 par);

/**
 * \brief	Retrieve the pin number from a pin mask; i.e., if pinMask is 0x01,
 *			return 0; if pinMask is 0x40, return 6
 *
 * \pre		Only 1 bit is set in pinMask (if more than one is set, the return value will
 *			be related to the least significant set bit)
 *
 * \param	pinMask		The bit number of the set bit in this variable will be returned
 *
 * \return		Returns the pin number of pinMask (no error checking)
 */
static uint8 SPIGetPinNum (const uint32 pinMask);

#endif /* SPI_H_ */
