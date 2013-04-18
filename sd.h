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

/* @Brief: Extra code options - Uncomment to enable features
 *
 * @option	SD_DEBUG			Enables thorough debugging features similar to exceptions; Errors
 * 								will be caught the program will enter an infinite loop
 * 								DEFAULT: OFF
 * @option	SD_VERBOSE			Verbose functions will be enabled (such as SDPrintHexBlock) and
 * 								error checking will display pertinent information through UART
 * 								DEFAULT: ON
 * @option	SD_VERBOSE_BLOCKS	Select data blocks/sectors will be display via UART for
 * 								debugging purposes
 * 								DEFAULT: OFF
 * @option	SD_SHELL			Unix-like command-line arguments will be defined and available
 * 								DEFAULT: ON
 * @option	SD_SPEED_OVER_SPACE	Extra RAM will be used to keep file and directory sectors separate;
 * 								This saves time and does not force the chip to reload directory
 * 								contents with each file-switch
 * 								DEFAULT: ON
 */
//#define SD_DEBUG
//#define SD_VERBOSE
//#define SD_VERBOSE_BLOCKS
#define SD_SHELL
#define SD_SPEED_OVER_SPACE

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
 * @param	mosi		Pin mask for MOSI pin
 * @param	miso		Pin mask for MISO pin
 * @param	sclk		Pin mask for SCLK pin
 * @param	cs			Pin mask for CS pin
 *
 * @return		Returns 0 upon success, otherwise error code
 */
uint8 SDStart (const uint32 mosi, const uint32 miso, const uint32 sclk, const uint32 cs);

/* @Brief: Mount either FAT16 or FAT32 filesystem
 *
 * @return		Returns 0 upon success, error code otherwise
 */
uint8 SDMount (void);

/* @Brief: Load the first sector of a file into the file buffer; Initialize global
 * 		   character pointers (seek and tell)
 *
 * @param	*filename		C-string containing the filename to open
 *
 * @return 		Returns 0 upon success, error code otherwise
 */
// TODO: Implement this
uint8 SDOpen (const char *filename);

/* @Brief: In terms of buffer offset, find a file that matches the name in *filename in the current
 *         directory.
 *
 * @param	*filename			C-string representing the short (standard) filename
 * @param	*fileEntryOffset	The buffer offset will be returned via this address if the file is found
 *
 * @return		Returns 0 upon success, error code otherwise (common error code is SD_EOC_END for
 *              end-of-chain marker)
 */
uint8 SDFind (const char *filename, uint16 *fileEntryOffset);

#ifdef SD_SHELL
#include <stdio.h>
/* @Brief: Provide the user with a very basic unix-like shell. The following commands
 *         are available to the user: ls, cat, cd.
 *
 * @return		Returns 0 upon success, error code otherwise
 */
uint8 SD_Shell (void);

/* @Brief: List the contents of a directory on the screen (similar to 'ls .')
 *
 * @param	*absPath	Absolute path of the directory to be printed
 *
 * @return		Returns 0 upon success, error code otherwise
 */
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

/* @Brief: Change the current working directory to *f (similar to 'cd f');
 *
 * @param	*d			Short filename of directory to change to
 *
 * @return		Returns 0 upon success, error code otherwise
 */
uint8 SD_Shell_cd (const char *d);
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

/*******************************************
 *** Private SD Definitions & Prototypes ***
 *******************************************/
// SPI config
#define SD_SPI_INIT_FREQ			400000					// Run SD initialization at 200 kHz
#define SD_SPI_FINAL_FREQ			1900000					// Speed clock to 1.8 MHz after initialization; TODO: implement a custom send/receive command that does not use waitcnt instructions
#define SD_SPI_POLARITY				SPI_POLARITY_LOW		// SD cards like low polarity
#define SD_SPI_MODE_OUT				SPI_MSB_FIRST
#define SD_SPI_MODE_IN				SPI_MSB_PRE
#define SD_SPI_BYTE_IN_SZ			1

// Misc. SD Definitions
#define SD_WIGGLE_ROOM				10000
#define SD_RESPONSE_TIMEOUT			CLKFREQ/10			// Wait 0.1 seconds for a response before timing out
#define SD_SECTOR_SIZE				512

// SD Commands
#define SD_CMD_IDLE					0x40 + 0			// Send card into idle state
#define	SD_CMD_SDHC					0x40 + 8			// Set SD card version (1 or 2) and voltage level range
#define SD_CMD_RD_CSD				0x40 + 9			// Request "Card Specific Data" block contents
#define SD_CMD_RD_CID				0x40 + 10			// Request "Card Identification" block contents
#define SD_CMD_RD_BLOCK				0x40 + 17			// Request data block
#define SD_CMD_READ_OCR				0x40 + 58			// Request "Operating Conditions Register" contents
#define SD_CMD_APP					0x40 + 55			// Inform card that following instruction is application specific
#define SD_CMD_WR_OP				0x40 + 41			// Send operating conditions for SDC
// SD Arguments
#define SD_CMD_VOLT_ARG				0x000001AA
#define SD_ARG_LEN					5

// SD CRCs
#define SD_CRC_IDLE					0x95
#define SD_CRC_SDHC					0x87
#define SD_CRC_ACMD					0x77
#define SD_CRC_OTHER				0x01

// SD Responses
#define SD_RESPONSE_IDLE			0x01
#define SD_RESPONSE_ACTIVE			0x00
#define SD_DATA_START_ID			0xFE
#define SD_RESPONSE_LEN_R1			1
#define SD_RESPONSE_LEN_R3			5
#define	SD_RESPONSE_LEN_R7			5

// Boot sector addresses/values
#define SD_FAT_16					2					// A FAT entry in FAT16 is 2-bytes
#define SD_FAT_32					4					// A FAT entry in FAT32 is 4-bytes
#define SD_BOOT_SECTOR_ID			0xeb
#define SD_BOOT_SECTOR_ID_ADDR		0
#define SD_BOOT_SECTOR_BACKUP		0x1c6
#define SD_CLUSTER_SIZE_ADDR		0x0d
#define SD_RSVD_SCTR_CNT_ADDR		0x0e
#define SD_NUM_FATS_ADDR			0x10
#define SD_ROOT_ENTRY_CNT_ADDR		0x11
#define SD_TOT_SCTR_16_ADDR			0x13
#define SD_FAT_SIZE_16_ADDR			0x16
#define SD_TOT_SCTR_32_ADDR			0x20
#define SD_FAT_SIZE_32_ADDR			0x24
#define SD_ROOT_CLUSTER_ADDR		0x2c
#define SD_FAT12_CLSTR_CNT			4085
#define SD_FAT16_CLSTR_CNT			65525

// FAT file/directory values
enum cluster_types {
	SD_ROOT_DIR, SD_SUB_DIR, SD_DATA_FILE
};
#define SD_FILE_ENTRY_LENGTH		32					// An entry in a directory uses 32 bytes
#define SD_DELETED_FILE_MARK		0xe5				// Marks that a file has been deleted here, continue to the next entry
#define SD_FILE_ATTRIBUTE_OFFSET	11					// Byte of a file entry to store attribute flags
#define SD_FILE_START_CLSTR_OFFSET	0x1a				// Starting cluster number
#define SD_FILE_START_CLSTR_HIGH	0x14				// High word (16-bits) of the starting cluster number (FAT32 only)
#define SD_FILE_LEN_OFFSET			0x1c				// Length of a file in bytes
#define SD_FILE_NAME_LEN			8					// 8 characters in the standard file name
#define SD_FILE_EXTENSION_LEN		3					// 3 character file name extension
#define SD_FILENAME_STR_LEN			SD_FILE_NAME_LEN + SD_FILE_EXTENSION_LEN + 2
#define SD_FREE_CLUSTER				0					// Cluster is unused
#define SD_RESERVED_CLUSTER			1
#define SD_RSVD_CLSTR_VAL_BEG		-15					// First reserved cluster value
#define SD_RSVD_CLSTR_VAL_END		-9					// Last reserved cluster value
#define SD_BAD_CLUSTER				-8					// Cluster is corrupt
#define SD_EOC_BEG					-7					// Last marker for end-of-chain (end of file entry within FAT)
#define SD_EOC_END					-1					// First marker for end-of-chain
// FAT file attributes (definitions with trailing underscore represent character for a cleared attribute flag)
#define SD_READ_ONLY				BIT_0
#define SD_READ_ONLY_CHAR			'r'
#define SD_READ_ONLY_CHAR_			'w'
#define SD_HIDDEN_FILE				BIT_1
#define SD_HIDDEN_FILE_CHAR			'h'
#define SD_HIDDEN_FILE_CHAR_		'.'
#define SD_SYSTEM_FILE				BIT_2
#define SD_SYSTEM_FILE_CHAR			's'
#define SD_SYSTEM_FILE_CHAR_		'.'
#define SD_VOLUME_ID				BIT_3
#define SD_VOLUME_ID_CHAR			'v'
#define SD_VOLUME_ID_CHAR_			'.'
#define SD_SUB_DIR					BIT_4
#define SD_SUB_DIR_CHAR				'd'
#define SD_SUB_DIR_CHAR_			'f'
#define SD_ARCHIVE					BIT_5
#define SD_ARCHIVE_CHAR				'a'
#define SD_ARCHIVE_CHAR_			'.'

// Shell definitions
#define SD_SHELL_INPUT_LEN			64
#define SD_SHELL_CMD_LEN			8
#define SD_SHELL_ARG_LEN			32
#define SD_SHELL_EXIT				("exit")
#define SD_SHELL_LS					("ls")
#define SD_SHELL_CAT				("cat")
#define SD_SHELL_CD					("cd")

// File constants
#ifndef SD_EOF
#define SD_EOF						-1					// System dependent - may need to be defined elsewhere
#endif

/***********************************
 *** Private Function Prototypes ***
 ***********************************/
/* @Brief: Send a command and argument over SPI to the SD card
 *
 * @param    command       6-bit value representing the command sent to the SD card
 *
 * @return
 */
static uint8 SDSendCommand (const uint8 cmd, const uint32 arg, const uint8 crc);

/* Brief: Receive response and data from SD card over SPI
 *
 * @param	bytes		Number of bytes to receive
 * @param	*data		Location in memory with enough space to store 'bytes' bytes of data
 *
 * @return		Returns 0 for success, else error code
 */
static uint8 SDGetResponse (const uint8 numBytes, uint8 *dat);

/* @Brief: Receive data from SD card via SPI
 *
 * @param	bytes		Number of bytes to receive
 * @param	*data		Location in memory with enough space to store 'bytes' bytes of data
 * @param	address		Address for the SD card from which data should be read
 *
 * @return		Returns 0 for success, else error code
 */
static uint8 SDReadBlock (uint16 bytes, uint8 *dat);

/* @Brief: Read SD_SECTOR_SIZE-byte data block from SD card
 *
 * @param	address		Block address to read from SD card
 * @param	*dat		Location in chip memory to store data block
 *
 * @return		Returns 0 upon success, error code otherwise
 */
static uint8 SDReadDataBlock (uint32 address, uint8 *dat);

/* @Brief: Read the standard length name of a file entry. If an extension exists, a period will be
 *         inserted before the extension. A null-terminator is always appended to the end
 *
 * @param	*buf		First byte in local memory containing a FAT entry
 * @param	*filename	Address in memory where the filename string will be stored
 *
 * @Pre: *buf must point to the first byte in a FAT entry - no error checking is executed on buf
 * @Pre: Errors may occur if at least 13 (8 + 1 + 3 + 1) bytes of memory are not allocated for filename
 *
 * @return
 */
static void SDGetFilename (const uint8 *buf, char *filename);

/* @Brief: Return byte-reversed 16-bit variable (SD cards store bytes little-endian therefore we must
 * 		   reverse them to use multi-byte variables)
 *
 * @param	dat[]		Address of first byte of data
 *
 * @return		Returns a normal (big-endian) 16-bit word
 */
static uint16 SDConvertDat16 (const uint8 dat[]);

/* @Brief: Return byte-reversed 32-bit variable (SD cards store bytes little-endian therefore we must
 * 		   reverse them to use multi-byte variables)
 *
 * @param	dat[]		Address of first byte of data
 *
 * @return	Returns a normal (big-endian) 32-bit word
 */
static uint32 SDConvertDat32 (const uint8 dat[]);

/* @Brief: Find and return the starting sector's address for a directory path given in a c-string. Use
 *         Unix-style path names (like /foo/bar/)
 *
 * @param	*path		C-string representing Unix-style path
 *
 * @return		Returns sector address of desired path
 */
// TODO: Implement this (more than simply returning root directory)
// TODO: Implement minimalist error checking (-1 or 0 would be valid error codes)
// TODO: Allow for paths outside the current directory
static uint32 SDGetSectorFromPath (const char *path);

/* @Brief: Find and return the starting sector's address for a given allocation unit (note - not cluster)
 *
 * @param	allocUnit	Allocation unit in FAT filesystem
 *
 * @return		Returns sector address of desired allocation unit
 */
static uint32 SDGetSectorFromAlloc (uint32 allocUnit);

/* @Brief: Read an entry from the FAT
 *
 * @param	fatEntry		Entry number (allocation unit) to read in the FAT
 * @param	*value			Address to store the value into (the next allocation unit)
 *
 * @return		Returns 0 upon success, error code otherwise
 */
static uint8 SDGetFATValue (const uint32 fatEntry, uint32 *value);

/* @Brief: Find the next sector in the FAT, directory, or file. When it is found, load it into the
 *         appropriate global buffer
 *
 * @param	*buf		Array of SD_SECTOR_SIZE bytes that can be filled with the requested sector
 *
 * @return		Returns 0 upon success, otherwise error code
 */
static uint8 SDLoadNextSector (uint8 *buf);

/* @Brief: When the final sector of a cluster is finished, SDIncCluster can be called. The appropriate
 *         global variables will be set according (incremented or set by the FAT) and the first sector
 *         of the next cluster will be read into the desired buffer.
 *
 * @param	*curSectorOffset		Address of current sector offset variable
 * @param	*nextAllocUnit			Address of the next allocation unit variable
 * @param	*curAllocUnit			Address of the current allocation unit variable
 * @param	*curClusterStartAddr	Address of the current clutser's starting address variable
 * @param	*buf					Array of SD_SECTOR_SIZE bytes used to hold a sector from the SD card
 *
 * @return		Returns 0 upon success, error code otherwise
 */
static uint8 SDIncCluster (uint8 *curSectorOffset, uint32 *nextAllocUnit, uint32 *curAllocUnit,
		uint32 *curClusterStartAddr, uint8 *buf);

/* @Brief: Load the first sector of a file (note - not directory) and initialize global variables dealing
 *         with files (seek/tell pointers)
 *
 * @param	fileEntryOffset		Offset amount from the beginning of the currently loaded sector; Used to read
 * 								file parameters such as allocation unit and size
 * @param	*fileLen			Length of the file in bytes will be stored into this address
 *
 * @return		Returns 0 upon success, else error code
 */
static uint8 SDOpenFile_ptr (const uint16 fileEntryOffset, uint32 *fileLen);

/* @Brief: Load the first sector of a file (note - not directory) and initialize global variables dealing
 *         with files (seek/tell pointers)
 *
 * @param	fileEntryOffset		Offset amount from the beginning of the currently loaded sector; Used to read
 * 								file parameters such as allocation unit and size
 *
 * @return		Returns 0 upon success, else error code
 */
static uint8 SDOpenDir_ptr (const uint16 fileEntryOffset);

#ifdef SD_SHELL
static inline void SDPrintFileEntry (const uint8 *file, char filename[]);
static void SDPrintFileAttributes (const uint8 flag);
#endif

#ifdef SD_DEBUG
#include <stdio.h>
/* Brief: Print an error through UART string followed by entering an infinite loop
 *
 * @param	err		Error number used to determine error string
 */
static void SDError (const uint8 err);

/* @Brief: Print to screen each status bit individually with human-readable descriptions
 *
 * @param	response		first-byte response from the SD card
 */
static void SDFirstByteExpansion (const uint8 response);
#else
// Exit calling function by returning 'err'
#define SDError(err)				return err
#endif

#endif /* SD_H_ */
