/* File:    mcp300x.h
 *
 * Author:  David Zemon
 *          Collin Winans
 *
 * Description: MCP300x analog-to-digital driver for Parallax Propeller using
 *              SPI communication. Supports both MCP3004 and MCP3008 devices
 *              along with both single-ended and differential readings.
 *
 *              NOTE: MCP300x chips uses SPI mode 2 and shifts data MSB first
 */

#ifndef MCP300X_H_
#define MCP300X_H_

#include <propeller.h>
#include <PropWare.h>
#include <spi.h>
//#include <SPI_c.h>

#define MCP300X_SPI_DEFAULT_FREQ	100000
#define MCP300X_SPI_MODE			SPI_MODE_2
#define MCP300X_SPI_BITMODE			SPI_MSB_FIRST

typedef enum {
	MCP_CHANNEL_0,
	MCP_CHANNEL_1,
	MCP_CHANNEL_2,
	MCP_CHANNEL_3,
	MCP_CHANNEL_4,  // Only accessible on MCP3008
	MCP_CHANNEL_5,  // Only accessible on MCP3008
	MCP_CHANNEL_6,  // Only accessible on MCP3008
	MCP_CHANNEL_7,  // Only accessible on MCP3008
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
 * \brief   Initialize communication with an MCP300x device
 *
 * \param   mosi        Pin mask for MOSI
 * \param   miso        Pin mask for MISO
 * \param   sclk        Pin mask for SCLK
 * \param   cs          Pin mask for CS
 *
 * \return      Returns 0 upon success, error code otherwise
 */
int8_t MCP300xStart (const uint32_t mosi, const uint32_t miso,
		const uint32_t sclk, const uint32_t cs);

/**
 * \brief   Choose whether to always set the SPI mode and bitmode before reading
 *          or writing to the ADC; Useful when multiple devices are
 *          connected to the SPI bus
 *
 * \param   alwaysSetMode   For any non-zero value, the SPI modes will always be
 *                          set before a read or write routine
 */
void MCP300xAlwaysSetMode (const uint8_t alwaysSetMode);

/**
 * \brief   Read a specific channel's data in single-ended mode
 *
 * \param   axis    One of MCP_CHANNEL_<x>, where <x> is a number 0 through 3
 *                  (or 0 through 7 for the MCP3008); Selects the channel to be
 *                  read
 * \param   *val    Address that data should be placed into
 *
 * \return      Returns 0 upon success, error code otherwise
 */
int8_t MCP300xRead (const mcp_channel_t channel, uint16_t *dat);

/**
 * \brief   Read a specific axis's data in differential mode
 *
 * \param   axis    One of DIFF_<x>_<y>, where <x> is a number 0 through 3
 *                  (or 0 through 7 for the MCP3008) and <y> is
 *                  <x> + (<x> + 1)%2 (See above defined enum or datasheet for
 *                  details)
 * \param   *val    Address that data should be placed into
 *
 * \return      Returns 0 upon success, error code otherwise
 */
int8_t MCP300xReadDif (const mcp_channel_diff_t channels, uint16_t *dat);

#endif /* MCP300X_H_ */
