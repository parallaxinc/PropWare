/* File:   sd.h
 * 
 * Author:  David Zemon
 */

#ifndef SD_H_
#define SD_H_

#include <propeller.h>
#include <types.h>
#include <gpio.h>
#include <spi.h>

#define SD_DEBUG
#define SD_VERBOSE

#define SD_LINE_SIZE			16

// Error codes - preceded by SPI
#define SD_ERRORS_BASE			16
#define SD_ERRORS_LIMIT			16
#define SD_INVALID_CMD			SD_ERRORS_BASE + 0
#define SD_READ_TIMEOUT			SD_ERRORS_BASE + 1
#define SD_INVALID_NUM_BYTES	SD_ERRORS_BASE + 2
#define SD_INVALID_RESPONSE		SD_ERRORS_BASE + 3
#define SD_INVALID_INIT			SD_ERRORS_BASE + 4
#define SD_INVALID_FILESYSTEM	SD_ERRORS_BASE + 5
#define SD_INVALID_DAT_STRT_ID	SD_ERRORS_BASE + 6

/* @Brief: Initialize SD card communication over SPI for 3.3V configuration
 *
 * @param	mosi
 */
uint8 SDStart (const uint32 mosi, const uint32 miso, const uint32 sclk, const uint32 cs);

/* @Brief: Mount filesystem
 *
 * @return		Returns 0 upon success, error code otherwise
 */
uint8 SDMount (void);

#ifdef SD_VERBOSE
/* @Brief: Print a block of data in hex format to the screen in 8-byte lines
 *
 * @param	*dat		Pointer to the beginning of the data
 * @param	bytes		Number of bytes to print
 *
 * @return		Returns 0 upon success, error code otherwise
 */
uint8 SDPrintHexBlock (uint8 *dat, uint16 bytes);
#endif

#endif /* SD_H_ */

