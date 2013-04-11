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

// Default EOF character is -1. If this is incompatible with your system, please
// un-comment the following line and set accordingly
//#define SD_EOF

// Extra code options: un-comment as desired to enable extra features
#define SD_DEBUG
#define SD_VERBOSE
//#define SD_VERBOSE_BLOCKS
#define SD_SHELL
//#define SD_SPEED_OVER_SPACE

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
#define SD_FILENAME_NOT_FOUND	SD_ERRORS_BASE + 7
#define SD_EMPTY_FAT_ENTRY		SD_ERRORS_BASE + 8
#define SD_CORRUPT_CLUSTER		SD_ERRORS_BASE + 9

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

/* @Brief: Load the first sector of a file into the file buffer; Initialize global
 * 		   character pointers (seek and tell)
 *
 * @param *filename		C-string containing the filename to open
 *
 * @return 		Returns 0 upon success, error code otherwise
 */
// TODO: Implement this
uint8 SDOpen (const char *filename);

#ifdef SD_SHELL
#include <stdio.h>
/* @Brief: List the contents of a directory on the screen (similar to 'ls .')
 *
 * @param	*absPath	Absolute path of the directory to be printed
 *
 * @return		Returns 0 upon success, error code otherwise
 */
// NOTE: Beginning work by reading only root directory
uint8 SD_Shell_ls (void);

/* @Brief: Dump the contents of a file to the screen (similar to 'cat f');
 *
 * @Note: Does not currently follow paths
 *
 * @param	*f			Short filename of file to print
 *
 * @return		Returns 0 upon success, error code otherwise
 */
uint8 SD_Shell_cat (const char *f);
#endif

#ifdef SD_VERBOSE
/* @Brief: Print a block of data in hex format to the screen in SD_LINE_SIZE-byte lines
 *
 * @param	*dat		Pointer to the beginning of the data
 * @param	bytes		Number of bytes to print
 *
 * @return		Returns 0 upon success, error code otherwise
 */
uint8 SDPrintHexBlock (uint8 *dat, uint16 bytes);
#endif

#endif /* SD_H_ */
