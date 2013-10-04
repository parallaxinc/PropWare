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

#define MCP300X_SPI_MODE		SPI_MODE_0
#define MCP300X_SPI_BITMODE		SPI_MSB_FIRST

#define CHANNEL_0				0
#define CHANNEL_1				1
#define CHANNEL_2				2
#define CHANNEL_3				3
#define CHANNEL_4				4	// Only accessible on MCP3008
#define CHANNEL_5				5	// Only accessible on MCP3008
#define CHANNEL_6				6	// Only accessible on MCP3008
#define CHANNEL_7				7	// Only accessible on MCP3008

// Channel numbers listed as DIFF_<positive>_<negative>
#define DIFF_0_1				0
#define DIFF_1_0				1
#define DIFF_2_3				2
#define DIFF_3_2				3
#define DIFF_4_5				4	// Only accessible on MCP3008
#define DIFF_5_4				5	// Only accessible on MCP3008
#define DIFF_6_7				6	// Only accessible on MCP3008
#define DIFF_7_6				7	// Only accessible on MCP3008

uint8_t MCP300xStart (const uint8_t cs);

uint8_t MCP300xRead (const uint8_t channel, uint16_t *dat);

uint8_t MCP300xReadDif (const uint8_t channels, uint16_t *dat);

#endif /* MCP300X_H_ */
