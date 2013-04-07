/* File:    spi.c
 * 
 * Author:  David Zemon
 */

// Includes
#include <spi.h>

// Private Definitions
#define	SPI_TIMEOUT_WIGGLE_ROOM		300
#define SPI_FUNC_SEND				0
#define	SPI_FUNC_READ				1
#define SPI_FUNC_CLK				2
#define SPI_BITS_OFFSET				8
#define SPI_MODE_OFFSET				16

extern uint32 _load_start_spi_as_cog[];
static uint32 g_mailbox = -1;
uint8 g_spiCog = -1;

// Private Function Declarations
inline uint8 SPIPassPar (const uint32 par);
inline uint8 SPIReadPar (void *par, const uint8 bytes);
uint8 SPICountBits (uint32 par);
uint8 SPIGetPinNum (const uint32 pinMask);

uint8 SPIStart (const uint32 mosi, const uint32 miso, const uint32 sclk,
		const uint32 frequency, const uint8 polarity) {
	uint8 err;
	const char str[] = "SPIStart()";

	// If cog already started, do not start another
	if (((uint8) -1) != g_spiCog)
		return 0;

	// Ensure all pin-mask parameters have exactly 1 set bit
	if (1 != SPICountBits(mosi))
		SPIError(SPI_INVALID_PIN_MASK);
	if (1 != SPICountBits(miso))
		SPIError(SPI_INVALID_PIN_MASK);
	if (1 != SPICountBits(sclk))
		SPIError(SPI_INVALID_PIN_MASK);

	// Check clock frequency
	if (CLKFREQ / 4 <= frequency)
		SPIError(SPI_INVALID_FREQ);

	// Start GAS cog
	g_spiCog = cognew(_load_start_spi_as_cog, &g_mailbox);
	if (8 <= g_spiCog)
		SPIError(SPI_COG_NOT_STARTED);

	// Pass in all parameters
	if (err = SPIPassPar(mosi))
		SPIError(err, str);
	if (err = SPIPassPar(SPIGetPinNum(mosi)))
		SPIError(err, str);
	if (err = SPIPassPar(miso))
		SPIError(err, str);
	if (err = SPIPassPar(SPIGetPinNum(miso)))
		SPIError(err, str);
	if (err = SPIPassPar(sclk))
		SPIError(err, str);
	if (err = SPIPassPar(CLKFREQ / frequency))
		SPIError(err, str);
	switch (polarity) {							// Assign clock polarity
		case SPI_POLARITY_HIGH:
			if (err = SPIPassPar(sclk))
				SPIError(err, str);
			break;
		case SPI_POLARITY_LOW:
			if (err = SPIPassPar(0))
				SPIError(err, str);
			break;
		default:
			SPIError(SPI_INVALID_CLOCK_INIT);
			break;
	}

	return 0;
}

uint8 SPIStop (void) {
	if (-1 == g_spiCog)
		SPIError(SPI_COG_NOT_STARTED);

	cogstop(g_spiCog);
	g_spiCog = -1;
	g_mailbox = -1;

	return 0;
}

inline uint8 SPIPassPar (const uint32 par) {
	const uint32 timeoutCnt = SPI_WR_TIMEOUT_VAL + CNT;

	g_mailbox = par;					// Pass parameter in
	while (-1 != g_mailbox) {			// Wait for GAS cog to read in value and write -1
		waitcnt(SPI_TIMEOUT_WIGGLE_ROOM + CNT);

		if ((timeoutCnt - CNT) < SPI_TIMEOUT_WIGGLE_ROOM)
			return SPI_TIMEOUT_WR;// Always use return instead of SPIError() for private functions
	}

	return 0;
}

inline uint8 SPIReadPar (void *par, const uint8 bytes) {
	uint8 *par8;
	uint16 *par16;
	uint32 *par32;
	const uint32 timeoutCnt = SPI_RD_TIMEOUT_VAL + CNT;

	// Wait for a value to be written
	while (-1 == g_mailbox) {
		waitcnt(SPI_TIMEOUT_WIGGLE_ROOM + CNT);

		if ((timeoutCnt - CNT) < SPI_TIMEOUT_WIGGLE_ROOM)
			return SPI_TIMEOUT_RD;
	}

	// Determine if output variable is char, short or long and write data to that location
	switch (bytes) {
		case 1:
			par8 = par;
			*par8 = g_mailbox;
			break;
		case 2:
			par16 = par;
			*par16 = g_mailbox;
			break;
		case 4:
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

uint8 SPICountBits (uint32 par) {
	/* Brian Kernighan's method for counting set bits in a variable */
	uint32 c;					// c accumulates the total bits set in par
	for (c = 0; par; ++c)
		par &= par - 1;				// clear the least significant bit set

	return c;
}

uint8 SPIGetPinNum (const uint32 pinMask) {
	uint8 temp = 0;
	while (!(0x01 & (pinMask >> temp++)))
		;
	return --temp;
}

uint8 SPIShiftOut (uint8 bits, uint32 value, const uint8 mode) {
	uint8 err;
	const char str[] = "SPIShiftOut()";

	// Check for errors
	if (8 <= g_spiCog)
		SPIError(SPI_MODULE_NOT_RUNNING);
	if (SPI_MAX_PAR_BITS < bits)
		SPIError(SPI_TOO_MANY_BITS);
	if ((SPI_LSB_FIRST != mode) && (SPI_MSB_FIRST != mode))
		SPIError(SPI_INVALID_MODE);

	// Call GAS function
	if (err = SPIPassPar(
			SPI_FUNC_SEND | (bits << SPI_BITS_OFFSET) | (mode << SPI_MODE_OFFSET)))
		SPIError(err, str);
	if (err = SPIPassPar(value & (~BIT_31)))// Bit 31 is cleared to indicate data is being sent. Without this
		SPIError(err, str);	// limitation, who's to say the value being passed is not -1?

	return 0;
}

uint8 SPIShiftIn (uint8 bits, const uint8 mode, void *data, const uint8 bytes) {
	uint8 err;
	uint32 retVal;
	const char str[] = "SPIShiftIn()";

	// Check for errors
	if (8 <= g_spiCog)
		SPIError(SPI_MODULE_NOT_RUNNING);
	if (SPI_MAX_PAR_BITS < bits)
		SPIError(SPI_TOO_MANY_BITS);
	if ((SPI_MSB_PRE != mode) && (SPI_LSB_PRE != mode) && (SPI_MSB_POST != mode)
			&& (SPI_LSB_POST != mode))
		SPIError(SPI_INVALID_MODE);
	if ((4 == bytes && ((uint32) data) % 4) || (2 == bytes && ((uint32) data) % 2))
		SPIError(SPI_ADDR_MISALIGN);

	// Call GAS function
	if (err = SPIPassPar(
			SPI_FUNC_READ | (bits << SPI_BITS_OFFSET) | (mode << SPI_MODE_OFFSET)))
		SPIError(err, str);

	// Read in parameter
	if (err = SPIReadPar(data, bytes))
		SPIError(err, str);

	return 0;
}

uint8 SPISetClock (const uint32 frequency) {
	uint8 err;

	if (8 <= g_spiCog)
		SPIError(SPI_MODULE_NOT_RUNNING);
	if (CLKFREQ / 4 <= frequency)
		SPIError(SPI_INVALID_FREQ);

	if (err = SPIPassPar(SPI_FUNC_CLK))
		SPIError(err);
	if (err = SPIPassPar(CLKFREQ / frequency))
		SPIError(err);

	return 0;
}

#ifdef SPI_DEBUG
void SPIError (const uint8 err, ...) {
	va_list list;
	char str[] = "SPI Error %u: %s\n";

	switch (err) {
		case SPI_INVALID_PIN:
			__simple_printf(str, (err - SPI_ERRORS_BASE), "Invalid pin");
			break;
		case SPI_INVALID_CLOCK_INIT:
			__simple_printf(str, (err - SPI_ERRORS_BASE),
					"Invalid clock polarity. Choose from SPI_POLARITY_HIGH or SPI_POLARITY_LOW");
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
		case SPI_TIMEOUT_WR:
			va_start(list, 1);
			__simple_printf("SPI Error %u: %s\n\tCalling function was %s\n",
					(err - SPI_ERRORS_BASE), "Timed out during parameter passing",
					va_arg(list, char*));
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
			__simple_printf(str, (err - SPI_ERRORS_BASE), "SPI GAS cog not running");
			break;
		case SPI_INVALID_FREQ:
			__simple_printf(str, (err - SPI_ERRORS_BASE), "Frequency set too high");
			break;
		case SPI_ADDR_MISALIGN:
			__simple_printf(str, (err - SPI_ERRORS_BASE),
					"Passed in address is miss aligned");
			break;
		default:
			// Is the error an SPI error?
			if (err > SPI_ERRORS_BASE && err < (SPI_ERRORS_BASE + SPI_ERRORS_LIMIT))
				__simple_printf("Unknown SPI error %u\n", (err - SPI_ERRORS_BASE));
			else
				__simple_printf("Unknown error %u\n", (err));
			break;
	}
	while (1)
		;
}
#endif
