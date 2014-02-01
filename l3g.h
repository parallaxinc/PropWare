/**
 * @file    l3g.h
 */
/**
 * @brief   L3G gyroscope driver using SPI communication for the Parallax
 *          Propeller
 *
 * @project PropWare
 *
 * @author  David Zemon
 * @author  Collin Winans
 *
 * @copyright
 * The MIT License (MIT)<br>
 * <br>Copyright (c) 2013 David Zemon<br>
 * <br>Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:<br>
 * <br>The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.<br>
 * <br>THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef L3G_H_
#define L3G_H_

/**
 * @defgroup _propware_l3g  L3G 3-axis gyroscope
 * @{
 */

/**
 * @publicsection @{
 */

#include <propeller.h>
#include <PropWare.h>
#include <spi.h>

/**
 * Axes of the L3G device
 */
typedef enum {
    /** X axis */L3G_X,
    /** Y axis */L3G_Y,
    /** Z axis */L3G_Z
} L3G_Axis;

/**
 * Extra functions available on the L3G device; Callable by passing one as the
 * first parameter to @ref l3g_ioctl
 */
typedef enum {
    /** Set the sensitivity of input values; must be one of L3G_DPSMode */
    L3G_FUNC_MOD_DPS,
    /** Read the value on any internal register */
    L3G_FUNC_RD_REG,
    /** Total number of advanced functions */
    L3G_FUNCS
} L3G_IoctlFunction;

/**
 * Sensitivity measured in degrees per second
 */
typedef enum {
    /** 250 degrees per second */
    L3G_250_DPS = 0x00,
    /** 500 degrees per second */
    L3G_500_DPS = 0x10,
    /** 2000 degrees per second */
    L3G_2000_DPS = 0x20
} L3G_DPSMode;

/**
 * @name    Register address
 * @{
 */
#define L3G_WHO_AM_I        0x0F

#define L3G_CTRL_REG1       0x20
#define L3G_CTRL_REG2       0x21
#define L3G_CTRL_REG3       0x22
#define L3G_CTRL_REG4       0x23
#define L3G_CTRL_REG5       0x24
#define L3G_REFERENCE       0x25
#define L3G_OUT_TEMP        0x26
#define L3G_STATUS_REG      0x27

#define L3G_OUT_X_L         0x28
#define L3G_OUT_X_H         0x29
#define L3G_OUT_Y_L         0x2A
#define L3G_OUT_Y_H         0x2B
#define L3G_OUT_Z_L         0x2C
#define L3G_OUT_Z_H         0x2D

#define L3G_FIFO_CTRL_REG   0x2E
#define L3G_FIFO_SRC_REG    0x2F

#define L3G_INT1_CFG        0x30
#define L3G_INT1_SRC        0x31
#define L3G_INT1_THS_XH     0x32
#define L3G_INT1_THS_XL     0x33
#define L3G_INT1_THS_YH     0x34
#define L3G_INT1_THS_YL     0x35
#define L3G_INT1_THS_ZH     0x36
#define L3G_INT1_THS_ZL     0x37
#define L3G_INT1_DURATION   0x38
/**@}*/

/**
 * @brief       Initialize an L3G module
 *
 * @param[in]   mosi        Pin mask for MOSI
 * @param[in]   miso        Pin mask for MISO
 * @param[in]   sclk        Pin mask for SCLK
 * @param[in]   cs          Pin mask for CS
 * @param[in]   dpsMode     One of L3G_250_DPS, L3G_500_DPS, L3G_2000_DPS;
 *                          Determines the resolution of the L3G device in terms
 *                          of degrees per second
 *
 * @return       Returns 0 upon success, error code otherwise
 */
uint8_t l3g_start (const uint32_t mosi, const uint32_t miso, const uint32_t sclk,
        const uint32_t cs, const L3G_DPSMode dpsMode);

/**
 * @brief       Choose whether to always set the SPI mode and bitmode before
 *              reading or writing to the L3G module; Useful when multiple
 *              devices are connected to the SPI bus
 *
 * @param[in]   alwaysSetMode   For any non-zero value, the SPI modes will
 *                              always be set before a read or write routine
 */
void l3g_always_set_spi_mode (const uint8_t alwaysSetMode);

/**
 * @brief       Read a specific axis's data
 *
 * @param[in]   axis    One of L3G_X, L3G_Y, L3G_Z; Selects the axis to be read
 * @param[out]  *val    Address that data should be placed into
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t l3g_read (const L3G_Axis axis, int16_t *val);

/**
 * @brief       Read data from the X axis
 *
 * @param[out]  *val    Address that data should be placed into
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t l3g_read_x (int16_t *val);

/**
 * @brief       Read data from the Y axis
 *
 * @param[out]  *val    Address that data should be placed into
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t l3g_read_y (int16_t *val);

/**
 * @brief       Read data from the Z axis
 *
 * @param[out]  *val    Address that data should be placed into
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t l3g_read_z (int16_t *val);

/**
 * @brief       Read data from all three axes
 *
 * @param[out]  *val    Starting address for data to be placed; 6 contiguous
 *                      bytes of space are required for the read routine
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t l3g_read_all (int16_t *val);

/**
 * @brief       Allow numerous advanced functions to be performed on the L3G,
 *              depending on the value of the first parameter
 *
 *
 * @detailed    <strong>L3G_FUNC_MOD_DPS:</strong> Modify the precision of L3G
 *              in terms of degrees per second
 * @param[in]   func    Descriptor for which function should be performed
 * @param[in]   wrVal   One of L3G_250_DPS, L3G_500_DPS, L3G_2000_DPS
 * @param[out]  *rdVal  Unused
 *
 * @detailed    <strong>L3G_FUNC_RD_REG:</strong> Read any register from the L3G
 * @param[in]   func    Descriptor for which function should be performed
 * @param[in]   wrVal   Address of the desired register
 * @param[out]  *rdVal  Resulting value will be stored in rdVal
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t l3g_ioctl (const L3G_IoctlFunction func, const uint8_t wrVal,
        uint8_t *rdVal);
/**@}*/

/*************************************
 *** Private Function Declarations ***
 *************************************/
/**
 * @privatesection @{
 */

/**
 * @brief       Write one byte to the L3G module
 *
 * @param[in]   address     Destination register address
 * @param[in]   dat         Data to be written to the destination register
 *
 * @return      Returns 0 upon success, error code otherwise
 */
static uint8_t l3g_write8 (uint8_t addr, const uint8_t dat);

/**
 * @brief       Write one byte to the L3G module
 *
 * @param[in]   address     Destination register address
 * @param[in]   dat         Data to be written to the destination register
 *
 * @return      Returns 0 upon success, error code otherwise
 */
static uint8_t l3g_write16 (uint8_t addr, const uint16_t dat);

/**
 * @brief       Read one byte from the L3G module
 *
 * @param[in]   address     Origin register address
 * @param[out]  *dat        Address where incoming data should be stored
 *
 * @return      Returns 0 upon success, error code otherwise
 */
static uint8_t l3g_read8 (uint8_t addr, int8_t *dat);

/**
 * @brief       Read two bytes from the L3G module
 *
 * @param[in]   address     Origin register address
 * @param[out]  *dat        Address where incoming data should be stored
 *
 * @return      Returns 0 upon success, error code otherwise
 */
static uint8_t l3g_read16 (uint8_t addr, int16_t *dat);

/**@}*/

/**@}*/

#endif /* L3G_H_ */
