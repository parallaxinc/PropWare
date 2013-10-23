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
#include <spi.h>
//#include <SPI_c.h>

#define L3G_SPI_MODE			SPI_MODE_3
#define L3G_SPI_BITMODE			SPI_MSB_FIRST
#define L3G_SPI_DEFAULT_FREQ	100000

typedef enum {
	L3G_X,
	L3G_Y,
	L3G_Z
} l3g_axis;

typedef enum {
	L3G_FUNC_MOD_DPS,
	L3G_FUNC_RD_REG,
	L3G_FUNCS
} l3g_ioctl_function_t;

typedef enum {
	L3G_250_DPS = 0x00,
	L3G_500_DPS = 0x10,
	L3G_2000_DPS = 0x20
} l3g_dps_mode_t;

/**
 * \brief   Initialize an L3G module
 *
 * \param   mosi        Pin mask for MOSI
 * \param   miso        Pin mask for MISO
 * \param   sclk        Pin mask for SCLK
 * \param   cs          Pin mask for CS
 * \param   dpsMode     One of L3G_250_DPS, L3G_500_DPS, L3G_2000_DPS;
 *                      Determines the resolution of the L3G device in terms of
 *                      degrees per second
 *
 * \return       Returns 0 upon success, error code otherwise
 */
uint8_t L3GStart (const uint32_t mosi, const uint32_t miso, const uint32_t sclk,
		const uint32_t cs, const l3g_dps_mode_t dpsMode);

/**
 * \brief   Choose whether to always set the SPI mode and bitmode before reading
 *          or writing to the L3G module; Useful when multiple devices are
 *          connected to the SPI bus
 *
 * \param   alwaysSetMode   For any non-zero value, the SPI modes will always be
 *                          set before a read or write routine
 */
void L3GAlwaysSetMode (const uint8_t alwaysSetMode);

/**
 * \brief   Read a specific axis's data
 *
 * \param   axis    One of L3G_X, L3G_Y, L3G_Z; Selects the axis to be read
 * \param   *val    Address that data should be placed into
 *
 * \return      Returns 0 upon success, error code otherwise
 */
uint8_t L3GRead (const l3g_axis axis, int16_t *val);

/**
 * \brief   Read data from the X axis
 *
 * \param   *val    Address that data should be placed into
 *
 * \return      Returns 0 upon success, error code otherwise
 */
uint8_t L3GReadX (int16_t *val);

/**
 * \brief   Read data from the Y axis
 *
 * \param   *val    Address that data should be placed into
 *
 * \return      Returns 0 upon success, error code otherwise
 */
uint8_t L3GReadY (int16_t *val);

/**
 * \brief   Read data from the Z axis
 *
 * \param   *val    Address that data should be placed into
 *
 * \return      Returns 0 upon success, error code otherwise
 */
uint8_t L3GReadZ (int16_t *val);

/**
 * \brief   Read data from all three axes
 *
 * \param   *val    Starting address for data to be placed; 6 contiguous bytes
 *                  of space are required for the read routine
 *
 * \return      Returns 0 upon success, error code otherwise
 */
uint8_t L3GReadAll (int16_t *val);

/**
 * \brief   Allow numerous advanced functions to be performed on the L3G
 *
 * ALL Functions
 * \param   func    Descriptor for which function should be performed
 *
 * \return     Returns 0 upon success, error code otherwise
 *
 * L3G_FUNC_MOD_DPS
 * \detailed    Modify the precision of L3G in terms of degrees per second
 * \param   wrVal   One of L3G_250_DPS, L3G_500_DPS, L3G_2000_DPS
 * \param   *rdVal  Unused
 *
 * L3G_FUNC_RD_REG
 * \detailed	Read any register from the L3G
 * \param	wrVal	Address of the desired register
 * \param	*rdVal	Resulting value will be stored in rdVal
 */
uint8_t L3G_ioctl (const l3g_ioctl_function_t func, const uint8_t wrVal, uint8_t *rdVal);

/*************************************
 *** Private Function Declarations ***
 *************************************/
static uint8_t L3GWrite8 (uint8_t addr, const uint8_t dat);

static uint8_t L3GWrite16 (uint8_t addr, const uint16_t dat);

static uint8_t L3GRead8 (uint8_t addr, int8_t *dat);

static uint8_t L3GRead16 (uint8_t addr, int16_t *dat);

#endif /* L3G_H_ */
