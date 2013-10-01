/* File:    mcp300x.c
 *
 * Author:  David Zemon
 *          Collin Winans
 */

// Includes
#include "mcp300x.h"

#define MCP300X_START			BIT_4
#define MCP300X_SINGLE_ENDED	BIT_3
#define MCP300X_DIFFERENTIAL	0

#define MCP300X_OPTN_WIDTH		7
#define MCP300X_DATA_WIDTH		10

uint8_t g_mcp300x_cs;

uint8_t MCP300xStart (const uint8_t cs) {
	g_mcp300x_cs = cs;
	GPIODirModeSet(cs, GPIO_DIR_OUT);
	GPIOPinSet(cs);

	if (!SPIIsRunning())
		return -1; // TODO: Create real error codes

	return 0;
}

uint8_t MCP300xRead (const uint8_t channel, uint16_t *dat) {
	uint8_t err, options;
	uint32_t led;

	options = MCP300X_START | MCP300X_SINGLE_ENDED | channel;
	options <<= 2; // One dead bit between output and input - see page 19 of datasheet

	checkErrors(SPISetMode(MCP300X_SPI_MODE));
	checkErrors(SPISetBitMode(MCP300X_SPI_BITMODE));

	GPIOPinClear(g_mcp300x_cs);
	checkErrors(SPIShiftOut(MCP300X_OPTN_WIDTH, options));
	checkErrors(SPIShiftIn(MCP300X_DATA_WIDTH, dat, sizeof(*dat)));
	GPIOPinSet(g_mcp300x_cs);

	return 0;
}

uint8_t MCP300xReadDif (const uint8_t channels, uint16_t *dat) {
	uint8_t err, options;

	options = MCP300X_START | MCP300X_DIFFERENTIAL | channels;
	options <<= 2; // One dead bit between output and input - see page 19 of datasheet

	checkErrors(SPISetMode(MCP300X_SPI_MODE));
	checkErrors(SPISetBitMode(MCP300X_SPI_BITMODE));

	GPIOPinClear(g_mcp300x_cs);
	checkErrors(SPIShiftOut(MCP300X_OPTN_WIDTH, options));
	checkErrors(SPIShiftIn(MCP300X_DATA_WIDTH, dat, sizeof(*dat)));
	GPIOPinSet(g_mcp300x_cs);

	return 0;
}
