/**
 *  \brief     SDHC driver for FAT16 and FAT32 for Parallax Propeller
 *  \author    David Zemon
 *  \date      Spring 2013
 *  \pre       The SD card must be SDHC v2 and must be formatted to FAT16 or FAT32
 *  \warning   Unknown result if card is not SDHC v2
 *  \copyright MIT license
 */

/**
 * TODO:	Re-arrange errors in order of impact level; Allows the user to do something like:
 *	  		if ((SD_ERRORS_BASE + 6) < (err = SDFoo()))
 *	 			throw(err);
 * 			which would ignore any error less than 6
 */

#ifndef SD_H_
#define SD_H_

#include <propeller.h>
#include <stdlib.h>
#include <string.h>
#include <PropWare.h>
#include <spi.h>

/**
 * \brief	Extra code options - Uncomment definitions to enable features
 *
 * \param	SD_DEBUG			Enables thorough debugging features similar to exceptions;
 * 								Errors will be caught the program will enter an infinite
 * 								loop
 * 								DEFAULT: OFF
 * \param	SD_VERBOSE			Verbose functions will be enabled (such as SDPrintHexBlock)
 * 								and error checking will display pertinent information
 * 								through UART
 * 								DEFAULT: OFF
 * \param	SD_VERBOSE_BLOCKS	Select data blocks/sectors will be display via UART for
 * 								debugging purposes
 * 								DEFAULT: OFF
 * \param	SD_SHELL			Unix-like command-line arguments will be defined and
 * 								available
 * 								DEFAULT: ON
 * \param	SD_FILE_WRITE		Allows for files to be created and written to - adds
 * 								noticeable code size
 * 								NOTE: Work-in-progress, code size is not necessarily at
 * 								a minimum, nor is RAM usage
 * 								DEFAULT: ON
 */
#define SD_DEBUG
//#define SD_VERBOSE
//#define SD_VERBOSE_BLOCKS
#define SD_SHELL
//#define SD_FILE_WRITE

#define SD_LINE_SIZE			16
#define SD_SECTOR_SIZE			512
#define SD_DEFAULT_SPI_FREQ		1800000

// File modes
typedef enum {
	SD_FILE_MODE_R,
#ifdef SD_FILE_WRITE
	SD_FILE_MODE_R_PLUS, SD_FILE_MODE_A, SD_FILE_MODE_A_PLUS,
#endif
	SD_FILE_MODES
} sd_file_mode;

// File positions
typedef enum {
	SEEK_SET,  // Beginning of the file
	SEEK_CUR,  // Current position in the file
	SEEK_END   // End of the file
} file_pos;

// Error codes - preceded by SPI
#define SD_ERRORS_BASE			16
#define SD_ERRORS_LIMIT			32		// Maximum number of error codes allocated to this module
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
#define SD_INVALID_PTR_ORIGIN	SD_ERRORS_BASE + 10
#define SD_ENTRY_NOT_FILE		SD_ERRORS_BASE + 11
#define SD_INVALID_FILENAME		SD_ERRORS_BASE + 12
#define SD_INVALID_FAT_APPEND	SD_ERRORS_BASE + 13
#define SD_FILE_ALREADY_EXISTS	SD_ERRORS_BASE + 14
#define SD_INVALID_FILE_MODE	SD_ERRORS_BASE + 15
#define SD_TOO_MANY_FATS		SD_ERRORS_BASE + 16
#define SD_READING_PAST_EOC		SD_ERRORS_BASE + 17
#define SD_FILE_WITHOUT_BUFFER	SD_ERRORS_BASE + 18
#define SD_ERRORS_SIZE			SD_ERRORS_BASE + 19

// Forward declarations for buffers and files
typedef struct _sd_buffer sd_buffer;
typedef struct _sd_file sd_file;

// In case the system is low on RAM, allow the external program to access the generic buffer.
extern sd_buffer g_sd_buf;

/**
 * \brief		Initialize SD card communication over SPI for 3.3V configuration
 *
 * \detailed	Starts an SPI cog IFF an SPI cog has not already been started; If one has
 * 				been started, only the cs parameter will have effect
 *
 * \param	mosi		Pin mask for MOSI pin
 * \param	miso		Pin mask for MISO pin
 * \param	sclk		Pin mask for SCLK pin
 * \param	cs			Pin mask for CS pin
 * \param	freq		Frequency to run the clock after initialization IFF SPI_FAST is
 * 						disabled in spi.h; if (-1) is used, a system default will be used
 *
 * \return		Returns 0 upon success, otherwise error code
 */
uint8_t SDStart (const uint32_t mosi, const uint32_t miso, const uint32_t sclk,
		const uint32_t cs, const uint32_t freq);

/**
 * \brief	Mount either FAT16 or FAT32 filesystem
 *
 * \return		Returns 0 upon success, error code otherwise
 */
uint8_t SDMount (void);

#ifdef SD_FILE_WRITE
/**
 * \brief	Stop all SD activities and write any modified buffers
 *
 * \pre		All files must be explicitely closed before
 *
 * \return		Returns 0 upon success, error code otherwise
 */
uint8_t SDUnmount (void);
#endif

/**
 * \brief	Change the current working directory to *f (similar to 'cd f')
 *
 * \param	*d			Short filename of directory to change to
 *
 * \return		Returns 0 upon success, error code otherwise
 */
uint8_t SDchdir (const char *d);

/**
 * \brief	Open a file with a given name and load its information into the file pointer
 *
 * \detailed	Load the first sector of a file into the file buffer; Initialize global
 *				character pointers; NOTE: currently, only one file mode is supported and
 *				is best described as "r+"; NOTE: two position pointers are used, one for
 *				writing and one for reading, this may be changed later to comply with
 *				POSIX standards but is useful for my own purposes at the moment
 *				NOTE: This driver does not include any provision for timestamps; Niether
 *				file modification or creation will changed file's timestamp data (creation
 *				times are random and uninitialized)
 *
 * \pre		Files cannot be created in the root directory of a FAT16 filesystem
 * 			TODO: Fix this
 *
 * \param	*name	C-string containing the filename to open
 * \param	*f		Address where file information (such as the first allocation unit) can
 * 					be stored. Multiple files opened simultaneously is allowed.
 *
 * \return 		Returns 0 upon success, error code otherwise
 */
uint8_t SDfopen (const char *name, sd_file *f, const sd_file_mode mode);

#ifdef SD_FILE_WRITE
/**
 * \brief	Close a given file
 *
 * \param	*f		Address of the file object to close
 *
 * \return		Returns 0 upon success, error code otherwise
 */
uint8_t SDfclose (sd_file *f);
#endif

#ifdef SD_FILE_WRITE
/**
 * \brief	Insert a character into a given file
 *
 * \detailed	Insert 'c' at the location pointed to by the file's write pointer; Note: the read and
 * 				write pointers may be merged into one at a later date
 *
 * \param	c		Character to be inserted
 * \param	*f		Address of the desired file object
 *
 * \return
 */
uint8_t SDfputc (const char c, sd_file *f);

/**
 * \brief	Insert a c-string into a file
 *
 * \detailed	Insert an array of bytes into the file oject pointed to by 'f' beginning at address
 * 				's' until the value 0 is reached
 *
 * \param	*s		C-string to be inserted
 * \param	*f		Address of file object
 *
 * \return		Returns 0 upon success, otherwise error code
 */
uint8_t SDfputs (char *s, sd_file *f);
#endif

/**
 * \brief	Read one character from the currently opened file.
 *
 * \detailed	NOTE: This function does not include error checking
 *
 * \pre		*f must point to a currently opened and valid file
 * \pre		The file must have at least one byte left - no error checking is performed to
 * 			stop the user from reading past the end of a file (call SDfeof() for end-of-file
 * 			check)
 *
 * \param	*f		Address where file information (such as the first allocation unit) can be
 * 					stored.
 *
 * \return		Returns the character pointed to by the g_sd_rPtr pointer
 */
char SDfgetc (sd_file *f);

/**
 * \brief	Read a line from a file until either 'size' characters have been read or a
 * 			newline is found; Parameters should match stdio.h's fgets except for the file
 * 			pointer
 * 			NOTE: This function does not include error checking
 *
 * \pre		*f must point to a currently opened and valid file
 *
 * \param	s[]		Character array to store file characters
 * \param	size	Maximum number of characters to read from the file
 * \param	*f		Address with the currently opened file
 */
char * SDfgets (char s[], uint32_t size, sd_file *f);

/**
 * \brief	Determine whether the read pointer has reached the end of the file
 *
 * \Pre: *f must point to a currently opened and valid file
 *
 * \param	*f		Address of the requested file
 *
 * \return		Returns true if the read pointer points to the end of the file, false otherwise
 */
inline uint8_t SDfeof (sd_file *f);

/**
 * \brief	Set the read pointer for a given file to the position 'origin + offset'
 *
 * \Pre: *f must be an opened file
 *
 * \param	*f		Address of the file object being referenced
 * \param	offset	Bytes beyond 'origin' to set the pointer to
 * \param	origin	Gives a reference to the offset; can be one of SEEK_SET, SEEK_CUR, SEEK_END
 *
 * \return		Returns 0 upon success, error code otherwise
 */
uint8_t SDfseekr (sd_file *f, const int32_t offset, const file_pos origin);

/**
 * \brief	Set the write pointer for a given file to the position 'origin + offset'
 *
 * \Pre: *f must be an opened file
 *
 * \param	*f		Address of the file object being referenced
 * \param	offset	Bytes beyond 'origin' to set the pointer to
 * \param	origin	Gives a reference to the offset; can be one of SEEK_SET, SEEK_CUR, SEEK_END
 *
 * \return		Returns 0 upon success, error code otherwise
 */
uint8_t SDfseekw (sd_file *f, const int32_t offset, const file_pos origin);

/**
 * \brief	Retrieve the current position of the read pointer
 *
 * \Pre: *f must be an opened file
 *
 * \param	*f		Address of the file object being referenced
 *
 * \return		Returns the byte offset (from beginning) of the read pointer
 */
inline file_pos SDftellr (const sd_file *f);

/**
 * \brief	Retrieve the current position of the write pointer
 *
 * \Pre: *f must be an opened file
 *
 * \param	*f		Address of the file object being referenced
 *
 * \return		Returns the byte offset (from beginning) of the write pointer
 */
inline file_pos SDftellw (const sd_file *f);

#ifdef SD_SHELL
// Shell definitions
#define SD_SHELL_INPUT_LEN			64
#define SD_SHELL_CMD_LEN			8
#define SD_SHELL_ARG_LEN			32
#define SD_SHELL_EXIT				("exit")
#define SD_SHELL_LS					("ls")
#define SD_SHELL_CAT				("cat")
#define SD_SHELL_CD					("cd")
#define SD_SHELL_TOUCH				("touch")

/**
 * \brief	Provide the user with a very basic unix-like shell. The following commands
 *         are available to the user: ls, cat, cd.
 *
 * \param	*f		If a file is opened via a command such as 'cat', its information will
 * 					be stored at this address
 *
 * \return		Returns 0 upon success, error code otherwise
 */
uint8_t SD_Shell (sd_file *f);

/**
 * \brief	List the contents of a directory on the screen (similar to 'ls .')
 *
 * \param	*absPath	Absolute path of the directory to be printed
 *
 * \return		Returns 0 upon success, error code otherwise
 */
uint8_t SD_Shell_ls (void);

/**
 * \brief	Dump the contents of a file to the screen (similar to 'cat f');
 *
 * \Note: Does not currently follow paths
 *
 * \param	*f			Short filename of file to print
 *
 * \return		Returns 0 upon success, error code otherwise
 */
uint8_t SD_Shell_cat (const char *name, sd_file *f);

/**
 * \brief	Change the current working directory to *f (similar to 'cd f');
 *
 * \param	*d			Short filename of directory to change to
 *
 * \return		Returns 0 upon success, error code otherwise
 */
uint8_t SD_Shell_cd (const char *d);

#ifdef SD_FILE_WRITE
/**
 * \brief	Create a new file, do not open it
 *
 * \param	*name		C-string name for the file to be created
 */
uint8_t SD_Shell_touch (const char name[]);
#endif
#endif

#if (defined SD_VERBOSE || defined SD_VERBOSE_BLOCKS)
/**
 * \brief	Print a block of data in hex format to the screen in SD_LINE_SIZE-byte lines
 *
 * \param	*dat		Pointer to the beginning of the data
 * \param	bytes		Number of bytes to print
 *
 * \return		Returns 0 upon success, error code otherwise
 */
uint8_t SDPrintHexBlock (uint8_t *dat, uint16_t bytes);
#endif

/*******************************************
 *** Private SD Definitions & Prototypes ***
 *******************************************/
#if (defined SD_DEBUG || defined SD_VERBOSE || defined SD_VERBOSE_BLOCKS || defined SD_SHELL)
#include <stdio.h>
#endif

// SPI config
#define SD_SPI_INIT_FREQ			100000					// Run SD initialization at 200 kHz
#define SD_SPI_FINAL_FREQ			1900000					// Speed clock to 1.9 MHz after initialization
#define SD_SPI_MODE					SPI_MODE_0
#define SD_SPI_BITMODE				SPI_MSB_FIRST

// Misc. SD Definitions
#define SD_WIGGLE_ROOM				10000
#define SD_RESPONSE_TIMEOUT			CLKFREQ/10				// Wait 0.1 seconds for a response before timing out
#define SD_SECTOR_SIZE_SHIFT		9

// SD Commands
#define SD_CMD_IDLE					0x40 + 0				// Send card into idle state
#define	SD_CMD_SDHC					0x40 + 8				// Set SD card version (1 or 2) and voltage level range
#define SD_CMD_RD_CSD				0x40 + 9				// Request "Card Specific Data" block contents
#define SD_CMD_RD_CID				0x40 + 10				// Request "Card Identification" block contents
#define SD_CMD_RD_BLOCK				0x40 + 17				// Request data block
#define SD_CMD_WR_BLOCK				0x40 + 24				// Write data block
#define SD_CMD_READ_OCR				0x40 + 58				// Request "Operating Conditions Register" contents
#define SD_CMD_APP					0x40 + 55				// Inform card that following instruction is application specific
#define SD_CMD_WR_OP				0x40 + 41				// Send operating conditions for SDC
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
#define SD_RSPNS_TKN_BITS			0x0f
#define SD_RSPNS_TKN_ACCPT			((0x02 << 1) | 1)
#define SD_RSPNS_TKN_CRC			((0x05 << 1) | 1)
#define SD_RSPNS_TKN_WR				((0x06 << 1) | 1)

// Boot sector addresses/values
#define SD_FAT_16					2						// A FAT entry in FAT16 is 2-bytes
#define SD_FAT_32					4						// A FAT entry in FAT32 is 4-bytes
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
#define SD_FILE_ENTRY_LENGTH		32						// An entry in a directory uses 32 bytes
#define SD_DELETED_FILE_MARK		0xe5					// Marks that a file has been deleted here, continue to the next entry
#define SD_FILE_NAME_LEN			8						// 8 characters in the standard file name
#define SD_FILE_EXTENSION_LEN		3						// 3 character file name extension
#define SD_FILENAME_STR_LEN			SD_FILE_NAME_LEN + SD_FILE_EXTENSION_LEN + 2
#define SD_FILE_ATTRIBUTE_OFFSET	0x0B					// Byte of a file entry to store attribute flags
#define SD_FILE_START_CLSTR_LOW		0x1a					// Starting cluster number
#define SD_FILE_START_CLSTR_HIGH	0x14					// High word (16-bits) of the starting cluster number (FAT32 only)
#define SD_FILE_LEN_OFFSET			0x1c					// Length of a file in bytes
#define SD_FREE_CLUSTER				0						// Cluster is unused
#define SD_RESERVED_CLUSTER			1
#define SD_RSVD_CLSTR_VAL_BEG		-15						// First reserved cluster value
#define SD_RSVD_CLSTR_VAL_END		-9						// Last reserved cluster value
#define SD_BAD_CLUSTER				-8						// Cluster is corrupt
#define SD_EOC_BEG					-7						// First marker for end-of-chain (end of file entry within FAT)
#define SD_EOC_END					-1						// Last marker for end-of-chain
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

// File constants
#ifndef SD_EOF
#define SD_EOF						((uint8_t) -1)						// System dependent - may need to be defined elsewhere
#endif

#define SD_FOLDER_ID				((uint8_t) -1)						// Signal that the contents of a buffer are a directory

struct _sd_buffer {
	uint8_t buf[SD_SECTOR_SIZE];				// Buffer for SD card contents
	uint8_t id;				// Buffer ID - determine who owns the current information
	uint32_t curClusterStartAddr;	// Store the current cluster's starting sector number
	uint8_t curSectorOffset;  // Store the current sector offset from the beginning of the cluster
	uint32_t curAllocUnit;					// Store the current allocation unit
	uint32_t nextAllocUnit;					// Look-ahead at the next FAT entry
#ifdef SD_FILE_WRITE
	uint8_t mod;// When set, the currently loaded sector has been modified since it was read from
				// the SD card
#endif
};

struct _sd_file {
	sd_buffer *buf;
	uint8_t id;  // determine if the buffer is owned by this file
	file_pos wPtr;
	file_pos rPtr;
	sd_file_mode mode;
	uint32_t length;
	uint32_t maxSectors;	// Maximum number of sectors currently allocated to a file
	uint8_t mod;	// When the length of a file is changed, this variable will be set, otherwise cleared
	uint32_t firstAllocUnit;  // File's starting allocation unit
	uint32_t curSector;  // like curSectorOffset, but does not reset upon loading a new cluster
	uint32_t curCluster;  // like curSector, but for allocation units

	uint32_t dirSectorAddr;  // Which sector of the SD card contains this file's meta-data
	uint16_t fileEntryOffset;
};

/***********************************
 *** Private Function Prototypes ***
 ***********************************/
/**
 * \brief	Send a command and argument over SPI to the SD card
 *
 * \param    command       6-bit value representing the command sent to the SD card
 *
 * \return
 */
static uint8_t SDSendCommand (const uint8_t cmd, const uint32_t arg, const uint8_t crc);

/* brief	Receive response and data from SD card over SPI
 *
 * \param	bytes		Number of bytes to receive
 * \param	*data		Location in memory with enough space to store 'bytes' bytes of data
 *
 * \return		Returns 0 for success, else error code
 */
static uint8_t SDGetResponse (const uint8_t numBytes, uint8_t *dat);

/**
 * \brief	Receive data from SD card via SPI
 *
 * \param	bytes		Number of bytes to receive
 * \param	*data		Location in memory with enough space to store 'bytes' bytes of data
 *
 * \return		Returns 0 for success, else error code
 */
static uint8_t SDReadBlock (uint16_t bytes, uint8_t *dat);

/**
 * \brief	Write data to SD card via SPI
 *
 * \param	bytes		Block address to read from SD card
 * \param	*dat		Location in memory where data resides
 *
 * \return		Returns 0 upon success, error code otherwise
 */
static uint8_t SDWriteBlock (uint16_t bytes, uint8_t *dat);

/**
 * \brief	Read SD_SECTOR_SIZE-byte data block from SD card
 *
 * \param	address		Number of bytes to send
 * \param	*dat		Location in chip memory to store data block
 *
 * \return		Returns 0 upon success, error code otherwise
 */
static uint8_t SDReadDataBlock (uint32_t address, uint8_t *dat);

/**
 * \brief	Write SD_SECTOR_SIZE-byte data block to SD card
 *
 * \param	address		Block address to write to SD card
 * \param	*dat		Location in chip memory to read data block
 *
 * \return		Returns 0 upon success, error code otherwise
 */
static uint8_t SDWriteDataBlock (uint32_t address, uint8_t *dat);

/**
 * \brief	Return byte-reversed 16-bit variable (SD cards store bytes little-endian therefore we must
 * 		   reverse them to use multi-byte variables)
 *
 * \param	buf[]		Address of first byte of data
 *
 * \return		Returns a normal (big-endian) 16-bit word
 */
static uint16_t SDReadDat16 (const uint8_t buf[]);

/**
 * \brief	Return byte-reversed 32-bit variable (SD cards store bytes little-endian therefore we must
 * 		   reverse them to use multi-byte variables)
 *
 * \param	buf[]		Address of first byte of data
 *
 * \return	Returns a normal (big-endian) 32-bit word
 */
static uint32_t SDReadDat32 (const uint8_t buf[]);

#ifdef SD_FILE_WRITE
/**
 * \brief	Write a byte-reversed 16-bit variable (SD cards store bytes little-endian therefore we must
 *			reverse them to use multi-byte variables)
 *
 * \param	buf[]		Address to store the first byte of data
 * \param	dat			Normal, 16-bit variable to be written to RAM in reverse endian
 */
void SDWriteDat16 (uint8_t buf[], const uint16_t dat);

/**
 * \brief	Write a byte-reversed 32-bit variable (SD cards store bytes little-endian therefore we must
 *			reverse them to use multi-byte variables)
 *
 * \param	buf[]		Address to store the first byte of data
 * \param	dat			Normal, 32-bit variable to be written to RAM in reverse endian
 */
void SDWriteDat32 (uint8_t buf[], const uint32_t dat);
#endif

/**
 * \brief	Find and return the starting sector's address for a directory path given in a c-string. Use
 *         Unix-style path names (like /foo/bar/)
 *
 * \param	*path		C-string representing Unix-style path
 *
 * \return		Returns sector address of desired path
 */
// TODO: Implement this (more than simply returning root directory)
// TODO: Allow for paths outside the current directory
static uint32_t SDGetSectorFromPath (const char *path);

/**
 * \brief	Find and return the starting sector's address for a given allocation unit (note - not cluster)
 *
 * \param	allocUnit	Allocation unit in FAT filesystem
 *
 * \return		Returns sector address of desired allocation unit
 */
static uint32_t SDGetSectorFromAlloc (uint32_t allocUnit);

/**
 * \brief	Read an entry from the FAT
 *
 * \param	fatEntry		Entry number (allocation unit) to read in the FAT
 * \param	*value			Address to store the value into (the next allocation unit)
 *
 * \return		Returns 0 upon success, error code otherwise
 */
static uint8_t SDGetFATValue (const uint32_t fatEntry, uint32_t *value);

/**
 * \brief	Find the next sector in the FAT, directory, or file. When it is found, load it into the
 *         appropriate global buffer
 *
 * \param	*buf		Array of SD_SECTOR_SIZE bytes that can be filled with the requested sector
 *
 * \return		Returns 0 upon success, otherwise error code
 */
static uint8_t SDLoadNextSector (sd_buffer *buf);

/**
 * \brief	Load a requested sector into the buffer independent of the current sector or cluster
 *
 * \Pre: *buf must point to a file entry (g_sd_file_
 *
 * \param	*buf		Address of the sd_buffer object to be updated
 * \param	offset		How many sectors past the first one should be skipped (sector number of the file)
 *
 */
static uint8_t SDLoadSectorFromOffset (sd_file *f, const uint32_t offset);

/**
 * \brief	When the final sector of a cluster is finished, SDIncCluster can be called. The appropriate
 *         global variables will be set according (incremented or set by the FAT) and the first sector
 *         of the next cluster will be read into the desired buffer.
 *
 * \param	*buf		Array of SD_SECTOR_SIZE bytes used to hold a sector from the SD card
 *
 * \return		Returns 0 upon success, error code otherwise
 */
static uint8_t SDIncCluster (sd_buffer *buf);

/**
 * \brief	Read the standard length name of a file entry. If an extension exists, a period will be
 *         inserted before the extension. A null-terminator is always appended to the end
 *
 * \param	*buf		First byte in local memory containing a FAT entry
 * \param	*filename	Address in memory where the filename string will be stored
 *
 * \Pre: *buf must point to the first byte in a FAT entry - no error checking is executed on buf
 * \Pre: Errors may occur if at least 13 (8 + 1 + 3 + 1) bytes of memory are not allocated for filename
 *
 * \return
 */
static void SDGetFilename (const uint8_t *buf, char *filename);

/**
 * \brief	Find a file entry (file or sub-directory)
 *
 * \detailed	Find a file or directory that matches the name in *filename in the current directory;
 *         its relative location is communicated by placing it in the address of *fileEntryOffset
 *
 * \param	*filename			C-string representing the short (standard) filename
 * \param	*fileEntryOffset	The buffer offset will be returned via this address if the file
 * 								is found
 *
 * \return		Returns 0 upon success, error code otherwise (common error code is SD_EOC_END for
 *              end-of-chain or file-not-found marker)
 */
static uint8_t SDFind (const char *filename, uint16_t *fileEntryOffset);

/**
 * \brief	Reload the sector currently in use by a given file
 *
 * \param	*f		Address of the file object requested
 *
 * \return		Returns 0 upon success, error code otherwise
 */
static uint8_t SDReloadBuf (sd_file *f);

#ifdef SD_FILE_WRITE
/**
 * \brief		Find the first empty allocation unit in the FAT
 *
 * \detailed	The value of the first empty allocation unit is returned and its location
 *				will contain the end-of-chain marker, SD_EOC_END. NOTE: It is important
 *				to realize that, though the new entry now contains an EOC marker, this
 *				function does not know what cluster is being extended and therefore the
 *				calling function must modify the previous EOC to contain the return value
 *
 * \param	restore		If non-zero, the original fat-sector will be restored to g_sd_fat
 * 						before returning; if zero, the last-used sector will remain loaded
 *
 * \return		Returns the number of the first unused allocation unit
 */
static uint32_t SDFindEmptySpace (const uint8_t restore);

/* \brief	Enlarge a file or directory by one cluster
 *
 * \pre
 *
 * \param	*buf		Address of the buffer (containing information for a file or directory)
 * 						to be enlarged
 *
 * \return		Returns 0 upon success, error code otherwise
 */
static uint8_t SDExtendFAT (sd_buffer *buf);

/**
 * \brief	Allocate space for a new file
 *
 * \param	*name				Character array for the new file
 * \param	*fileEntryOffset	Offset from the currently loaded directory entry where
 * 								the file's metadata should be written
 */
static uint8_t SDCreateFile (const char *name, const uint16_t *fileEntryOffset);
#endif

#if (defined SD_SHELL || defined SD_VERBOSE)
// TODO: Document this
static inline void SDPrintFileEntry (const uint8_t *file, char filename[]);

// TODO: Document this
static void SDPrintFileAttributes (const uint8_t flag);
#endif

#ifdef SD_DEBUG
/* \brief	Print an error through UART string followed by entering an infinite loop
 *
 * \param	err		Error number used to determine error string
 */
static void SDError (const uint8_t err);

/**
 * \brief	Print to screen each status bit individually with human-readable descriptions
 *
 * \param	response		first-byte response from the SD card
 */
static void SDFirstByteExpansion (const uint8_t response);
#else
// Exit calling function by returning 'err'
#define SDError(err)				return err
#endif

#endif /* SD_H_ */
