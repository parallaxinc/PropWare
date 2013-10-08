/* File:    MCP300x_Demo.c
 *
 * Author:  David Zemon
 * Project: MCP300x_Demo
 */

// Includes
#include "MCP300x_Demo.h"

// Main function
void main (void) {
	int8_t err;
	uint16_t data;
	uint16_t divisor = 1024 / 8;
	uint8_t scaledValue, i;
	uint32_t ledOutput;

	if ((err = MCP300xStart(MOSI, MISO, SCLK, CS)))
		error(err);
	SPISetClock(FREQ);

	// Set the Quickstart LEDs for output (used as a secondary display)
	GPIODirModeSet(BYTE_2, GPIO_DIR_OUT);

	// Though this functional call is not necessary (default value is 0), I
	// want to bring attention to this function. It will determine whether the
	// MCP300xRead* functions will always explicitly set the SPI modes before
	// each call, or assume that the SPI cog is still running in the proper
	// configuration
	MCP300xAlwaysSetMode(0);

	__simple_printf("Welcome to the MCP300x demo!\n");

	while (1) {
		if ((err = MCP300xRead(CHANNEL, &data)))
			error(err);

		// Turn on LEDs proportional to the analog value
		scaledValue = data/divisor;
		ledOutput = 0;
		for (i = 0; i < scaledValue; ++i)
			ledOutput = (ledOutput << 1) | 1;
		ledOutput <<= 16;
		GPIOPinWrite(BYTE_2, ledOutput);

		__simple_printf("Channel %u is reading: %u\n", CHANNEL, data);
		waitcnt(CLKFREQ/2 + CNT);
	}
}

