/**
 * @file            max6675.h
 *
 * @author          David Zemon
 *
 * @description     TODO: Do me
 */

#ifndef MAX6675_H_
#define MAX6675_H_

#include <PropWare.h>
#include <spi.h>

#define MAX6675_SPI_DEFAULT_FREQ		1000000
#define MAX6675_SPI_MODE				SPI_MODE_1
#define MAX6675_SPI_BITMODE				SPI_MSB_FIRST
#define MAX6675_BIT_WIDTH				12

/**
 * @brief   Initialize communication with an MAX6675 device
 *
 * @param   mosi        Pin mask for MOSI
 * @param   miso        Pin mask for MISO
 * @param   sclk        Pin mask for SCLK
 * @param   cs          Pin mask for CS
 *
 * @return      Returns 0 upon success, error code otherwise
 */
int8_t MAX6675Start (const uint32_t mosi, const uint32_t miso,
		const uint32_t sclk, const uint32_t cs);

/**
 * @brief   Choose whether to always set the SPI mode and bitmode before reading
 *          or writing to the chip; Useful when multiple devices are
 *          connected to the SPI bus
 *
 * @param   alwaysSetMode   For any non-zero value, the SPI modes will always be
 *                          set before a read or write routine
 */
void MAX6675AlwaysSetMode (const uint8_t alwaysSetMode);

/**
 * @brief   Read data in fixed-point form
 *
 * @detailed    12-bit data is stored where lower 2 bits are fractional and
 *              upper 10 bits are the whole number. Value presented in degrees
 *              Celsius
 *
 * @param   *dat    Address where data should be stored
 *
 * @return      Returns 0 upon success, error code otherwise
 */
int8_t MAX6675Read (uint16_t *dat);

/**
 * @brief   Read data and return integer value
 *
 * @param   *dat    Address where data should be stored
 *
 * @return      Returns 0 upon success, error code otherwise
 */
int8_t MAX6675ReadWhole (uint16_t *dat);

/**
 * @brief   Read data in floating point form
 *
 * @param   *dat    Address where data should be stored
 *
 * @return      Returns 0 upon success, error code otherwise
 */
int8_t MAX6675ReadFloat (float *dat);

#endif /* MAX6675_H_ */
