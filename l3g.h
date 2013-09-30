/* File:    l3g.h
 *
 * Author:  David Zemon
 *          Collin Winans
 *
 * Description: L3G gyrometer driver for Parallax Propeller using SPI
 * 				communication
 */

#ifndef L3G_H_
#define L3G_H_

#include <propeller.h>
#include <PropWare.h>
//#include <spi.h>
#include "SPI_c.h"

#define L3G_SPI_MODE		SPI_MODE_3
#define L3G_SPI_BITMODE		SPI_MSB_FIRST

#define L3G_X				0
#define L3G_Y				1
#define L3G_Z				2

#define L3G_FUNC_MOD_DPS	0
#define L3G_FUNC_RD_REG		1

#define L3G_250_DPS			0x0
#define L3G_500_DPS			0x01
#define L3G_2000_DPS		0x02

/**
 * \brief	Start
 */
uint8_t L3GStart (const uint32_t cs, const uint8_t dpsMode);

uint8_t L3GReadX (int16_t *val);

uint8_t L3GReadY (int16_t *val);

uint8_t L3GReadZ (int16_t *val);

uint8_t L3GRead (int16_t *val);

/**
 * \brief	Allow numerous advanced functions to be performed on the L3G
 *
 * ALL Functions
 * @param	func	Descriptor for which function should be performed
 *
 * @return		Returns 0 upon success, error code otherwise
 *
 * L3G_FUNC_MOD_DPS
 * @detailed	Modify the precision of L3G in terms of degrees per second
 * @param	wrVal	One of L3G_250_DPS, L3G_500_DPS, L3G_2000_DPS
 * @param	*rdVal	Unused
 *
 * L3G_FUNC_RD_REG
 * @detailed	Read any register from the L3G
 * @param	wrVal	Address of the desired register
 * @param	*rdVal	Resulting value will be stored in rdVal
 */
uint8_t L3G_ioctl (const uint8_t func, const uint8_t wrVal, uint8_t *rdVal);

/*************************************
 *** Private Function Declarations ***
 *************************************/
static uint8_t L3GWrite8 (uint8_t addr, const uint8_t dat);

static uint8_t L3GWrite16 (uint8_t addr, const uint16_t dat);

static uint8_t L3GRead8 (uint8_t addr, uint8_t *dat);

static uint8_t L3GRead16 (uint8_t addr, uint16_t *dat);

#endif /* L3G_H_ */
