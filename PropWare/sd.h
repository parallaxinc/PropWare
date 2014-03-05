/**
 * @file        sd.h
 *
 * @project     PropWare
 *
 * @author      David Zemon
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
#include <tinyio.h>
#include <PropWare/PropWare.h>
#include <PropWare/safeSpi.h>

namespace PropWare {

/** @name   SD Extra Code Options
 * @{ */
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

/**
 * @brief   SDHC driver for FAT16 and FAT32 for the Parallax Propeller
 *
 * @pre     The SD card must be SDHC v2 and must be formatted to FAT16 or FAT32;
 *          SD v1 cards will throw an error at SD::start(); non-FAT partitions
 *          will yield unknown results
 *
 * TODO:    Re-arrange errors in order of impact level; Allows the user to do
 *          something like:
 *              if ((SD_ERRORS_BASE + 6) < (err = SDFoo()))
 *                 throw(err);
 *          which would ignore any error less than 6
 */
class SD {
    public:
        static const uint8_t PROPWARE_OBJECT_NUMBER = 1;
    public:
        /** Number of characters printed to the terminal before a line break */
        static const uint8_t LINE_SIZE = 16;
        /**
         * Number of bytes in a sector of the SD card
         * Need a pre-processor macro here for static allocation of
         * SD::Buffer.buf
         */
#define SD_SECTOR_SIZE  512
        static const uint16_t SECTOR_SIZE = SD_SECTOR_SIZE;
        /** Default frequency to run the SPI module */
        static const uint32_t DEFAULT_SPI_FREQ = 900000;

        // Signal that the contents of a buffer are a directory
        static const int8_t FOLDER_ID = -1;

#ifdef SD_OPTION_SHELL
        /**
         * @name Shell Definitions
         * @{
         */
        /** Maximum number of characters allowed at the command prompt */
#define SD_SHELL_INPUT_LEN  128
        static const uint8_t SHELL_INPUT_LEN = SD_SHELL_INPUT_LEN;
        /**
         * Maximum number of characters for an individual command (does not include
         * parameters
         */
#define SD_SHELL_CMD_LEN    8
        static const uint8_t SHELL_CMD_LEN = SD_SHELL_CMD_LEN;
        /** Maximum number of characters for each command argument */
#define SD_SHELL_ARG_LEN    64
        static const uint8_t SHELL_ARG_LEN = SD_SHELL_ARG_LEN;
        /** String defining the "exit" command to quit the SD_Shell() function*/
        static const char SHELL_EXIT[];
        /** String defining the "ls" command to call SD_Shell_ls(); List dir contents */
        static const char SHELL_LS[];
        /** String defining the "cat" command to call SD_Shell_cat(); Prints a file */
        static const char SHELL_CAT[];
        /** String defining the "cd" command to call SD_Shell_cd(); Change directory */
        static const char SHELL_CD[];
        /** String defining the "touch" command; Creates an empty file */
        static const char SHELL_TOUCH[];
        /**@}*/
#endif

    public:
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
            FILE_MODE_R,
#ifdef SD_OPTION_FILE_WRITE
            /**
             * Read+ (read + write); Read and write pointers both start at first
             * character
             */
            FILE_MODE_R_PLUS,
            /**
             * Append (write only); Write pointer starts at last character + 1
             */
            FILE_MODE_A,
            /**
             * Append+ (read + write); Write pointer starts at last character + 1, read
             * pointer starts at first character
             */
            FILE_MODE_A_PLUS,
#endif
            /** Total number of different file modes */
            FILE_MODES
        } FileMode;

        /**
         * File Positions
         */
        typedef enum {
            /** Beginning of the file */SEEK_SET,
            /** Current position in the file */SEEK_CUR,
            /** End of the file */SEEK_END
        } FilePos;

        /**
         * Error codes - preceded by SPI
         */
        typedef enum {
            /** First SD error code */BEG_ERROR = SPI::END_ERROR + 1,
            /** Begin user errors */ BEG_USER_ERROR = SD::BEG_ERROR,
            /** SD Error  0 */FILE_ALREADY_EXISTS = SD::BEG_USER_ERROR,
            /** SD Error  1 */INVALID_FILE_MODE,
            /** SD Error  2 */ENTRY_NOT_FILE,
            /** SD Error  3 */ENTRY_NOT_DIR,
            /** SD Error  4 */FILENAME_NOT_FOUND,
            /** End user errors */END_USER_ERRORS = SD::FILENAME_NOT_FOUND,
            /** Begin system errors */BEG_SYS_ERROR,
            /** SD Error  5 */CORRUPT_CLUSTER = SD::BEG_SYS_ERROR,
            /** SD Error  6 */INVALID_FILENAME,
            /** SD Error  7 */INVALID_CMD,
            /** SD Error  8 */READ_TIMEOUT,
            /** SD Error  9 */INVALID_NUM_BYTES,
            /** SD Error 10 */INVALID_RESPONSE,
            /** SD Error 11 */INVALID_INIT,
            /** SD Error 12 */INVALID_DAT_STRT_ID,
            /** SD Error 13 */EMPTY_FAT_ENTRY,
            /** SD Error 14 */INVALID_PTR_ORIGIN,
            /** SD Error 15 */INVALID_FAT_APPEND,
            /** SD Error 16 */TOO_MANY_FATS,
            /** SD Error 17 */READING_PAST_EOC,
            /** SD Error 18 */FILE_WITHOUT_BUFFER,
            /** SD Error 19 */INVALID_FILESYSTEM,
            /** SD Error 20 */CMD8_FAILURE,
            /** End system errors */END_SYS_ERROR = SD::CMD8_FAILURE,
            /** Last SD error code */END_ERROR = SD::END_SYS_ERROR
        } ErrorCode;

        /**
         * Buffer object used for storing SD data; Each instance uses 527 bytes (526
         * if SD_OPTION_FILE_WRITE is disabled)
         */
        struct Buffer {
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

        /**
         * SD file object
         *
         * @note    Must be initialized with an sd_buffer object before use; If one has
         *          not been explicitly created then the global buffer, m_buf, can be
         *          used at the expense of decreased performance
         */
        struct File {
                SD::Buffer *buf;
                /** determine if the buffer is owned by this file */
                uint8_t id;  //
                uint32_t wPtr;
                uint32_t rPtr;
                SD::FileMode mode;
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

    public:
        /**
         * @brief       Construct an SD object; Set two simple member variables
         */
        SD (SPI *spi);

        /**
         * @brief   Give access to the internal buffer in case the user wants to
         *          save on system memory
         *
         * @return  Returns the system SD::Buffer
         */
        SD::Buffer* getGlobalBuffer ();

        /**
         * @brief       Initialize SD card communication over SPI for 3.3V configuration
         *
         * @detailed    Starts an SPI cog IFF an SPI cog has not already been started;
         *              If one has been started, only the cs and freq parameter will
         *              have effect
         *
         * @param[in]   mosi        Pin mask for MOSI pin
         * @param[in]   miso        Pin mask for MISO pin
         * @param[in]   sclk        Pin mask for SCLK pin
         * @param[in]   cs          Pin mask for CS pin
         * @param[in]   freq        Frequency to run the clock after initialization; if
         *                          -1 or 0 is passed in, a system default will be used
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode start (const PropWare::GPIO::Pin mosi,
                const PropWare::GPIO::Pin miso, const PropWare::GPIO::Pin sclk,
                const PropWare::GPIO::Pin cs, const int32_t freq);

        /**
         * @brief   Mount either FAT16 or FAT32 file system
         *
         * @return  Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode mount ();

#ifdef SD_OPTION_FILE_WRITE
        /**
         * @brief   Stop all SD activities and write any modified buffers
         *
         * @pre     All files must be explicitly closed before
         *
         * @return  Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode unmount ();
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
        PropWare::ErrorCode chdir (const char *d);

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
        PropWare::ErrorCode fopen (const char *name, SD::File *f, const SD::FileMode mode);

#ifdef SD_OPTION_FILE_WRITE
        /**
         * @brief       Close a given file
         *
         * @param[in]   *f  Address of the file object to close
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode fclose (SD::File *f);

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
        PropWare::ErrorCode fputc (const char c, SD::File *f);

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
        PropWare::ErrorCode fputs (char *s, SD::File *f);
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
         * @return      Returns the character pointed to by the m_rPtr pointer
         */
        char fgetc (SD::File *f);

        /**
         * @brief       Read a line from a file until either 'size' characters have been
         *              read or a newline is found; Parameters should match tinyio.h's
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
        char * fgets (char s[], uint32_t size, SD::File *f);

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
        inline bool feof (SD::File *f);

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
        PropWare::ErrorCode fseekr (SD::File *f, const int32_t offset,
                const SD::FilePos origin);

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
        PropWare::ErrorCode fseekw (SD::File *f, const int32_t offset,
                const SD::FilePos origin);

        /**
         * @brief       Retrieve the current position of the read pointer
         *
         * @pre         *f must be an opened file
         *
         * @param[in]   *f  Address of the file object being referenced
         *
         * @return      Returns the byte offset (from beginning) of the read pointer
         */
        int32_t ftellr (const SD::File *f);

        /**
         * @brief       Retrieve the current position of the write pointer
         *
         * @pre         *f must be an opened file
         *
         * @param[in]   *f  Address of the file object being referenced
         *
         * @return      Returns the byte offset (from beginning) of the write pointer
         */
        int32_t ftellw (const SD::File *f);

#ifdef SD_OPTION_SHELL
        /**
         * @brief       Provide the user with a very basic Unix-like shell. The
         *              following commands are available to the user: ls, cat, cd.
         *
         * @param[in]   *f  If a file is opened via a command such as 'cat', its
         *                  information will be stored at this address
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode shell (SD::File *f);

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
        PropWare::ErrorCode shell_ls ();

        /**
         * @brief       Dump the contents of a file to the screen (similar to 'cat f');
         *
         * @note        Does not currently follow paths
         *
         * @param[in]   *f  Short filename of file to print
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode shell_cat (const char *name, SD::File *f);

        /**
         * @brief       Change the current working directory to *d (similar to 'cd dir');
         *
         * @param[in]   *d  Short filename of directory to change to
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode shell_cd (const char *d);

#ifdef SD_OPTION_FILE_WRITE
        /**
         * @brief       Create a new file, do not open it
         *
         * @param[in]   name[]  C-string name for the file to be created
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode shell_touch (const char name[]);
#endif
#endif

#if (defined SD_OPTION_VERBOSE || defined SD_OPTION_VERBOSE_BLOCKS)
        /**
         * @brief       Print a block of data in hex format to the screen in
         *              SD_LINE_SIZE-byte lines
         *
         * @param[in]   *dat       Pointer to the beginning of the data
         * @param[in]   bytes      Number of bytes to print
         */
        void print_hex_block (uint8_t *dat, uint16_t bytes);
#endif

        /* @brief   Create a human-readable error string
         *
         * @param[in]   err         Error number used to determine error string
         * @param[out]  errorStr    Allocated space where a string of no more
         *                          than 128 characters can be printed
         */
        void print_error_str (const SD::ErrorCode err) const;

    private:
        typedef struct {
            uint8_t numFATs;
            uint32_t rsvdSectorCount;
            uint32_t rootEntryCount;
            uint32_t totalSectors;
            uint32_t FATSize;
            uint32_t dataSectors;
            uint32_t bootSector;
            uint32_t clusterCount;
        } InitFATInfo;

    private:
        /***********************
         *** Private Methods ***
         ***********************/
        inline PropWare::ErrorCode reset_and_verify_v2_0 (uint8_t response_param[]);

        inline PropWare::ErrorCode power_up ();

        inline PropWare::ErrorCode reset (uint8_t response[], bool *isIdle);

        inline PropWare::ErrorCode verify_v2_0 (uint8_t response[], bool *stageCleared);

        inline PropWare::ErrorCode send_active (uint8_t response[]);

        inline PropWare::ErrorCode increase_throttle (const int32_t freq);

        inline PropWare::ErrorCode read_boot_sector (InitFATInfo *fatInfo);

        inline PropWare::ErrorCode common_boot_sector_parser(InitFATInfo *fatInfo);

        inline void partition_info_parser (InitFATInfo *fatInfo);

        inline PropWare::ErrorCode determine_fat_type (InitFATInfo *fatInfo);

        inline void store_root_info (InitFATInfo *fatInfo);

        inline PropWare::ErrorCode read_fat_and_root_sectors ();

#if (defined SD_OPTION_VERBOSE)
        PropWare::ErrorCode print_init_debug_blocks (uint8_t response[]);
#endif

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
        PropWare::ErrorCode send_command (const uint8_t cmd, const uint32_t arg,
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
        PropWare::ErrorCode get_response (const uint8_t numBytes, uint8_t *dat);

        /**
         * @brief       Receive data from SD card via SPI
         *
         * @param[in]   bytes   Number of bytes to receive
         * @param[out]  *data   Location in memory with enough space to store `bytes`
         *                      bytes of data
         *
         * @return      Returns 0 for success, else error code
         */
        PropWare::ErrorCode read_block (uint16_t bytes, uint8_t *dat);

        /**
         * @brief       Write data to SD card via SPI
         *
         * @param[in]   bytes   Block address to read from SD card
         * @param[in]   *dat    Location in memory where data resides
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode write_block (uint16_t bytes, uint8_t *dat);

        /**
         * @brief       Read SD_SECTOR_SIZE-byte data block from SD card
         *
         * @param[in]   address    Number of bytes to send
         * @param[out]  *dat       Location in chip memory to store data block
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode read_data_block (uint32_t address, uint8_t *dat);

        /**
         * @brief       Write SD_SECTOR_SIZE-byte data block to SD card
         *
         * @param[in]   address     Block address to write to SD card
         * @param[in]   *dat        Location in chip memory to read data block
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode write_data_block (uint32_t address, uint8_t *dat);

        /**
         * @brief       Return byte-reversed 16-bit variable (SD cards store bytes
         *              little-endian therefore we must reverse them to use multi-byte
         *              variables)
         *
         * @param[in]   buf[]   Address of first byte of data
         *
         * @return      Returns a normal (big-endian) 16-bit word
         */
        uint16_t read_rev_dat16 (const uint8_t buf[]);

        /**
         * @brief       Return byte-reversed 32-bit variable (SD cards store bytes
         *              little-endian therefore we must reverse them to use multi-byte
         *              variables)
         *
         * @param[in]   buf[]   Address of first byte of data
         *
         * @return      Returns a normal (big-endian) 32-bit word
         */
        uint32_t read_rev_dat32 (const uint8_t buf[]);

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
        void write_rev_dat16 (uint8_t buf[], const uint16_t dat);

        /**
         * @brief       Write a byte-reversed 32-bit variable (SD cards store bytes
         *              little-endian therefore we must reverse them to use multi-byte
         *              variables)
         *
         * @param[out]  buf[]   Address to store the first byte of data
         * @param[in]   dat     Normal, 32-bit variable to be written to RAM in reverse
         *                      endian
         */
        void write_rev_dat32 (uint8_t buf[], const uint32_t dat);
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
        uint32_t find_sector_from_path (const char *path);

        /**
         * @brief       Find and return the starting sector's address for a given
         *              allocation unit (note - not cluster)
         *
         * @param[in]   allocUnit   Allocation unit in FAT filesystem
         *
         * @return      Returns sector address of the desired allocation unit
         */
        uint32_t find_sector_from_alloc (uint32_t allocUnit);

        /**
         * @brief       Read an entry from the FAT
         *
         * @param[in]   fatEntry    Entry number (allocation unit) to read in the FAT
         * @param[out]  *value      Address to store the value into (the next allocation
         *                          unit)
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode get_fat_value (const uint32_t fatEntry, uint32_t *value);

        /**
         * @brief       Find the next sector in the FAT, directory, or file. When it is
         *              found, load it into the appropriate global buffer
         *
         * @param[out]  *buf    Array of `SD_SECTOR_SIZE` bytes that can be filled with
         *                      the requested sector
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode load_next_sector (SD::Buffer *buf);

        /**
         * @brief       Load a requested sector into the buffer independent of the
         *              current sector or cluster
         *
         * @param[out]  *f      Address of the sd_buffer object to be updated
         * @param[in]   offset  How many sectors past the first one should be skipped
         *                      (sector number of the file)
         *
         * @return      Returns 0 upon success, error code otherwise
         *
         */
        PropWare::ErrorCode load_sector_from_offset (SD::File *f, const uint32_t offset);

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
        PropWare::ErrorCode inc_cluster (SD::Buffer *buf);

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
        void get_filename (const uint8_t *buf, char *filename);

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
        PropWare::ErrorCode find (const char *filename, uint16_t *fileEntryOffset);

        /**
         * @brief       Reload the sector currently in use by a given file
         *
         * @param[in]   *f  Address of the file object requested
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode reload_buf (SD::File *f);

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
         *                          restored to m_fat before returning; if zero, the
         *                          last-used sector will remain loaded
         *
         * @return      Returns the number of the first unused allocation unit
         */
        uint32_t find_empty_space (const uint8_t restore);

        /**
         * @brief       Enlarge a file or directory by one cluster
         *
         * @param[in]   *buf    Address of the buffer (containing information for a
         *                      file or directory) to be enlarged
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode extend_fat (SD::Buffer *buf);

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
        PropWare::ErrorCode create_file (const char *name, const uint16_t *fileEntryOffset);
#endif

#if (defined SD_OPTION_SHELL || defined SD_OPTION_VERBOSE)
        /**
         * @brief       Print the attributes and name of a file entry
         *
         * @param[in]   *fileEntry  Address of the first byte of the file entry
         * @param[out]  *filename   Allocated space for the filename string to be stored
         */
        void print_file_entry (const uint8_t *fileEntry, char filename[]);

        /**
         * @brief       Print attributes of a file entry
         *
         * @param[in]
         */
        void print_file_attributes (const uint8_t flags);

        /**
         * @brief   Print to screen each status bit individually with
         *          human-readable descriptions
         */
        void first_byte_expansion () const;
#endif

    private:
        /*************************
         *** Private Constants ***
         *************************/
        // SPI config
        static const uint32_t SPI_INIT_FREQ = 200000;  // Run SD initialization at 200 kHz
        static const SPI::Mode SPI_MODE = SPI::MODE_0;
        static const SPI::BitMode SPI_BITMODE = SPI::MSB_FIRST;

        // Misc. SD Definitions
        static const uint32_t RESPONSE_TIMEOUT;  // Wait 0.1 seconds for a response before timing out
        static const uint32_t WIGGLE_ROOM;
        static const uint8_t SECTOR_SIZE_SHIFT = 9;

        // SD Commands
        static const uint8_t CMD_IDLE = 0x40 + 0;  // Send card into idle state
        static const uint8_t CMD_INTERFACE_COND = 0x40 + 8;  // Send interface condition and host voltage range
        static const uint8_t CMD_RD_CSD = 0x40 + 9;  // Request "Card Specific Data" block contents
        static const uint8_t CMD_RD_CID = 0x40 + 10;  // Request "Card Identification" block contents
        static const uint8_t CMD_RD_BLOCK = 0x40 + 17;  // Request data block
        static const uint8_t CMD_WR_BLOCK = 0x40 + 24;  // Write data block
        static const uint8_t CMD_WR_OP = 0x40 + 41;  // Send operating conditions for SDC
        static const uint8_t CMD_APP = 0x40 + 55;  // Inform card that following instruction is application specific
        static const uint8_t CMD_READ_OCR = 0x40 + 58;  // Request "Operating Conditions Register" contents

        // SD Arguments
        static const uint32_t HOST_VOLTAGE_3V3 = 0x01;
        static const uint32_t R7_CHECK_PATTERN = 0xAA;
        static const uint32_t ARG_CMD8 = ((SD::HOST_VOLTAGE_3V3 << 8)
                | SD::R7_CHECK_PATTERN);
        static const uint32_t ARG_LEN = 5;

        // SD CRCs
        static const uint8_t CRC_IDLE = 0x95;
        static const uint8_t CRC_CMD8 = 0x87;  // CRC only valid for CMD8 argument of 0x000001AA
        static const uint8_t CRC_ACMD_PREP = 0x65;
        static const uint8_t CRC_ACMD = 0x77;
        static const uint8_t CRC_OTHER = 0x01;

        // SD Responses
        static const uint8_t RESPONSE_IDLE = 0x01;
        static const uint8_t RESPONSE_ACTIVE = 0x00;
        static const uint8_t DATA_START_ID = 0xFE;
        static const uint8_t RESPONSE_LEN_R1 = 1;
        static const uint8_t RESPONSE_LEN_R3 = 5;
        static const uint8_t RESPONSE_LEN_R7 = 5;
        static const uint8_t RSPNS_TKN_BITS = 0x0f;
        static const uint8_t RSPNS_TKN_ACCPT = (0x02 << 1) | 1;
        static const uint8_t RSPNS_TKN_CRC = (0x05 << 1) | 1;
        static const uint8_t RSPNS_TKN_WR = (0x06 << 1) | 1;

        // Boot sector addresses/values
        static const uint8_t FAT_16 = 2;  // A FAT entry in FAT16 is 2-bytes
        static const uint8_t FAT_32 = -4;  // A FAT entry in FAT32 is 4-bytes
        static const uint8_t BOOT_SECTOR_ID = 0xEB;
        static const uint8_t BOOT_SECTOR_ID_ADDR = 0;
        static const uint16_t BOOT_SECTOR_BACKUP = 0x1C6;
        static const uint8_t CLUSTER_SIZE_ADDR = 0x0D;
        static const uint8_t RSVD_SCTR_CNT_ADDR = 0x0E;
        static const uint8_t NUM_FATS_ADDR = 0x10;
        static const uint8_t ROOT_ENTRY_CNT_ADDR = 0x11;
        static const uint8_t TOT_SCTR_16_ADDR = 0x13;
        static const uint8_t FAT_SIZE_16_ADDR = 0x16;
        static const uint8_t TOT_SCTR_32_ADDR = 0x20;
        static const uint8_t FAT_SIZE_32_ADDR = 0x24;
        static const uint8_t ROOT_CLUSTER_ADDR = 0x2c;
        static const uint16_t FAT12_CLSTR_CNT = 4085;
        static const uint16_t FAT16_CLSTR_CNT = 65525;

        // FAT file/directory values
        static const uint8_t FILE_ENTRY_LENGTH = 32;  // An entry in a directory uses 32 bytes
        static const uint8_t DELETED_FILE_MARK = 0xE5;  // Marks that a file has been deleted here, continue to the next entry
#define SD_FILE_NAME_LEN        8
        static const uint8_t FILE_NAME_LEN = SD_FILE_NAME_LEN;  // 8 characters in the standard file name
#define SD_FILE_EXTENSION_LEN   3
        static const uint8_t FILE_EXTENSION_LEN = SD_FILE_EXTENSION_LEN;  // 3 character file name extension
#define SD_FILENAME_STR_LEN     (SD_FILE_NAME_LEN + SD_FILE_EXTENSION_LEN + 2)
        static const uint8_t FILENAME_STR_LEN = SD_FILENAME_STR_LEN;
        static const uint8_t FILE_ATTRIBUTE_OFFSET = 0x0B;  // Byte of a file entry to store attribute flags
        static const uint8_t FILE_START_CLSTR_LOW = 0x1A;  // Starting cluster number
        static const uint8_t FILE_START_CLSTR_HIGH = 0x14;  // High word (16-bits) of the starting cluster number (FAT32 only)
        static const uint8_t FILE_LEN_OFFSET = 0x1C;  // Length of a file in bytes
        static const int8_t FREE_CLUSTER = 0;  // Cluster is unused
        static const int8_t RESERVED_CLUSTER = 1;
        static const int8_t RSVD_CLSTR_VAL_BEG = -15;  // First reserved cluster value
        static const int8_t RSVD_CLSTR_VAL_END = -9;  // Last reserved cluster value
        static const int8_t BAD_CLUSTER = -8;  // Cluster is corrupt
        static const int32_t EOC_BEG = -7;  // First marker for end-of-chain (end of file entry within FAT)
        static const int32_t EOC_END = -1;  // Last marker for end-of-chain

        // FAT file attributes (definitions with trailing underscore represent character for a cleared attribute flag)
        static const uint8_t READ_ONLY = BIT_0;
        static const char READ_ONLY_CHAR = 'r';
        static const char READ_ONLY_CHAR_ = 'w';
        static const uint8_t HIDDEN_FILE = BIT_1;
        static const char HIDDEN_FILE_CHAR = 'h';
        static const char HIDDEN_FILE_CHAR_ = '.';
        static const uint8_t SYSTEM_FILE = BIT_2;
        static const char SYSTEM_FILE_CHAR = 's';
        static const char SYSTEM_FILE_CHAR_ = '.';
        static const uint8_t VOLUME_ID = BIT_3;
        static const char VOLUME_ID_CHAR = 'v';
        static const char VOLUME_ID_CHAR_ = '.';
        static const uint8_t SUB_DIR = BIT_4;
        static const char SUB_DIR_CHAR = 'd';
        static const char SUB_DIR_CHAR_ = 'f';
        static const uint8_t ARCHIVE = BIT_5;
        static const char ARCHIVE_CHAR = 'a';
        static const char ARCHIVE_CHAR_ = '.';

        /*******************************
         *** Private Member Variable ***
         *******************************/
    private:
        /*** Global variable declarations ***/
        // Initialization variables
        SPI *m_spi;
        PropWare::GPIO::Pin m_cs;  // Chip select pin mask
        uint8_t m_filesystem;  // File system type - one of SD::FAT_16 or SD::FAT_32
        uint8_t m_sectorsPerCluster_shift;  // Used as a quick multiply/divide; Stores log_2(Sectors per Cluster)
        uint32_t m_rootDirSectors;  // Number of sectors for the root directory
        uint32_t m_fatStart;  // Starting block address of the FAT
        uint32_t m_rootAddr;  // Starting block address of the root directory
        uint32_t m_rootAllocUnit;  // Allocation unit of root directory/first data sector (FAT32 only)
        uint32_t m_firstDataAddr;  // Starting block address of the first data cluster

        // FAT file system variables
        SD::Buffer m_buf;
        uint8_t m_fat[SD_SECTOR_SIZE];        // Buffer for FAT entries only
#ifdef SD_OPTION_FILE_WRITE
        uint8_t m_fatMod;  // Has the currently loaded FAT sector been modified
        uint32_t m_fatSize;
#endif
        uint16_t m_entriesPerFatSector_Shift;  // How many FAT entries are in a single sector of the FAT
        uint32_t m_curFatSector;  // Store the current FAT sector loaded into m_fat

        uint32_t m_dir_firstAllocUnit;  // Store the current directory's starting allocation unit

        // Assigned to a file and then to each buffer that it touches - overwritten by
        // other functions and used as a check by the file to determine if the buffer
        // needs to be reloaded with its sector
        uint8_t m_fileID;

        // First byte response receives special treatment to allow for proper debugging
        uint8_t m_firstByteResponse;
};

}

#endif /* SD_H_ */
