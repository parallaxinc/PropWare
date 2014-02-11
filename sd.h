/**
 * @file        sd.h
 */
/**
 * @brief       SDHC driver for FAT16 and FAT32 for the Parallax Propeller
 *
 * @project     PropWare
 *
 * @author      David Zemon
 *
 * @date        Spring 2013
 *
 * @pre         The SD card must be SDHC v2 and must be formatted to FAT16 or
 *              FAT32
 *
 * @warning     Unknown result if card is not SDHC v2
 *
 * TODO:    Re-arrange errors in order of impact level; Allows the user to do
 *          something like:
 *              if ((SD_ERRORS_BASE + 6) < (err = SDFoo()))
 *                 throw(err);
 *          which would ignore any error less than 6
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

#ifndef SD_H_
#define SD_H_

#include <propeller.h>
#include <stdlib.h>
#include <string.h>
#include <PropWare.h>
#include <spi.h>

/**
 * @defgroup _propware_sd           FAT16/32 SD card
 @{*/

/**
 * @publicsection @{
 */

/** @name   SD Extra Code Options
 * @{ */
/**
 * Enables thorough debugging features similar to exceptions; Errors caught by
 * the program will enter an infinite debug loop
 * <p>
 * DEFAULT: Off
 */
#define SD_OPTION_DEBUG
// This allows Doxygen to document the macro without permanently enabling it
#undef SD_OPTION_DEBUG
/**
 * Enables thorough debugging features similar to exceptions; Errors will be
 * caught the program will enter an infinite loop
 * <p>
 * DEFAULT: Off
 */
#define SD_OPTION_VERBOSE
// This allows Doxygen to document the macro without permanently enabling it
#undef SD_OPTION_VERBOSE
/**
 * Select data blocks/sectors will be display via UART for debugging purposes
 * <p>
 * DEFAULT: Off
 */
#define SD_OPTION_VERBOSE_BLOCKS
// This allows Doxygen to document the macro without permanently enabling it
#undef SD_OPTION_VERBOSE_BLOCKS
/**
 * Unix-like command-line arguments will be defined and available
 * <p>
 * DEFAULT: On
 */
#define SD_OPTION_SHELL
/**
 * Allows for files to be created and written to
 * <p>
 * DEFAULT: ON
 *
 * @note    Adds noticeable code size
 * @note    Work-in-progress, code size is not necessarily at a minimum, nor is
 *          RAM usage
 */
#define SD_OPTION_FILE_WRITE
/** @} */

/** Number of characters printed to the terminal before a line break */
#define SD_LINE_SIZE            16
/** Number of bytes in a sector of the SD card */
#define SD_SECTOR_SIZE          512
/** Default frequency to run the SPI module */
#define SD_DEFAULT_SPI_FREQ     1800000

/**
 * File modes
 *
 * TODO: Learn what these modes *should* do and do it; At the moment, these modes
 *       essentially aren't used for anything
 */
typedef enum {
    /**
     * Read only; Read pointer starts at first character
     */
    SD_FILE_MODE_R,
#ifdef SD_OPTION_FILE_WRITE
    /**
     * Read+ (read + write); Read and write pointers both start at first
     * character
     */
    SD_FILE_MODE_R_PLUS,
    /**
     * Append (write only); Write pointer starts at last character + 1
     */
    SD_FILE_MODE_A,
    /**
     * Append+ (read + write); Write pointer starts at last character + 1, read
     * pointer starts at first character
     */
    SD_FILE_MODE_A_PLUS,
#endif
    /** Total number of different file modes */
    SD_FILE_MODES
} SD_FileMode;

/**
 * File Positions
 */
typedef enum {
    /** Beginning of the file */SEEK_SET,
    /** Current position in the file */SEEK_CUR,
    /** End of the file */SEEK_END
} SD_FilePos;

/** Number of allocated error codes for SD */
#define SD_ERRORS_LIMIT     32
/** First SD error code */
#define SD_ERRORS_BASE      16

/**
 * Error codes - preceded by SPI
 */
typedef enum {
    /** SD Error  0 */SD_INVALID_CMD = SD_ERRORS_BASE,
    /** SD Error  1 */SD_READ_TIMEOUT,
    /** SD Error  2 */SD_INVALID_NUM_BYTES,
    /** SD Error  3 */SD_INVALID_RESPONSE,
    /** SD Error  4 */SD_INVALID_INIT,
    /** SD Error  5 */SD_INVALID_FILESYSTEM,
    /** SD Error  6 */SD_INVALID_DAT_STRT_ID,
    /** SD Error  7 */SD_FILENAME_NOT_FOUND,
    /** SD Error  8 */SD_EMPTY_FAT_ENTRY,
    /** SD Error  9 */SD_CORRUPT_CLUSTER,
    /** SD Error 10 */SD_INVALID_PTR_ORIGIN,
    /** SD Error 11 */SD_ENTRY_NOT_FILE,
    /** SD Error 12 */SD_INVALID_FILENAME,
    /** SD Error 13 */SD_INVALID_FAT_APPEND,
    /** SD Error 14 */SD_FILE_ALREADY_EXISTS,
    /** SD Error 15 */SD_INVALID_FILE_MODE,
    /** SD Error 16 */SD_TOO_MANY_FATS,
    /** SD Error 17 */SD_READING_PAST_EOC,
    /** SD Error 18 */SD_FILE_WITHOUT_BUFFER,
    /** SD Error 19 */SD_CMD8_FAILURE
} SD_ErrorCode;

/**
 * Buffer object used for storing SD data; Each instance uses 527 bytes (526
 * if SD_OPTION_FILE_WRITE is disabled)
 */
typedef struct _SD_Buffer SD_Buffer;

/**
 * SD file object
 *
 * @note    Must be initialized with an sd_buffer object before use; If one has
 *          not been explicitly created then the global buffer, g_sd_buf, can be
 *          used at the expense of decreased performance
 */
typedef struct _sd_file SD_File;

/**
 * In case the system is low on RAM, allow the external program to access the
 * generic buffer
 */
extern SD_Buffer g_sd_buf;

/**
 * @brief       Initialize SD card communication over SPI for 3.3V configuration
 *
 * @detailed    Starts an SPI cog IFF an SPI cog has not already been started;
 *              If one has been started, only the cs parameter will have effect
 *
 * @param[in]   mosi        Pin mask for MOSI pin
 * @param[in]   miso        Pin mask for MISO pin
 * @param[in]   sclk        Pin mask for SCLK pin
 * @param[in]   cs          Pin mask for CS pin
 * @param[in]   freq        Frequency to run the clock after initialization IFF
 *                          SPI_FAST is disabled in spi.h; if (-1) is used, a
 *                          system default will be used
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t sd_start (const uint32_t mosi, const uint32_t miso, const uint32_t sclk,
        const uint32_t cs, const uint32_t freq);

/**
 * @brief   Mount either FAT16 or FAT32 filesystem
 *
 * @return  Returns 0 upon success, error code otherwise
 */
uint8_t sd_mount (void);

#ifdef SD_OPTION_FILE_WRITE
/**
 * @brief   Stop all SD activities and write any modified buffers
 *
 * @pre     All files must be explicitly closed before
 *
 * @return  Returns 0 upon success, error code otherwise
 */
uint8_t sd_unmount (void);
#endif

/**
 * @brief       Change the current working directory to *d (similar to 'cd dir')
 *
 * @detailed    At the moment, the target directory must be an immediate child
 *              of the current directory ("." and ".." are allowed). I hope to
 *              implement the ability to change to any directory soon (such as
 *              "cd ../siblingDirectory") but attempting to do this now would
 *              currently result in an SD_FILENAME_NOT_FOUND error
 *
 * @param[in]   *d     Short filename of directory to change to
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t sd_chdir (const char *d);

/**
 * @brief       Open a file with a given name and load its information into the
 *              file pointer
 *
 * @detailed    Load the first sector of a file into the file buffer; Initialize
 *              global character pointers; NOTE: currently, only one file mode
 *              is supported and is best described as "r+"; NOTE: two position
 *              pointers are used, one for writing and one for reading, this may
 *              be changed later to comply with POSIX standards but is useful
 *              for my own purposes at the moment
 *              NOTE: This driver does not include any provision for timestamps;
 *              Niether file modification or creation will changed file's
 *              timestamp data (creation times are random and uninitialized)
 *
 * @pre         Files cannot be created in the root directory of a FAT16
 *              filesystem
 *              TODO: Fix this
 *
 * @param[in]   *name   C-string containing the filename to open
 * @param[in]   *f      Address where file information (such as the first
 *                      allocation unit) can be stored. Multiple files opened
 *                      simultaneously is allowed.
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t sd_fopen (const char *name, SD_File *f, const SD_FileMode mode);

#ifdef SD_OPTION_FILE_WRITE
/**
 * @brief       Close a given file
 *
 * @param[in]   *f  Address of the file object to close
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t sd_fclose (SD_File *f);

/**
 * @brief       Insert a character into a given file
 *
 * @detailed    Insert 'c' at the location pointed to by the file's write
 *              pointer; Note: the read and write pointers may be merged into
 *              one at a later date
 *
 * @param[in]   c       Character to be inserted
 * @param[in]   *f      Address of the desired file object
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t sd_fputc (const char c, SD_File *f);

/**
 * @brief       Insert a c-string into a file
 *
 * @detailed    Insert an array of bytes into the file object pointed to by 'f'
 *              beginning at address 's' until the value 0 is reached
 *
 * @param[in]   *s  C-string to be inserted
 * @param[in]   *f  Address of file object
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t sd_fputs (char *s, SD_File *f);
#endif

/**
 * @brief       Read one character from the currently opened file.
 *
 * @detailed    NOTE: This function does not include error checking
 *
 * @pre         *f must point to a currently opened and valid file
 * @pre         The file must have at least one byte left - no error checking is
 *              performed to stop the user from reading past the end of a file
 *              (call SDfeof() for end-of-file check)
 *
 * @param[in]   *f  Address where file information (such as the first allocation
 *                  unit) can be stored.
 *
 * @return      Returns the character pointed to by the g_sd_rPtr pointer
 */
char sd_fgetc (SD_File *f);

/**
 * @brief       Read a line from a file until either 'size' characters have been
 *              read or a newline is found; Parameters should match stdio.h's
 *              fgets except for the file pointer
 *
 * @note        This function does not include error checking
 *
 * @pre         *f must point to a currently opened and valid file
 *
 * @param[out]  s[]     Character array to store file characters
 * @param[in]   size    Maximum number of characters to read from the file
 * @param[in]   *f      Address with the currently opened file
 *
 * @return      Returns character memory location of character array
 */
char * sd_fgets (char s[], uint32_t size, SD_File *f);

/**
 * @brief       Determine whether the read pointer has reached the end of the
 *              file
 *
 * @pre         *f must point to a currently opened and valid file
 *
 * @param[in]   *f  Address of the requested file
 *
 * @return      Returns true if the read pointer points to the end of the file,
 *              false otherwise
 */
inline uint8_t sd_feof (SD_File *f);

/**
 * @brief       Set the read pointer for a given file to the position 'origin +
 *              offset'
 *
 * @pre         *f must be an opened file
 *
 * @param[in]   *f      Address of the file object being referenced
 * @param[in]   offset  Bytes beyond 'origin' to set the pointer to
 * @param[in]   origin  Gives a reference to the offset; can be one of SEEK_SET,
 *                      SEEK_CUR, SEEK_END
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t sd_fseekr (SD_File *f, const int32_t offset, const SD_FilePos origin);

/**
 * @brief       Set the write pointer for a given file to the position 'origin +
 *              offset'
 *
 * @pre         *f must be an opened file
 *
 * @param[in]   *f      Address of the file object being referenced
 * @param[in]   offset  Bytes beyond 'origin' to set the pointer to
 * @param[in]   origin  Gives a reference to the offset; can be one of SEEK_SET,
 *                      SEEK_CUR, SEEK_END
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t sd_fseekw (SD_File *f, const int32_t offset, const SD_FilePos origin);

/**
 * @brief       Retrieve the current position of the read pointer
 *
 * @pre         *f must be an opened file
 *
 * @param[in]   *f  Address of the file object being referenced
 *
 * @return      Returns the byte offset (from beginning) of the read pointer
 */
inline SD_FilePos sd_ftellr (const SD_File *f);

/**
 * @brief       Retrieve the current position of the write pointer
 *
 * @pre         *f must be an opened file
 *
 * @param[in]   *f  Address of the file object being referenced
 *
 * @return      Returns the byte offset (from beginning) of the write pointer
 */
inline SD_FilePos sd_ftellw (const SD_File *f);

#ifdef SD_OPTION_SHELL
/**
 * @name Shell Definitions
 * @{
 */
/** Maximum number of characters allowed at the command prompt */
#define SD_SHELL_INPUT_LEN          64
/**
 * Maximum number of characters for an individual command (does not include
 * parameters
 */
#define SD_SHELL_CMD_LEN            8
/** Maximum number of characters for each command argument */
#define SD_SHELL_ARG_LEN            32
/** String defining the "exit" command to quit the SD_Shell() function*/
#define SD_SHELL_EXIT               ("exit")
/** String defining the "ls" command to call SD_Shell_ls(); List dir contents */
#define SD_SHELL_LS                 ("ls")
/** String defining the "cat" command to call SD_Shell_cat(); Prints a file */
#define SD_SHELL_CAT                ("cat")
/** String defining the "cd" command to call SD_Shell_cd(); Change directory */
#define SD_SHELL_CD                 ("cd")
/** String defining the "touch" command; Creates an empty file */
#define SD_SHELL_TOUCH              ("touch")
/**@}*/

/**
 * @brief       Provide the user with a very basic Unix-like shell. The
 *              following commands are available to the user: ls, cat, cd.
 *
 * @param[in]   *f  If a file is opened via a command such as 'cat', its
 *                  information will be stored at this address
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t sd_shell (SD_File *f);

/**
 * @brief       List the contents of a directory on the screen (similar to 'ls
 *              .')
 *
 * @note        TODO: Implement *abspath when SDGetSectorFromPath() is
 *              functional
 *
 * @param[in]   *absPath    Absolute path of the directory to be printed
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t sd_shell_ls (void);

/**
 * @brief       Dump the contents of a file to the screen (similar to 'cat f');
 *
 * @note        Does not currently follow paths
 *
 * @param[in]   *f  Short filename of file to print
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t sd_shell_cat (const char *name, SD_File *f);

/**
 * @brief       Change the current working directory to *d (similar to 'cd dir');
 *
 * @param[in]   *d  Short filename of directory to change to
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t sd_shell_cd (const char *d);

#ifdef SD_OPTION_FILE_WRITE
/**
 * @brief       Create a new file, do not open it
 *
 * @param[in]   name[]  C-string name for the file to be created
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t sd_shell_touch (const char name[]);
#endif
#endif

#if (defined SD_OPTION_VERBOSE || defined SD_OPTION_VERBOSE_BLOCKS)
/**
 * @brief       Print a block of data in hex format to the screen in
 *              SD_LINE_SIZE-byte lines
 *
 * @param[in]   *dat       Pointer to the beginning of the data
 * @param[in]   bytes      Number of bytes to print
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t sd_print_hex_block (uint8_t *dat, uint16_t bytes);
#endif

/**@}*/

/*******************************************
 *** Private SD Definitions & Prototypes ***
 *******************************************/
/**
 * @privatesection
 * @{
 */
#if (defined SD_OPTION_DEBUG || defined SD_OPTION_VERBOSE || defined SD_OPTION_VERBOSE_BLOCKS || \
defined SD_Shell)
#include <stdio.h>
#endif

// SPI config
#define SD_SPI_INIT_FREQ            60000          // Run SD initialization at 200 kHz
#define SD_SPI_MODE                 SPI_MODE_0
#define SD_SPI_BITMODE              SPI_MSB_FIRST

// Misc. SD Definitions
#define SD_WIGGLE_ROOM              10000
#define SD_RESPONSE_TIMEOUT         CLKFREQ/10      // Wait 0.1 seconds for a response before timing out
#define SD_SECTOR_SIZE_SHIFT        9

// SD Commands
#define SD_CMD_IDLE                 0x40 + 0        // Send card into idle state
#define SD_CMD_INTERFASE_COND       0x40 + 8        // Send interface condition and host voltage range
#define SD_CMD_RD_CSD               0x40 + 9        // Request "Card Specific Data" block contents
#define SD_CMD_RD_CID               0x40 + 10       // Request "Card Identification" block contents
#define SD_CMD_RD_BLOCK             0x40 + 17       // Request data block
#define SD_CMD_WR_BLOCK             0x40 + 24       // Write data block
#define SD_CMD_READ_OCR             0x40 + 58       // Request "Operating Conditions Register" contents
#define SD_CMD_APP                  0x40 + 55       // Inform card that following instruction is application specific
#define SD_CMD_WR_OP                0x40 + 41       // Send operating conditions for SDC
// SD Arguments
#define SD_HOST_VOLTAGE_3V3         (((uint16_t) 0x01) << 8)
#define SD_R7_CHECK_PATTERN         0xAA
#define SD_ARG_CMD8                 (SD_HOST_VOLTAGE_3V3 | SD_R7_CHECK_PATTERN)
#define SD_ARG_LEN                  5

// SD CRCs
#define SD_CRC_IDLE                 0x95
#define SD_CRC_CMD8                 0x87            // CRC only valid for CMD8 argument of 0x000001AA
#define SD_CRC_ACMD                 0x77
#define SD_CRC_OTHER                0x01

// SD Responses
#define SD_RESPONSE_IDLE            0x01
#define SD_RESPONSE_ACTIVE          0x00
#define SD_DATA_START_ID            0xFE
#define SD_RESPONSE_LEN_R1          1
#define SD_RESPONSE_LEN_R3          5
#define SD_RESPONSE_LEN_R7          5
#define SD_RSPNS_TKN_BITS           0x0f
#define SD_RSPNS_TKN_ACCPT          ((0x02 << 1) | 1)
#define SD_RSPNS_TKN_CRC            ((0x05 << 1) | 1)
#define SD_RSPNS_TKN_WR             ((0x06 << 1) | 1)

// Boot sector addresses/values
#define SD_FAT_16                   2               // A FAT entry in FAT16 is 2-bytes
#define SD_FAT_32                   4               // A FAT entry in FAT32 is 4-bytes
#define SD_BOOT_SECTOR_ID           0xeb
#define SD_BOOT_SECTOR_ID_ADDR      0
#define SD_BOOT_SECTOR_BACKUP       0x1c6
#define SD_CLUSTER_SIZE_ADDR        0x0d
#define SD_RSVD_SCTR_CNT_ADDR       0x0e
#define SD_NUM_FATS_ADDR            0x10
#define SD_ROOT_ENTRY_CNT_ADDR      0x11
#define SD_TOT_SCTR_16_ADDR         0x13
#define SD_FAT_SIZE_16_ADDR         0x16
#define SD_TOT_SCTR_32_ADDR         0x20
#define SD_FAT_SIZE_32_ADDR         0x24
#define SD_ROOT_CLUSTER_ADDR        0x2c
#define SD_FAT12_CLSTR_CNT          4085
#define SD_FAT16_CLSTR_CNT          65525

// FAT file/directory values
#define SD_FILE_ENTRY_LENGTH        32              // An entry in a directory uses 32 bytes
#define SD_DELETED_FILE_MARK        0xe5            // Marks that a file has been deleted here, continue to the next entry
#define SD_FILE_NAME_LEN            8               // 8 characters in the standard file name
#define SD_FILE_EXTENSION_LEN       3               // 3 character file name extension
#define SD_FILENAME_STR_LEN         SD_FILE_NAME_LEN + SD_FILE_EXTENSION_LEN + 2
#define SD_FILE_ATTRIBUTE_OFFSET    0x0B            // Byte of a file entry to store attribute flags
#define SD_FILE_START_CLSTR_LOW     0x1a            // Starting cluster number
#define SD_FILE_START_CLSTR_HIGH    0x14            // High word (16-bits) of the starting cluster number (FAT32 only)
#define SD_FILE_LEN_OFFSET          0x1c            // Length of a file in bytes
#define SD_FREE_CLUSTER             0               // Cluster is unused
#define SD_RESERVED_CLUSTER         1
#define SD_RSVD_CLSTR_VAL_BEG       -15             // First reserved cluster value
#define SD_RSVD_CLSTR_VAL_END       -9              // Last reserved cluster value
#define SD_BAD_CLUSTER              -8              // Cluster is corrupt
#define SD_EOC_BEG                  -7              // First marker for end-of-chain (end of file entry within FAT)
#define SD_EOC_END                  -1              // Last marker for end-of-chain
// FAT file attributes (definitions with trailing underscore represent character for a cleared attribute flag)
#define SD_READ_ONLY                BIT_0
#define SD_READ_ONLY_CHAR           'r'
#define SD_READ_ONLY_CHAR_          'w'
#define SD_HIDDEN_FILE              BIT_1
#define SD_HIDDEN_FILE_CHAR         'h'
#define SD_HIDDEN_FILE_CHAR_        '.'
#define SD_SYSTEM_FILE              BIT_2
#define SD_SYSTEM_FILE_CHAR         's'
#define SD_SYSTEM_FILE_CHAR_        '.'
#define SD_VOLUME_ID                BIT_3
#define SD_VOLUME_ID_CHAR           'v'
#define SD_VOLUME_ID_CHAR_          '.'
#define SD_SUB_DIR                  BIT_4
#define SD_SUB_DIR_CHAR             'd'
#define SD_SUB_DIR_CHAR_            'f'
#define SD_ARCHIVE                  BIT_5
#define SD_ARCHIVE_CHAR             'a'
#define SD_ARCHIVE_CHAR_            '.'

// File constants
#ifndef SD_EOF
#define SD_EOF                      ((uint8_t) -1)  // System dependent - may need to be defined elsewhere
#endif

// Signal that the contents of a buffer are a directory
#define SD_FOLDER_ID                ((uint8_t) -1)

struct _SD_Buffer {
    /**  Buffer for SD card contents */
    uint8_t buf[SD_SECTOR_SIZE];
    /** Buffer ID - determine who owns the current information */
    uint8_t id;
    /** Store the current cluster's starting sector number */
    uint32_t curClusterStartAddr;
    /** Store the current sector offset from the beginning of the cluster */
    uint8_t curSectorOffset;
    /** Store the current allocation unit */
    uint32_t curAllocUnit;
    /** Look-ahead at the next FAT entry */
    uint32_t nextAllocUnit;
#ifdef SD_OPTION_FILE_WRITE
    /**
     * When set, the currently loaded sector has been modified since it was
     * read from the SD card
     */
    uint8_t mod;
#endif
};

struct _sd_file {
    SD_Buffer *buf;
    /** determine if the buffer is owned by this file */
    uint8_t id;  //
    SD_FilePos wPtr;
    SD_FilePos rPtr;
    SD_FileMode mode;
    uint32_t length;
    /** Maximum number of sectors currently allocated to a file */
    uint32_t maxSectors;
    /**
     * When the length of a file is changed, this variable will be set,
     * otherwise cleared
     */
    uint8_t mod;
    /** File's starting allocation unit */
    uint32_t firstAllocUnit;
    /** like curSectorOffset, but does not reset upon loading a new cluster */
    uint32_t curSector;
    /** like curSector, but for allocation units */
    uint32_t curCluster;
    /** Which sector of the SD card contains this file's meta-data */
    uint32_t dirSectorAddr;
    /** Address within the sector of this file's entry */
    uint16_t fileEntryOffset;
};

/***********************************
 *** Private Function Prototypes ***
 ***********************************/
/**
 * @brief       Send a command and argument over SPI to the SD card
 *
 * @param[in]   command     6-bit value representing the command sent to the SD
 *                          card
 * @param[in]   arg         Any argument applicable to the command
 * @param[in]   crc         CRC for the command and argument
 *
 * @return      Returns 0 for success, else error code
 */
uint8_t sd_send_command (const uint8_t cmd, const uint32_t arg,
                       const uint8_t crc);

/**
 * @brief       receive response and data from SD card over SPI
 *
 * @param[in]   bytes   Number of bytes to receive
 * @param[out]  *data   Location in memory with enough space to store `bytes`
 *                      bytes of data
 *
 * @return      Returns 0 for success, else error code
 */
uint8_t sd_get_response (const uint8_t numBytes, uint8_t *dat);

/**
 * @brief       Receive data from SD card via SPI
 *
 * @param[in]   bytes   Number of bytes to receive
 * @param[out]  *data   Location in memory with enough space to store `bytes`
 *                      bytes of data
 *
 * @return      Returns 0 for success, else error code
 */
uint8_t sd_read_block (uint16_t bytes, uint8_t *dat);

/**
 * @brief       Write data to SD card via SPI
 *
 * @param[in]   bytes   Block address to read from SD card
 * @param[in]   *dat    Location in memory where data resides
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t sd_write_block (uint16_t bytes, uint8_t *dat);

/**
 * @brief       Read SD_SECTOR_SIZE-byte data block from SD card
 *
 * @param[in]   address    Number of bytes to send
 * @param[out]  *dat       Location in chip memory to store data block
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t sd_read_data_block (uint32_t address, uint8_t *dat);

/**
 * @brief       Write SD_SECTOR_SIZE-byte data block to SD card
 *
 * @param[in]   address     Block address to write to SD card
 * @param[in]   *dat        Location in chip memory to read data block
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t sd_write_data_block (uint32_t address, uint8_t *dat);

/**
 * @brief       Return byte-reversed 16-bit variable (SD cards store bytes
 *              little-endian therefore we must reverse them to use multi-byte
 *              variables)
 *
 * @param[in]   buf[]   Address of first byte of data
 *
 * @return      Returns a normal (big-endian) 16-bit word
 */
uint16_t sd_read_rev_dat16 (const uint8_t buf[]);

/**
 * @brief       Return byte-reversed 32-bit variable (SD cards store bytes
 *              little-endian therefore we must reverse them to use multi-byte
 *              variables)
 *
 * @param[in]   buf[]   Address of first byte of data
 *
 * @return      Returns a normal (big-endian) 32-bit word
 */
uint32_t sd_read_rev_dat32 (const uint8_t buf[]);

#ifdef SD_OPTION_FILE_WRITE
/**
 * @brief       Write a byte-reversed 16-bit variable (SD cards store bytes
 *              little-endian therefore we must reverse them to use multi-byte
 *              variables)
 *
 * @param[out]  buf[]   Address to store the first byte of data
 * @param[in]   dat     Normal, 16-bit variable to be written to RAM in reverse
 *                      endian
 */
void sd_write_rev_dat16 (uint8_t buf[], const uint16_t dat);

/**
 * @brief       Write a byte-reversed 32-bit variable (SD cards store bytes
 *              little-endian therefore we must reverse them to use multi-byte
 *              variables)
 *
 * @param[out]  buf[]   Address to store the first byte of data
 * @param[in]   dat     Normal, 32-bit variable to be written to RAM in reverse
 *                      endian
 */
void sd_write_rev_dat32 (uint8_t buf[], const uint32_t dat);
#endif

/**
 * @brief       Find and return the starting sector's address for a directory
 *              path given in a c-string. Use Unix-style path names (like
 *              /foo/bar/)
 *
 * @note        !!!Not yet implemented!!!
 *
 * @param[in]   *path   C-string representing Unix-style path
 *
 * @return      Returns sector address of desired path
 */
// TODO: Implement this (more than simply returning root directory)
// TODO: Allow for paths outside the current directory
uint32_t sd_find_sector_from_path (const char *path);

/**
 * @brief       Find and return the starting sector's address for a given
 *              allocation unit (note - not cluster)
 *
 * @param[in]   allocUnit   Allocation unit in FAT filesystem
 *
 * @return      Returns sector address of the desired allocation unit
 */
uint32_t sd_find_sector_from_alloc (uint32_t allocUnit);

/**
 * @brief       Read an entry from the FAT
 *
 * @param[in]   fatEntry    Entry number (allocation unit) to read in the FAT
 * @param[out]  *value      Address to store the value into (the next allocation
 *                          unit)
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t sd_get_fat_value (const uint32_t fatEntry, uint32_t *value);

/**
 * @brief       Find the next sector in the FAT, directory, or file. When it is
 *              found, load it into the appropriate global buffer
 *
 * @param[out]  *buf    Array of `SD_SECTOR_SIZE` bytes that can be filled with
 *                      the requested sector
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t sd_load_next_sector (SD_Buffer *buf);

/**
 * @brief       Load a requested sector into the buffer independent of the
 *              current sector or cluster
 *
 * @pre         *buf must point to a file entry (g_sd_file_
 *
 * @param[out]  *buf    Address of the sd_buffer object to be updated
 * @param[in]   offset  How many sectors past the first one should be skipped
 *                      (sector number of the file)
 *
 * @return      Returns 0 upon success, error code otherwise
 *
 */
uint8_t sd_load_sector_from_offset (SD_File *f, const uint32_t offset);

/**
 * @brief       Read the next sector from SD card into memory
 * @detailed    When the final sector of a cluster is finished, SDIncCluster can
 *              be called. The appropriate global variables will be set
 *              according (incremented or set by the FAT) and the first sector
 *              of the next cluster will be read into the desired buffer.
 *
 * @param[out]  *buf    Array of `SD_SECTOR_SIZE` bytes used to hold a sector
 *                      from the SD card
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t sd_inc_cluster (SD_Buffer *buf);

/**
 * @brief       Read the standard length name of a file entry. If an extension
 *              exists, a period will be inserted before the extension. A null-
 *              terminator is always appended to the end
 *
 * @pre         *buf must point to the first byte in a FAT entry - no error
 *              checking is executed on buf
 * @pre         Errors may occur if at least 13 (8 + 1 + 3 + 1) bytes of memory
 *              are not allocated for filename
 *
 * @param[in]   *buf        First byte in local memory containing a FAT entry
 * @param[out]  *filename   Address in memory where the filename string will be
 *                          stored
 */
void sd_get_filename (const uint8_t *buf, char *filename);

/**
 * @brief       Find a file entry (file or sub-directory)
 *
 * @detailed    Find a file or directory that matches the name in *filename in
 *              the current directory; its relative location is communicated by
 *              placing it in the address of *fileEntryOffset
 *
 * @param[in]   *filename           C-string representing the short (standard)
 *                                  filename
 * @param[out]  *fileEntryOffset    The buffer offset will be returned via this
 *                                  address if the file is found
 *
 * @return      Returns 0 upon success, error code otherwise (common error code
 *              is SD_EOC_END for end-of-chain or file-not-found marker)
 */
uint8_t sd_find (const char *filename, uint16_t *fileEntryOffset);

/**
 * @brief       Reload the sector currently in use by a given file
 *
 * @param[in]   *f  Address of the file object requested
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t sd_reload_buf (SD_File *f);

#ifdef SD_OPTION_FILE_WRITE
/**
 * @brief       Find the first empty allocation unit in the FAT
 *
 * @detailed    The value of the first empty allocation unit is returned and its
 *              location will contain the end-of-chain marker, SD_EOC_END.
 *              NOTE: It is important to realize that, though the new entry now
 *              contains an EOC marker, this function does not know what cluster
 *              is being extended and therefore the calling function must
 *              modify the previous EOC to contain the return value
 *
 * @param[in]   restore     If non-zero, the original fat-sector will be
 *                          restored to g_sd_fat before returning; if zero, the
 *                          last-used sector will remain loaded
 *
 * @return      Returns the number of the first unused allocation unit
 */
uint32_t sd_find_empty_space (const uint8_t restore);

/* @brief       Enlarge a file or directory by one cluster
 *
 * @param[in]   *buf    Address of the buffer (containing information for a
 *                      file or directory) to be enlarged
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t sd_extend_fat (SD_Buffer *buf);

/**
 * @brief       Allocate space for a new file
 *
 * @param[in]   *name               Character array for the new file
 * @param[in]   *fileEntryOffset    Offset from the currently loaded directory
 *                                  entry where the file's metadata should be
 *                                  written
 *
 * @return      Returns 0 upon success, error code otherwise
 */
uint8_t sd_create_file (const char *name, const uint16_t *fileEntryOffset);
#endif

#if (defined SD_OPTION_SHELL || defined SD_OPTION_VERBOSE)
/**
 * @brief       Print the attributes and name of a file entry
 *
 * @param[in]   *fileEntry  Address of the first byte of the file entry
 * @param[out]  *filename   Allocated space for the filename string to be stored
 */
inline void sd_print_file_entry (const uint8_t *fileEntry, char filename[]);

/**
 * @brief       Print attributes of a file entry
 *
 * @param[in]
 */
void sd_print_file_attributes (const uint8_t flags);
#endif

/**@}*/

/**@}*/

#endif /* SD_H_ */
