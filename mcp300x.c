/* File:    mcp300x.c
 *
 * Author:  David Zemon
 *          Collin Winans
 */

// Includes
#include <mcp300x.h>

#define MCP300X_START			BIT_4
#define MCP300X_SINGLE_ENDED	BIT_3
#define MCP300X_DIFFERENTIAL	0

#define MCP300X_OPTN_WIDTH		7
#define MCP300X_DATA_WIDTH		10

uint8_t g_mcp300x_cs;
uint8_t g_mcp300x_alwaysSetMode = 0;

int8_t MCP300xStart (const uint32_t mosi, const uint32_t miso, const uint32_t sclk,
		const uint32_t cs) {
	int8_t err;

	g_mcp300x_cs = cs;
	GPIODirModeSet(cs, GPIO_DIR_OUT);
	GPIOPinSet(cs);

	if (!SPIIsRunning()) {
		checkErrors(
				SPIStart(mosi, miso, sclk, MCP300X_SPI_DEFAULT_FREQ, MCP300X_SPI_MODE, MCP300X_SPI_BITMODE));
	} else {
		checkErrors(SPISetMode(MCP300X_SPI_MODE));
		checkErrors(SPISetBitMode(MCP300X_SPI_BITMODE));
	}

	return 0;
}

void MCP300xAlwaysSetMode (const uint8_t alwaysSetMode) {
	g_mcp300x_alwaysSetMode = alwaysSetMode;
}

int8_t MCP300xRead (const mcp_channel_t channel, uint16_t *dat) {
	int8_t err, options;

	options = MCP300X_START | MCP300X_SINGLE_ENDED | channel;
	options <<= 2;  // One dead bit between output and input - see page 19 of datasheet

	if (g_mcp300x_alwaysSetMode) {
		checkErrors(SPISetMode(MCP300X_SPI_MODE));
		checkErrors(SPISetBitMode(MCP300X_SPI_BITMODE));
	}

	GPIOPinClear(g_mcp300x_cs);
	checkErrors(SPIShiftOut(MCP300X_OPTN_WIDTH, options));
	checkErrors(SPIShiftIn(MCP300X_DATA_WIDTH, dat, sizeof(*dat)));
	GPIOPinSet(g_mcp300x_cs);

	return 0;
}

int8_t MCP300xReadDif (const mcp_channel_diff_t channels, uint16_t *dat) {
	int8_t err, options;

	options = MCP300X_START | MCP300X_DIFFERENTIAL | channels;
	options <<= 2;  // One dead bit between output and input - see page 19 of datasheet

	if (g_mcp300x_alwaysSetMode) {
		checkErrors(SPISetMode(MCP300X_SPI_MODE));
		checkErrors(SPISetBitMode(MCP300X_SPI_BITMODE));
	}

	GPIOPinClear(g_mcp300x_cs);
	checkErrors(SPIShiftOut(MCP300X_OPTN_WIDTH, options));
	checkErrors(SPIShiftIn(MCP300X_DATA_WIDTH, dat, sizeof(*dat)));
	GPIOPinSet(g_mcp300x_cs);

	return 0;
}
