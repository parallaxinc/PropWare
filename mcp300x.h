/* File:    mcp300x.h
 *
 * Author:  David Zemon
 *          Collin Winans
 *
 * Description: MCP300x analog-to-digital driver for Parallax Propeller using SPI
 * 				communication
 *
 * 				NOTE: MCP300x chips use MSB first, high clock polarity, and MSB_PRE
 * 					  SPI modes
 */

#ifndef MCP300X_H_
#define MCP300X_H_

#include <propeller.h>
#include <PropWare.h>
#include <spi.h>
//#include "SPI_c.h"
#include <gpio.h>

#define MCP300X_SPI_DEFAULT_FREQ	1000000
#define MCP300X_SPI_MODE			SPI_MODE_0
#define MCP300X_SPI_BITMODE			SPI_MSB_FIRST

typedef enum {
	CHANNEL_0,
	CHANNEL_1,
	CHANNEL_2,
	CHANNEL_3,
	CHANNEL_4,  // Only accessible on MCP3008
	CHANNEL_5,  // Only accessible on MCP3008
	CHANNEL_6,  // Only accessible on MCP3008
	CHANNEL_7,  // Only accessible on MCP3008
} mcp_channel_t;

// Channel numbers listed as DIFF_<positive>_<negative>
typedef enum {
	DIFF_0_1,
	DIFF_1_0,
	DIFF_2_3,
	DIFF_3_2,
	DIFF_4_5,  // Only accessible on MCP3008
	DIFF_5_4,  // Only accessible on MCP3008
	DIFF_6_7,  // Only accessible on MCP3008
	DIFF_7_6  // Only accessible on MCP3008
} mcp_channel_diff_t;

/**
 *
 */
int8_t MCP300xStart (const uint32_t mosi, const uint32_t miso, const uint32_t sclk,
		const uint32_t cs);

void MCP300xAlwaysSetMode (const uint8_t alwaysSetMode);

int8_t MCP300xRead (const mcp_channel_t channel, uint16_t *dat);

int8_t MCP300xReadDif (const mcp_channel_diff_t channels, uint16_t *dat);

#endif /* MCP300X_H_ */
