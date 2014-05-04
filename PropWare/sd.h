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

#ifndef PROPWARE_SD_H_
#define PROPWARE_SD_H_

#include <stdlib.h>
#include <string.h>
#include <PropWare/PropWare.h>
#include <PropWare/spi.h>
#include <PropWare/pin.h>

// Rather than include all of stdio.h, only these definitions will be declared
#if (defined USE_PRINTF)
#if (!(defined __TINY_IO_H || _STDIO_H))
extern char * gets(char *s);
#endif
#else
#include <simpletext.h>
#define printf print
#define putchar putChar
#endif

#ifndef EOF
#define EOF (-1)
#endif

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
         * Maximum number of characters for an individual command (does not
         * include parameters
         */
#define SD_SHELL_CMD_LEN    8
        static const uint8_t SHELL_CMD_LEN = SD_SHELL_CMD_LEN;
        /** Maximum number of characters for each command argument */
#define SD_SHELL_ARG_LEN    64
        static const uint8_t SHELL_ARG_LEN = SD_SHELL_ARG_LEN;
        /** String defining the "exit" command to quit the SD_Shell() function*/
        static const char SHELL_EXIT[];
        /**
         * String defining the "ls" command to call SD_Shell_ls(); List dir
         * contents
         */
        static const char SHELL_LS[];
        /**
         * String defining the "cat" command to call SD_Shell_cat(); Prints a
         * file
         */
        static const char SHELL_CAT[];
        /**
         * String defining the "cd" command to call SD_Shell_cd(); Change
         * directory
         */
        static const char SHELL_CD[];
        /** String defining the "touch" command; Creates an empty file */
        static const char SHELL_TOUCH[];
        /**@}*/
#endif

    public:
        /**
         * File modes
         *
         * TODO: Learn what these modes *should* do and do it; At the moment,
         *       these modes essentially aren't used for anything
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
             * Append+ (read + write); Write pointer starts at last character
             * + 1, read pointer starts at first character
             */
            FILE_MODE_A_PLUS,
#endif
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
            /** No error */NO_ERROR = 0,
            /** First SD error code */BEG_ERROR = SPI::END_ERROR + 1,
            /** Begin user errors */BEG_USER_ERROR = PropWare::SD::BEG_ERROR,
            /** SD Error  0 */FILE_ALREADY_EXISTS = PropWare::SD::BEG_USER_ERROR,
            /** SD Error  1 */INVALID_FILE_MODE,
            /** SD Error  2 */ENTRY_NOT_FILE,
            /** SD Error  3 */ENTRY_NOT_DIR,
            /** SD Error  4 */FILENAME_NOT_FOUND,
            /** End user errors */END_USER_ERRORS = PropWare::SD::FILENAME_NOT_FOUND,
            /** Begin system errors */BEG_SYS_ERROR,
            /** SD Error  5 */CORRUPT_CLUSTER = PropWare::SD::BEG_SYS_ERROR,
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
            /** End system errors */END_SYS_ERROR = PropWare::SD::CMD8_FAILURE,
            /** Last SD error code */END_ERROR = PropWare::SD::END_SYS_ERROR
        } ErrorCode;

        /**
         * Buffer object used for storing SD data; Each instance uses 527 bytes
         * (526 if SD_OPTION_FILE_WRITE is disabled)
         */
        class Buffer {
            public:
                /**  Buffer for SD card contents */
                uint8_t buf[SD_SECTOR_SIZE];
                /** Buffer ID - determine who owns the current information */
                uint8_t id;
                /** Store the current cluster's starting sector number */
                uint32_t curClusterStartAddr;
                /**
                 * Store the current sector offset from the beginning of the
                 * cluster
                 */
                uint8_t curSectorOffset;
                /** Store the current allocation unit */
                uint32_t curAllocUnit;
                /** Look-ahead at the next FAT entry */
                uint32_t nextAllocUnit;
#ifdef SD_OPTION_FILE_WRITE
                /**
                 * When set, the currently loaded sector has been modified since
                 * it was read from the SD card
                 */
                bool mod;
#endif
        };

        /**
         * SD file object
         *
         * @note    Must be initialized with an sd_buffer object before use; If
         *          one has not been explicitly created then the global buffer,
         *          m_buf, can be used at the expense of decreased performance
         */
        class File {
            public:
                PropWare::SD::Buffer *buf;
                /** determine if the buffer is owned by this file */
                uint8_t id;  //
                uint32_t wPtr;
                uint32_t rPtr;
                PropWare::SD::FileMode mode;
                uint32_t length;
                /** Maximum number of sectors currently allocated to a file */
                uint32_t maxSectors;
                /**
                 * When the length of a file is changed, this variable will be
                 * set, otherwise cleared
                 */
                uint8_t mod;
                /** File's starting allocation unit */
                uint32_t firstAllocUnit;
                /**
                 * like curSectorOffset, but does not reset upon loading a new
                 * cluster
                 */
                uint32_t curSector;
                /** like curSector, but for allocation units */
                uint32_t curCluster;
                /**
                 * Which sector of the SD card contains this file's meta-data
                 */
                uint32_t dirSectorAddr;
                /** Address within the sector of this file's entry */
                uint16_t fileEntryOffset;
        };

    public:
        /**
         * @brief       Construct an SD object; Set two simple member variables
         */
        SD (SPI *spi) {
            this->m_spi = spi;
            this->m_fileID = 0;
#ifdef SD_OPTION_FILE_WRITE
            this->m_fatMod = false;
#endif
        }

        /**
         * @brief   Give access to the internal buffer in case the user wants to
         *          save on system memory
         *
         * @return  Returns the system SD::Buffer
         */
        SD::Buffer* get_global_buffer () {
            return &(this->m_buf);
        }

        /**
         * @brief       Initialize SD card communication over SPI for 3.3V
         *              configuration
         *
         * @detailed    Starts an SPI cog IFF an SPI cog has not already been
         *              started; If one has been started, only the cs and freq
         *              parameter will have effect
         *
         * @param[in]   mosi        PinNum mask for MOSI pin
         * @param[in]   miso        PinNum mask for MISO pin
         * @param[in]   sclk        PinNum mask for SCLK pin
         * @param[in]   cs          PinNum mask for CS pin
         * @param[in]   freq        Frequency to run the clock after
         *                          initialization; if -1 or 0 is passed in, a
         *                          system default will be used
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode start (const PropWare::Port::Mask mosi,
                const PropWare::Port::Mask miso,
                const PropWare::Port::Mask sclk, const PropWare::Port::Mask cs,
                const int32_t freq) {
            PropWare::ErrorCode err;
            uint8_t response[16];

            // Set CS for output and initialize high
            this->m_cs.set_mask(cs);
            this->m_cs.set_dir(PropWare::Pin::OUT);
            this->m_cs.set();

            // Start SPI module
            if ((err = this->m_spi->start(mosi, miso, sclk, SD::SPI_INIT_FREQ,
                    SD::SPI_MODE, SD::SPI_BITMODE)))
                return err;

            // Try and get the card up and responding to commands first
            check_errors(this->reset_and_verify_v2_0(response));

            check_errors(this->send_active(response));

            check_errors(this->increase_throttle(freq));

#ifdef SD_OPTION_VERBOSE
            check_errors(this->print_init_debug_blocks(response));
#endif

            // We're finally done initializing everything. Set chip select high
            // again to release the SPI port
            this->m_cs.set();

            // Initialization complete
            return 0;
        }

        /**
         * @brief   Mount either FAT16 or FAT32 file system
         *
         * @return  Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode mount () {
            PropWare::ErrorCode err;

            // FAT system determination variables:
            SD::InitFATInfo fatInfo;
            fatInfo.bootSector = 0;

            check_errors(this->read_boot_sector(&fatInfo));

            check_errors(this->common_boot_sector_parser(&fatInfo));

            this->partition_info_parser(&fatInfo);

            check_errors(this->determine_fat_type(&fatInfo));

            this->store_root_info(&fatInfo);

            check_errors(this->read_fat_and_root_sectors());

            return 0;
        }

#ifdef SD_OPTION_FILE_WRITE
        /**
         * @brief   Stop all SD activities and write any modified buffers
         *
         * @pre     All files must be explicitly closed before
         *
         * @return  Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode unmount () {
            PropWare::ErrorCode err;

            // If the directory buffer was modified, write it
            if (this->m_buf.mod)
                check_errors(
                        this->write_data_block(
                                this->m_buf.curClusterStartAddr
                                        + this->m_buf.curSectorOffset,
                                this->m_buf.buf));

            // If the FAT sector was modified, write it
            if (this->m_fatMod) {
                check_errors(
                        this->write_data_block(
                                this->m_curFatSector + this->m_fatStart,
                                this->m_fat));
                check_errors(
                        this->write_data_block(
                                this->m_curFatSector + this->m_fatStart
                                        + this->m_fatSize, this->m_fat));
            }

            return 0;
        }
#endif

        /**
         * @brief       Change the current working directory to *d (similar to
         *              'cd dir')
         *
         * @detailed    At the moment, the target directory must be an immediate
         *              child of the current directory ("." and ".." are
         *              allowed). I hope to implement the ability to change to
         *              any directory soon (such as "cd ../siblingDirectory")
         *              but attempting to do this now would currently result in
         *              an SD_FILENAME_NOT_FOUND error
         *
         * @param[in]   *d     Short filename of directory to change to
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode chdir (const char *d) {
            PropWare::ErrorCode err;
            uint16_t fileEntryOffset = 0;

            this->m_buf.id = SD::FOLDER_ID;

            // Attempt to find the file and return an error code if not found
            check_errors(this->find(d, &fileEntryOffset));

            // If the returned entry isn't a file, throw an error
            if (!(SD::SUB_DIR
                    & this->m_buf.buf[fileEntryOffset
                            + SD::FILE_ATTRIBUTE_OFFSET]))
                return SD::ENTRY_NOT_DIR;

#ifdef SD_OPTION_FILE_WRITE
            // If the previous sector was modified, write it back to the SD card
            // before reading
            if (this->m_buf.mod)
                this->write_data_block(
                        this->m_buf.curClusterStartAddr
                                + this->m_buf.curSectorOffset, this->m_buf.buf);
            this->m_buf.mod = false;
#endif

#ifdef SD_OPTION_VERBOSE
            printf("%s found at offset 0x%04x from address 0x%08x\n", d,
                    fileEntryOffset,
                    this->m_buf.curClusterStartAddr +
                    this->m_buf.curSectorOffset);
#endif

            // File entry was found successfully, load it into the buffer and
            // update status variables
            if (SD::FAT_16 == this->m_filesystem)
                this->m_buf.curAllocUnit = this->read_rev_dat16(
                        &(this->m_buf.buf[fileEntryOffset
                                + SD::FILE_START_CLSTR_LOW]));
            else {
                this->m_buf.curAllocUnit = this->read_rev_dat16(
                        &(this->m_buf.buf[fileEntryOffset
                                + SD::FILE_START_CLSTR_LOW]));
                this->m_buf.curAllocUnit |= this->read_rev_dat16(
                        &(this->m_buf.buf[fileEntryOffset
                                + SD::FILE_START_CLSTR_HIGH])) << 16;
                // Clear the highest 4 bits - they are always reserved
                this->m_buf.curAllocUnit &= 0x0FFFFFFF;
            }
            this->get_fat_value(this->m_buf.curAllocUnit,
                    &(this->m_buf.nextAllocUnit));
            if (0 == this->m_buf.curAllocUnit) {
                this->m_buf.curAllocUnit = -1;
                this->m_dir_firstAllocUnit = this->m_rootAllocUnit;
            } else
                this->m_dir_firstAllocUnit = this->m_buf.curAllocUnit;
            this->m_buf.curSectorOffset = 0;
            this->read_data_block(this->m_buf.curClusterStartAddr,
                    this->m_buf.buf);

#ifdef SD_OPTION_VERBOSE
            printf("Opening directory from...\n");
            printf("\tAllocation unit 0x%08x\n", this->m_buf.curAllocUnit);
            printf("\tCluster starting address 0x%08x\n",
                    this->m_buf.curClusterStartAddr);
            printf("\tSector offset 0x%04x\n", this->m_buf.curSectorOffset);
#ifdef SD_OPTION_VERBOSE_BLOCKS
            printf("And the first directory sector looks like....\n");
            this->print_hex_block(this->m_buf.buf, SD::SECTOR_SIZE);
            putchar('\n');
#endif
#endif
            return 0;
        }

        /**
         * @brief       Open a file with a given name and load its information
         *              into the file pointer
         *
         * @detailed    Load the first sector of a file into the file buffer;
         *              Initialize global character pointers
         *
         * @note        Currently, only one file mode is supported and is best
         *              described as "r+"
         * @note        Two position pointers are used, one for writing and one
         *              for reading, this may be changed later to comply with
         *              POSIX standards but is useful for my own purposes at the
         *              moment
         * @note        This driver does not include any provision for
         *              timestamps; Neither file modification or creation will
         *              change a file's timestamp data (creation times are
         *              random and uninitialized)
         *
         * @pre         Files cannot be created in the root directory of a FAT16
         *              filesystem
         *              TODO: Fix this
         *
         * @param[in]   *name   C-string containing the filename to open
         * @param[in]   *f      Address where file information (such as the
         *                      first allocation unit) can be stored. Opening
         *                      multiple files simultaneously is allowed.
         *
         * @return      Returns 0 upon success, error code otherwise; Look for
         *              PropWare::SD::
         */
        PropWare::ErrorCode fopen (const char *name, SD::File *f,
                const SD::FileMode mode) {
            PropWare::ErrorCode err;
            uint16_t fileEntryOffset = 0;

#ifdef SD_OPTION_VERBOSE
            printf("Attempting to open %s\n", name);
#endif

            if (NULL == f->buf)
                return SD::FILE_WITHOUT_BUFFER;

            f->id = this->m_fileID++;
            f->rPtr = 0;
            f->wPtr = 0;
#ifndef SD_OPTION_FILE_WRITE
            if (FILE_MODE_R != mode)
            return SD::INVALID_FILE_MODE;
#endif
            f->mode = mode;
            f->mod = false;

            // Attempt to find the file
            if ((err = this->find(name, &fileEntryOffset))) {
#ifdef SD_OPTION_FILE_WRITE
                // Find returned an error; ensure it was EOC...
                if (SD::EOC_END == err) {
                    // And return a FILE_NOT_FOUND error if using read only mode
                    if (SD::FILE_MODE_R == mode)
                        return PropWare::SD::FILENAME_NOT_FOUND;
                    // Or create the file for any other mode
                    else {
                        // File wasn't found and the cluster is full; add another
                        // to the directory
#ifdef SD_OPTION_VERBOSE
                        printf("Directory cluster was full, adding another...\n");
#endif
                        check_errors(this->extend_fat(&this->m_buf));
                        check_errors(this->load_next_sector(&this->m_buf));
                    }
                }
                if (SD::EOC_END == err || SD::FILENAME_NOT_FOUND == err) {
                    // File wasn't found, but there is still room in this
                    // cluster (or a new cluster was just added)
#ifdef SD_OPTION_VERBOSE
                    printf("Creating a new directory entry...\n");
#endif
                    check_errors(this->create_file(name, &fileEntryOffset));
                } else
#endif
                    // SDFind returned unknown error - throw it
                    return err;
            }

            // `name` was found successfully, determine if it is a file or
            // directory
            if (SD::SUB_DIR
                    & this->m_buf.buf[fileEntryOffset
                            + SD::FILE_ATTRIBUTE_OFFSET])
                return SD::ENTRY_NOT_FILE;

            // Passed the file-not-directory test, load it into the buffer and
            // update status variables
            f->buf->id = f->id;
            f->curSector = 0;
            if (SD::FAT_16 == this->m_filesystem)
                f->buf->curAllocUnit = this->read_rev_dat16(
                        &(this->m_buf.buf[fileEntryOffset
                                + SD::FILE_START_CLSTR_LOW]));
            else {
                f->buf->curAllocUnit = this->read_rev_dat16(
                        &(this->m_buf.buf[fileEntryOffset
                                + SD::FILE_START_CLSTR_LOW]));
                f->buf->curAllocUnit |= this->read_rev_dat16(
                        &(this->m_buf.buf[fileEntryOffset
                                + SD::FILE_START_CLSTR_HIGH])) << 16;

                // Clear the highest 4 bits - they are always reserved
                f->buf->curAllocUnit &= 0x0FFFFFFF;
            }
            f->firstAllocUnit = f->buf->curAllocUnit;
            f->curCluster = 0;
            f->buf->curClusterStartAddr = this->find_sector_from_alloc(
                    f->buf->curAllocUnit);
            f->dirSectorAddr = this->m_buf.curClusterStartAddr
                    + this->m_buf.curSectorOffset;
            f->fileEntryOffset = fileEntryOffset;
            check_errors(
                    this->get_fat_value(f->buf->curAllocUnit,
                            &(f->buf->nextAllocUnit)));
            f->buf->curSectorOffset = 0;
            f->length = this->read_rev_dat32(
                    &(this->m_buf.buf[fileEntryOffset + SD::FILE_LEN_OFFSET]));
#ifdef SD_OPTION_FILE_WRITE
            // Determine the number of sectors currently allocated to this file;
            // useful in the case that the file needs to be extended
            f->maxSectors = f->length >> SD::SECTOR_SIZE_SHIFT;
            if (!(f->maxSectors))
                f->maxSectors = 1 << this->m_sectorsPerCluster_shift;
            while (f->maxSectors % (1 << this->m_sectorsPerCluster_shift))
                ++(f->maxSectors);
            f->buf->mod = false;
#endif
            check_errors(
                    this->read_data_block(f->buf->curClusterStartAddr,
                            f->buf->buf));

#ifdef SD_OPTION_VERBOSE
            printf("Opening file from...\n");
            printf("\tAllocation unit 0x%08x\n", f->buf->curAllocUnit);
            printf("\tNext allocation unit 0x%08x\n", f->buf->nextAllocUnit);
            printf("\tCluster starting address 0x%08x\n",
                    f->buf->curClusterStartAddr);
            printf("\tSector offset 0x%04x\n", f->buf->curSectorOffset);
            printf("\tFile length 0x%08x\n", f->length);
            printf("\tMax sectors 0x%08x\n", f->maxSectors);
#ifdef SD_OPTION_VERBOSE_BLOCKS
            printf("And the first file sector looks like....\n");
            this->print_hex_block(f->buf->buf, SD::SECTOR_SIZE);
            putchar('\n');
#endif
#endif

            return 0;
        }

#ifdef SD_OPTION_FILE_WRITE
        /**
         * @brief       Close a given file
         *
         * @param[in]   *f  Address of the file object to close
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode fclose (SD::File *f) {
            PropWare::ErrorCode err;

#ifdef SD_OPTION_VERBOSE
            printf("Closing file...\n");
#endif
            // If the currently loaded sector has been modified, save the
            // changes
            if ((f->buf->id == f->id) && f->buf->mod) {
                check_errors(
                        this->write_data_block(
                                f->buf->curClusterStartAddr
                                        + f->buf->curSectorOffset,
                                f->buf->buf));;
                f->buf->mod = false;
#ifdef SD_OPTION_VERBOSE
                printf("Modified sector in file has been saved...\n");
                printf("\tDestination address: 0x%08x / %u\n",
                        f->buf->curClusterStartAddr + f->buf->curSectorOffset,
                        f->buf->curClusterStartAddr + f->buf->curSectorOffset);
                printf("\tFile first sector address: 0x%08x / %u\n",
                        this->find_sector_from_alloc(f->firstAllocUnit),
                        this->find_sector_from_alloc(f->firstAllocUnit));
#endif
            }

            // If we modified the length of the file...
#ifdef SD_OPTION_VERBOSE
            printf("Closing file and \"f->mod\" value is %u\n", f->mod);
            printf("File length is: 0x%08x / %u\n", f->length, f->length);
#endif
            if (f->mod) {
#ifdef SD_OPTION_VERBOSE
                printf("File length has been modified - write it to the "
                        "directory\n");
#endif
                // Then check if the directory sector is still loaded...
                if ((this->m_buf.curClusterStartAddr
                        + this->m_buf.curSectorOffset) != f->dirSectorAddr) {
                    // If it isn't, load it...
                    if (this->m_buf.mod)
                        // And if it's been modified since the last read, save
                        // it...
                        check_errors(
                                this->write_data_block(
                                        this->m_buf.curClusterStartAddr
                                                + this->m_buf.curSectorOffset,
                                        this->m_buf.buf));
                    check_errors(
                            this->read_data_block(f->dirSectorAddr,
                                    this->m_buf.buf));
                }
                // Finally, edit the length of the file
                this->write_rev_dat32(
                        &(this->m_buf.buf[f->fileEntryOffset
                                + SD::FILE_LEN_OFFSET]), f->length);
                this->m_buf.mod = 01;
            }

            return 0;
        }

        /**
         * @brief       Insert a character into a given file
         *
         * @detailed    Insert 'c' at the location pointed to by the file's
         *              write pointer; Note: the read and write pointers may be
         *              merged into one at a later date
         *
         * @param[in]   c       Character to be inserted
         * @param[in]   *f      Address of the desired file object
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode fputc (const char c, SD::File *f) {
            PropWare::ErrorCode err;
            // Determines byte-offset within a sector
            uint16_t sectorPtr = f->wPtr % SD::SECTOR_SIZE;
            // Determine the needed file sector
            uint32_t sectorOffset = (f->wPtr >> SD::SECTOR_SIZE_SHIFT);

            // Determine if the correct sector is loaded
            if (f->buf->id != f->id)
                check_errors(this->reload_buf(f));

            // Even the the buffer was just reloaded, this snippet needs to be
            // called in order to extend the FAT if needed
            if (sectorOffset != f->curSector) {
                // Incorrect sector loaded
#ifdef SD_OPTION_VERBOSE
                printf("Need new sector:\n");
                printf("\tMax available sectors: 0x%08x / %u\n", f->maxSectors,
                        f->maxSectors);
                printf("\tDesired file sector: 0x%08x / %u\n", sectorOffset,
                        sectorOffset);
#endif

                // If the sector needed exceeds the available sectors, extend
                // the file
                if (f->maxSectors == sectorOffset) {
                    check_errors(this->extend_fat(f->buf));
                    f->maxSectors += 1 << this->m_sectorsPerCluster_shift;
                }

#ifdef SD_OPTION_VERBOSE
                printf("Loading new file sector at file-offset: 0x%08x / %u\n",
                        sectorOffset, sectorOffset);
#endif
                // SDLoadSectorFromOffset() will ensure that, if the current
                // buffer has been modified, it is written back to the SD card
                // before loading a new one
                check_errors(this->load_sector_from_offset(f, sectorOffset));
            }

            if (++(f->wPtr) > f->length) {
                ++(f->length);
                f->mod = true;
            }
            f->buf->buf[sectorPtr] = c;
            f->buf->mod = true;

            return 0;
        }

        /**
         * @brief       Insert a c-string into a file
         *
         * @detailed    Insert an array of bytes into the file object pointed to
         *              by 'f' beginning at address 's' until the value 0 is
         *              reached
         *
         * @param[in]   *s  C-string to be inserted
         * @param[in]   *f  Address of file object
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode fputs (char *s, SD::File *f) {
            PropWare::ErrorCode err;

            while (*s)
                check_errors(this->fputc(*(s++), f));

            return 0;
        }
#endif

        /**
         * @brief       Read one character from the currently opened file.
         *
         * @detailed    NOTE: This function does not include error checking
         *
         * @pre         *f must point to a currently opened and valid file
         * @pre         The file must have at least one byte left - no error
         *              checking is performed to stop the user from reading past
         *              the end of a file (call SDfeof() for end-of-file check)
         *
         * @param[in]   *f  Address where file information (such as the first
         *                  allocation unit) can be stored.
         *
         * @return      Returns the character pointed to by the m_rPtr pointer
         */
        char fgetc (SD::File *f) {
            char c;
            uint16_t ptr = f->rPtr % SD::SECTOR_SIZE;

            // Determine if the currently loaded sector is what we need
            uint32_t sectorOffset = (f->rPtr >> SD::SECTOR_SIZE_SHIFT);

            // Determine if the correct sector is loaded
            if (f->buf->id != f->id)
                this->reload_buf(f);
            else if (sectorOffset != f->curSector) {
#ifdef SD_OPTION_VERBOSE
                printf("File sector offset: 0x%08x / %u\n", sectorOffset,
                        sectorOffset);
#endif
                this->load_sector_from_offset(f, sectorOffset);
            }
            ++(f->rPtr);
            c = f->buf->buf[ptr];
            return c;
        }

        /**
         * @brief       Read a line from a file until either 'size' characters
         *              have been read or a newline is found; Parameters should
         *              match tinyio.h's fgets except for the file pointer
         *
         * @note        This function does not include error checking
         *
         * @pre         *f must point to a currently opened and valid file
         *
         * @param[out]  s[]     Character array to store file characters
         * @param[in]   size    Maximum number of characters to read from the
         *                      file
         * @param[in]   *f      Address with the currently opened file
         *
         * @return      Returns character memory location of character array
         */
        char * fgets (char s[], uint32_t size, SD::File *f) {
            /**
             * Code taken from fgets.c in the propgcc source, originally written
             * by Eric R. Smith and (slightly) modified to fit this SD driver
             */
            uint32_t c;
            uint32_t count = 0;

            --size;
            while (count < size) {
                c = this->fgetc(f);
                if ((uint32_t) EOF == c)
                    break;
                s[count++] = c;
                if ('\n' == c)
                    break;
            }
            s[count] = 0;
            return (0 < count) ? s : NULL;
        }

        /**
         * @brief       Determine whether the read pointer has reached the end
         *              of the file
         *
         * @pre         *f must point to a currently opened and valid file
         *
         * @param[in]   *f  Address of the requested file
         *
         * @return      Returns true if the read pointer points to the end of
         *              the file, false otherwise
         */
        inline bool feof (SD::File *f) {
            return f->length == f->rPtr;
        }

        /**
         * @brief       Set the read pointer for a given file to the position
         *              'origin + offset'
         *
         * @pre         *f must be an opened file
         *
         * @param[in]   *f      Address of the file object being referenced
         * @param[in]   offset  Bytes beyond 'origin' to set the pointer to
         * @param[in]   origin  Gives a reference to the offset; can be one of
         *                      SEEK_SET, SEEK_CUR, SEEK_END
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode fseekr (SD::File *f, const int32_t offset,
                const uint8_t origin) {
            switch (origin) {
                case SEEK_SET:
                    f->rPtr = offset;
                    break;
                case SEEK_CUR:
                    f->rPtr = f->rPtr + offset;
                    break;
                case SEEK_END:
                    f->rPtr = f->length + offset - 1;
                    break;
                default:
                    return SD::INVALID_PTR_ORIGIN;
                    break;
            }

            return 0;
        }

        /**
         * @brief       Set the write pointer for a given file to the position
         *              'origin + offset'
         *
         * @pre         *f must be an opened file
         *
         * @param[in]   *f      Address of the file object being referenced
         * @param[in]   offset  Bytes beyond 'origin' to set the pointer to
         * @param[in]   origin  Gives a reference to the offset; can be one of
         *                      SEEK_SET, SEEK_CUR, SEEK_END
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode fseekw (SD::File *f, const int32_t offset,
                const uint8_t origin) {
            switch (origin) {
                case SEEK_SET:
                    f->wPtr = offset;
                    break;
                case SEEK_CUR:
                    f->wPtr += offset;
                    break;
                case SEEK_END:
                    f->wPtr = f->length + offset - 1;
                    break;
                default:
                    return SD::INVALID_PTR_ORIGIN;
                    break;
            }

            return 0;
        }

        /**
         * @brief       Retrieve the current position of the read pointer
         *
         * @pre         *f must be an opened file
         *
         * @param[in]   *f  Address of the file object being referenced
         *
         * @return      Returns the byte offset (from beginning) of the read
         *              pointer
         */
        int32_t ftellr (const SD::File *f) {
            return f->rPtr;
        }

        /**
         * @brief       Retrieve the current position of the write pointer
         *
         * @pre         *f must be an opened file
         *
         * @param[in]   *f  Address of the file object being referenced
         *
         * @return      Returns the byte offset (from beginning) of the write
         *              pointer
         */
        int32_t ftellw (const SD::File *f) {
            return f->wPtr;
        }

#ifdef SD_OPTION_SHELL
        /**
         * @brief       Provide the user with a very basic Unix-like shell. The
         *              following commands are available to the user: ls, cat,
         *              cd.
         *
         * @param[in]   *f  If a file is opened via a command such as 'cat', its
         *                  information will be stored at this address
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode shell (SD::File *f) {
            char usrInput[SD_SHELL_INPUT_LEN] = "";
            char cmd[SD_SHELL_CMD_LEN] = "";
            char arg[SD_SHELL_ARG_LEN] = "";
            char uppercaseName[SD_SHELL_ARG_LEN] = "";
            uint8_t i, j;
            PropWare::ErrorCode err;

            printf("Welcome to David's quick shell! "
                    "There is no help, nor much to do.\n");
            printf("Have fun...\n");

            // Loop until the user types the SD::SHELL_EXIT string
            while (strcmp(usrInput, this->SHELL_EXIT)) {
                printf(">>> ");
#ifdef USE_PRINTF
                gets(usrInput);
#else
                getStr(usrInput, SD_SHELL_INPUT_LEN - 1);
#endif

#ifdef SD_OPTION_VERBOSE
                printf("Received \"%s\" as the complete line\n", usrInput);
#endif

                // Retrieve command
                for (i = 0; (0 != usrInput[i] && ' ' != usrInput[i]); ++i)
                    cmd[i] = usrInput[i];

#ifdef SD_OPTION_VERBOSE
                printf("Received \"%s\" as command\n", cmd);
#endif

                // Retrieve argument if it exists (skip over spaces)
                if (0 != usrInput[i]) {
                    j = 0;
                    while (' ' == usrInput[i])
                        ++i;
                    for (; (0 != usrInput[i] && ' ' != usrInput[i]); ++i)
                        arg[j++] = usrInput[i];
#ifdef SD_OPTION_VERBOSE
                    printf("And \"%s\" as the argument\n", arg);
#endif
                }

                // Convert the arg to uppercase
                for (i = 0; arg[i]; ++i)
                    if ('a' <= arg[i] && 'z' >= arg[i])
                        uppercaseName[i] = arg[i] + 'A' - 'a';
                    else
                        uppercaseName[i] = arg[i];

                // Interpret the command
                if (!strcmp(cmd, this->SHELL_LS))
                    err = this->shell_ls();
                else if (!strcmp(cmd, this->SHELL_CAT))
                    err = this->shell_cat(uppercaseName, f);
                else if (!strcmp(cmd, this->SHELL_CD))
                    err = this->chdir(uppercaseName);
#ifdef SD_OPTION_FILE_WRITE
                else if (!strcmp(cmd, this->SHELL_CAT))
                    err = this->shell_touch(uppercaseName);
#endif
#ifdef SD_OPTION_VERBOSE_BLOCKS
                else if (!strcmp(cmd, "d"))
                this->print_hex_block(this->m_buf.buf, SD::SECTOR_SIZE);
#endif
                else if (!strcmp(cmd, SD::SHELL_EXIT))
                    break;
                else if (strcmp(usrInput, ""))
                    printf("Invalid command: %s\n", cmd);

                // Handle errors; Print user errors and continue; Return system
                // errors
                if (err) {
                    if (SD::BEG_ERROR <= err && err <= SD::END_USER_ERRORS)
                        SD::print_error_str((SD::ErrorCode) err);
                    else
                        return err;
                }

                // Erase the command and argument strings
                for (i = 0; i < SD::SHELL_CMD_LEN; ++i)
                    cmd[i] = 0;
                for (i = 0; i < SD::SHELL_ARG_LEN; ++i)
                    uppercaseName[i] = arg[i] = 0;
                err = 0;
            }

            return 0;
        }

        /**
         * @brief       List the contents of a directory on the screen (similar
         *              to 'ls .')
         *
         * @note        TODO: Implement *abspath when SDGetSectorFromPath() is
         *              functional
         *
         * @param[in]   *absPath    Absolute path of the directory to be printed
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode shell_ls () {
            PropWare::ErrorCode err;
            uint16_t fileEntryOffset = 0;

            // Allocate space for a filename string
            char string[PropWare::SD::FILENAME_STR_LEN];

            // If we aren't looking at the beginning of a cluster, we must
            // backtrack to the beginning and then begin listing files
            if (this->m_buf.curSectorOffset
                    || (this->find_sector_from_alloc(this->m_dir_firstAllocUnit)
                            != this->m_buf.curClusterStartAddr)) {
#ifdef SD_OPTION_VERBOSE
                printf("'ls' requires a backtrack to beginning of directory's "
                        "cluster\n");
#endif
                this->m_buf.curClusterStartAddr = this->find_sector_from_alloc(
                        this->m_dir_firstAllocUnit);
                this->m_buf.curSectorOffset = 0;
                this->m_buf.curAllocUnit = this->m_dir_firstAllocUnit;
                check_errors(
                        this->get_fat_value(this->m_buf.curAllocUnit,
                                &(this->m_buf.nextAllocUnit)));
                check_errors(
                        this->read_data_block(this->m_buf.curClusterStartAddr,
                                this->m_buf.buf));
            }

            // Loop through all files in the current directory until we find the
            // correct one; Function will exit normally without an error code if
            // the file is not found
            while (this->m_buf.buf[fileEntryOffset]) {
                // Check if file is valid, retrieve the name if it is
                if ((SD::DELETED_FILE_MARK != this->m_buf.buf[fileEntryOffset])
                        && !(SD::SYSTEM_FILE
                                & this->m_buf.buf[fileEntryOffset
                                        + SD::FILE_ATTRIBUTE_OFFSET]))
                    this->print_file_entry(&(this->m_buf.buf[fileEntryOffset]),
                            string);

                // Increment to the next file
                fileEntryOffset += SD::FILE_ENTRY_LENGTH;

                // If it was the last entry in this sector, proceed to the next
                // one
                if (SD::SECTOR_SIZE == fileEntryOffset) {
                    // Last entry in the sector, attempt to load a new sector
                    // Possible error value includes end-of-chain marker
                    if ((err = this->load_next_sector(&this->m_buf))) {
                        if ((uint8_t) SD::EOC_END == err)
                            break;
                        else
                            return err;
                    }

                    fileEntryOffset = 0;
                }
            }

            return 0;
        }

        /**
         * @brief       Dump the contents of a file to the screen (similar to
         *              'cat f');
         *
         * @note        Does not currently follow paths
         *
         * @param[in]   *f  Short filename of file to print
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode shell_cat (const char *name,
                PropWare::SD::File *f) {
            PropWare::ErrorCode err;

            // Attempt to find the file
            if ((err = this->fopen(name, f, SD::FILE_MODE_R))) {
                if ((uint8_t) SD::EOC_END == err)
                    return err;
                else
                    return err;
            } else {
                // Loop over each character and print them to the screen
                // one-by-one
                while (!this->feof(f))
                    // Using SDfgetc() instead of SDfgets to ensure
                    // compatibility with binary files
                    // TODO: Should probably create something better to output
                    //       binary files don't ya think!?
                    putchar(this->fgetc(f));
                putchar('\n');
            }

            return 0;
        }

#ifdef SD_OPTION_FILE_WRITE
        /**
         * @brief       Create a new file, do not open it
         *
         * @param[in]   name[]  C-string name for the file to be created
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode shell_touch (const char name[]) {
            PropWare::ErrorCode err;
            uint16_t fileEntryOffset;

            // Attempt to find the file if it already exists
            if ((err = this->find(name, &fileEntryOffset))) {
                // Error occurred - hopefully it was a "file not found" error
                if (SD::FILENAME_NOT_FOUND == err)
                    // File wasn't found, let's create it
                    err = this->create_file(name, &fileEntryOffset);
                return err;
            }

            // If SDFind() returns 0, the file already existed and an error
            // should be thrown
            return SD::FILE_ALREADY_EXISTS;
        }
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
        void print_hex_block (uint8_t *dat, uint16_t bytes) {
            uint8_t i, j;
            uint8_t *s;

            printf("Printing %u bytes...\n", bytes);
            printf("Offset\t");
            for (i = 0; i < SD::LINE_SIZE; ++i)
            printf("0x%x  ", i);
            putchar('\n');

            if (bytes % SD::LINE_SIZE)
            bytes = bytes / SD::LINE_SIZE + 1;
            else
            bytes /= SD::LINE_SIZE;

            for (i = 0; i < bytes; ++i) {
                s = (uint8_t *) (dat + SD::LINE_SIZE * i);
                printf("0x%04x:\t", SD::LINE_SIZE * i);
                for (j = 0; j < SD::LINE_SIZE; ++j)
                printf("0x%02x ", s[j]);
                printf(" - ");
                for (j = 0; j < SD::LINE_SIZE; ++j) {
                    if ((' ' <= s[j]) && (s[j] <= '~'))
                    putchar(s[j]);
                    else
                    putchar('.');
                }

                putchar('\n');
            }
        }
#endif

        /**
         * @brief   Create a human-readable error string
         *
         * @param[in]   err         Error number used to determine error string
         * @param[out]  errorStr    Allocated space where a string of no more
         *                          than 128 characters can be printed
         */
        void print_error_str (const SD::ErrorCode err) const {
            const char str[] = "SD Error %u: %s\n";
            const uint8_t relativeError = err - SD::BEG_ERROR;

            switch (err) {
                case SD::INVALID_CMD:
                    printf(str, relativeError, "Invalid command");
                    break;
                case SD::READ_TIMEOUT:
                    printf(str, relativeError, "Timed out "
                            "during read");
                    break;
                case SD::INVALID_NUM_BYTES:
                    printf(str, relativeError, "Invalid number of bytes");
                    break;
                case SD::INVALID_RESPONSE:
#ifdef SD_OPTION_VERBOSE
                    printf("SD Error %u: %s0x%02x\nThe following bits are "
                            "set:\n", relativeError,
                            "Invalid first-byte response\n\tReceived: ",
                            this->m_firstByteResponse);
#else
                    printf("SD Error %u: %s%u\n", relativeError,
                            "Invalid first-byte response\n\tReceived: ",
                            this->m_firstByteResponse);
#endif
                    this->first_byte_expansion();
                    break;
                case SD::INVALID_INIT:
#ifdef SD_OPTION_VERBOSE
                    printf("SD Error %u: %s\n\tResponse: 0x%02x\n",
                            relativeError,
                            "Invalid response during initialization",
                            this->m_firstByteResponse);
#else
                    printf("SD Error %u: %s\n\tResponse: %u\n", relativeError,
                            "Invalid response during initialization",
                            this->m_firstByteResponse);
#endif
                    break;
                case SD::INVALID_FILESYSTEM:
                    printf(str, relativeError, "Invalid file system; Likely not"
                            " a FAT16 or FAT32 system");
                    break;
                case SD::INVALID_DAT_STRT_ID:
#ifdef SD_OPTION_VERBOSE
                    printf("SD Error %u: %s0x%02x\n", relativeError,
                            "Invalid data-start ID\n\tReceived: ",
                            this->m_firstByteResponse);
#else
                    printf("SD Error %u: %s%u\n", relativeError,
                            "Invalid data-start ID\n\tReceived: ",
                            this->m_firstByteResponse);
#endif
                    break;
                case SD::FILENAME_NOT_FOUND:
                    printf(str, relativeError, "Filename not found");
                    break;
                case SD::EMPTY_FAT_ENTRY:
                    printf(str, relativeError, "FAT points to empty entry");
                    break;
                case SD::CORRUPT_CLUSTER:
                    printf(str, relativeError, "SD cluster is corrupt");
                    break;
                case SD::INVALID_PTR_ORIGIN:
                    printf(str, relativeError, "Invalid pointer origin");
                    break;
                case SD::ENTRY_NOT_FILE:
                    printf(str, relativeError,
                            "Requested file entry is not a file");
                    break;
                case SD::INVALID_FILENAME:
                    printf(str, relativeError,
                            "Invalid filename - please use 8.3 format");
                    break;
                case SD::INVALID_FAT_APPEND:
                    printf(str, relativeError,
                            "FAT entry append was attempted unnecessarily");
                    break;
                case SD::FILE_ALREADY_EXISTS:
                    printf(str, relativeError,
                            "Attempting to create an already existing file");
                    break;
                case SD::INVALID_FILE_MODE:
                    printf(str, relativeError, "Invalid file mode");
                    break;
                case SD::TOO_MANY_FATS:
                    printf(str, relativeError,
                            "This driver is only capable of writing files on "
                                    "FAT partitions with two (2) copies of the "
                                    "FAT");
                    break;
                case SD::FILE_WITHOUT_BUFFER:
                    printf(str, relativeError,
                            "SDfopen() was passed a file struct with "
                                    "an uninitialized buffer");
                    break;
                case SD::CMD8_FAILURE:
                    printf(str, relativeError,
                            "CMD8 never received a proper response; This is "
                                    "most likely to occur when the SD card "
                                    "does not support the 3.3V I/O used by "
                                    "the Propeller");
                    break;
                case SD::READING_PAST_EOC:
                    printf(str, relativeError, "Reading past the"
                            " end-of-chain marker");
                    break;
                case SD::ENTRY_NOT_DIR:
                    printf(str, relativeError, "Requested name is not a"
                            " directory");
                    break;
                default:
                    return;
            }
        }

    private:
        typedef struct {
            public:
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
        inline PropWare::ErrorCode reset_and_verify_v2_0 (uint8_t response[]) {
            PropWare::ErrorCode err;
            uint8_t i, j;
            bool stageCleared;

#ifdef SD_OPTION_VERBOSE
            printf("Starting SD card...\n");
#endif

            // Attempt initialization no more than 10 times
            stageCleared = false;
            for (i = 0; i < 10 && !stageCleared; ++i) {
                // Initialization loop (reset SD card)
                for (j = 0; j < 10 && !stageCleared; ++j) {
                    check_errors(this->power_up());

                    check_errors(this->reset(response, &stageCleared));
                }

                // If we couldn't go idle after 10 tries, give up
                if (!stageCleared)
                    return SD::INVALID_INIT;

                stageCleared = false;
                check_errors(this->verify_v2_0(response, &stageCleared));
            }

            // If CMD8 never succeeded, throw an error
            if (!stageCleared)
                return SD::CMD8_FAILURE;

            // The card is idle, that's good. Let's make sure we get the correct
            // response back
            if ((SD::HOST_VOLTAGE_3V3 != response[2])
                    || (SD::R7_CHECK_PATTERN != response[3]))
                return SD::CMD8_FAILURE;

            return 0;
        }

        inline PropWare::ErrorCode power_up () {
            uint8_t i;
            PropWare::ErrorCode err;

            waitcnt(CLKFREQ / 10 + CNT);

            // Send at least 72 clock cycles to enable the SD card
            this->m_cs.set();
            for (i = 0; i < 128; ++i)
                check_errors(this->m_spi->shift_out(16, -1));

            // Be very super 100% sure that all clocks have finished ticking
            // before setting chip select low
            check_errors(this->m_spi->wait());
            waitcnt(10*MILLISECOND + CNT);

            // Chip select goes low for the duration of this function
            this->m_cs.clear();

            return 0;
        }

        inline PropWare::ErrorCode reset (uint8_t response[], bool *isIdle) {
            PropWare::ErrorCode err;

            // Send SD into idle state, retrieve a response and ensure it is the
            // "idle" response
            check_errors(this->send_command(SD::CMD_IDLE, 0, SD::CRC_IDLE));
            this->get_response(SD::RESPONSE_LEN_R1, response);

            // Check if idle
            if (SD::RESPONSE_IDLE == this->m_firstByteResponse)
                *isIdle = true;
#ifdef SD_OPTION_VERBOSE
            else
            printf("Failed attempt at CMD0: 0x%02x\n",
                    this->m_firstByteResponse);
#endif

            return 0;
        }

        inline PropWare::ErrorCode verify_v2_0 (uint8_t response[],
                bool *stageCleared) {
            PropWare::ErrorCode err;

#ifdef SD_OPTION_VERBOSE
            printf("SD card in idle state. Now sending CMD8...\n");
#endif

            // Inform SD card that the Propeller uses the 2.7-3.6V range;
            check_errors(
                    this->send_command(SD::CMD_INTERFACE_COND, SD::ARG_CMD8,
                            SD::CRC_CMD8));
            check_errors(this->get_response(SD::RESPONSE_LEN_R7, response));
            if (SD::RESPONSE_IDLE == this->m_firstByteResponse)
                *stageCleared = true;

            // Print an error message after every failure
#ifdef SD_OPTION_VERBOSE
            if (!stageCleared)
            printf("Failed attempt at CMD8: 0x%02x, 0x%02x, 0x%02x;\n",
                    this->m_firstByteResponse, response[2], response[3]);
#endif

            return 0;
        }

        inline PropWare::ErrorCode send_active (uint8_t response[]) {
            PropWare::ErrorCode err;
            uint32_t timeout;
            uint32_t longWiggleRoom = 3 * MILLISECOND;
            bool stageCleared = false;

            // Attempt to send active
            timeout = SD::SEND_ACTIVE_TIMEOUT + CNT;  //
            do {
                // Send the application-specific pre-command
                check_errors(
                        this->send_command(SD::CMD_APP, 0, SD::CRC_ACMD_PREP));
                check_errors(this->get_response(SD::RESPONSE_LEN_R1, response));

                // Request that the SD card go active!
                check_errors(this->send_command(SD::CMD_WR_OP, BIT_30, 0));
                check_errors(this->get_response(SD::RESPONSE_LEN_R1, response));

                // If the card ACKed with the active state, we're all good!
                if (SD::RESPONSE_ACTIVE == this->m_firstByteResponse)
                    stageCleared = true;

                // Check for timeout
                if (abs(timeout - CNT) < longWiggleRoom)
                    return SD::READ_TIMEOUT;

                // Wait until we have received the active response
            } while (!stageCleared);

#ifdef SD_OPTION_VERBOSE
            // We did it!
            printf("Activated!\n");
#endif

            return 0;
        }

        inline PropWare::ErrorCode increase_throttle (const int32_t freq) {
            PropWare::ErrorCode err;

            // Initialization nearly complete, increase clock
#ifdef SD_OPTION_VERBOSE
            printf("Increasing clock to full speed\n");
#endif
            if (-1 == freq || 0 == freq) {
                check_errors(this->m_spi->set_clock(SD::DEFAULT_SPI_FREQ));
            } else {
                check_errors(this->m_spi->set_clock(freq));
            }

            return 0;
        }

        inline PropWare::ErrorCode read_boot_sector (InitFATInfo *fatInfo) {
            PropWare::ErrorCode err;
            // Read in first sector
            check_errors(
                    this->read_data_block(fatInfo->bootSector,
                            this->m_buf.buf));
            // Check if sector 0 is boot sector or MBR; if MBR, skip to boot
            // sector at first partition
            if (SD::BOOT_SECTOR_ID
                    != this->m_buf.buf[SD::BOOT_SECTOR_ID_ADDR]) {
                fatInfo->bootSector = this->read_rev_dat32(
                        &(this->m_buf.buf[SD::BOOT_SECTOR_BACKUP]));
                check_errors(
                        this->read_data_block(fatInfo->bootSector,
                                this->m_buf.buf));
            }

            // Print the boot sector if requested
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_VERBOSE_BLOCKS)
            printf("***BOOT SECTOR***\n");
            this->print_hex_block(this->m_buf.buf, SD::SECTOR_SIZE);
            putchar('\n');
#endif

            return 0;
        }

        inline PropWare::ErrorCode common_boot_sector_parser (
                InitFATInfo *fatInfo) {
            uint8_t temp;

            // Determine number of sectors per cluster
            temp = this->m_buf.buf[SD::CLUSTER_SIZE_ADDR];
#ifdef SD_OPTION_VERBOSE
            printf("Preliminary sectors per cluster: %u\n", temp);
#endif
            while (temp) {
                temp >>= 1;
                ++this->m_sectorsPerCluster_shift;
            }
            --this->m_sectorsPerCluster_shift;

            // Get the reserved sector count
            fatInfo->rsvdSectorCount = this->read_rev_dat16(
                    &((this->m_buf.buf)[SD::RSVD_SCTR_CNT_ADDR]));

            // Total number of FATs
            fatInfo->numFATs = this->m_buf.buf[SD::NUM_FATS_ADDR];
#ifdef SD_OPTION_FILE_WRITE
            if (2 != fatInfo->numFATs)
                return SD::TOO_MANY_FATS;
#endif

            // Number of entries in the root directory
            fatInfo->rootEntryCount = this->read_rev_dat16(
                    &(this->m_buf.buf[SD::ROOT_ENTRY_CNT_ADDR]));

            return 0;
        }

        inline void partition_info_parser (InitFATInfo *fatInfo) {
            // Check if FAT size is valid in 16- or 32-bit location
            fatInfo->FATSize = this->read_rev_dat16(
                    &(this->m_buf.buf[SD::FAT_SIZE_16_ADDR]));
            if (!(fatInfo->FATSize))
                fatInfo->FATSize = this->read_rev_dat32(
                        &(this->m_buf.buf[SD::FAT_SIZE_32_ADDR]));

            // Check if FAT16 total sectors is valid
            fatInfo->totalSectors = this->read_rev_dat16(
                    &(this->m_buf.buf[SD::TOT_SCTR_16_ADDR]));
            if (!(fatInfo->totalSectors))
                fatInfo->totalSectors = this->read_rev_dat32(
                        &(this->m_buf.buf[SD::TOT_SCTR_32_ADDR]));

            // Compute necessary numbers to determine FAT type (12/16/32)
            this->m_rootDirSectors = (fatInfo->rootEntryCount * 32)
                    >> SD::SECTOR_SIZE_SHIFT;
            fatInfo->dataSectors = fatInfo->totalSectors
                    - (fatInfo->rsvdSectorCount
                            + fatInfo->numFATs * fatInfo->FATSize
                            + fatInfo->rootEntryCount);
            fatInfo->clusterCount = fatInfo->dataSectors
                    >> this->m_sectorsPerCluster_shift;

#ifdef SD_OPTION_VERBOSE
            printf("Sectors per cluster: %u\n",
                    1 << this->m_sectorsPerCluster_shift);
            printf("Reserved sector count: 0x%08x / %u\n",
                    fatInfo->rsvdSectorCount, fatInfo->rsvdSectorCount);
            printf("Number of FATs: 0x%02x / %u\n", fatInfo->numFATs,
                    fatInfo->numFATs);
            printf("Total sector count: 0x%08x / %u\n", fatInfo->totalSectors,
                    fatInfo->totalSectors);
            printf("Total cluster count: 0x%08x / %u\n", fatInfo->clusterCount,
                    fatInfo->clusterCount);
            printf("Total data sectors: 0x%08x / %u\n", fatInfo->dataSectors,
                    fatInfo->dataSectors);
            printf("FAT Size: 0x%04x / %u\n", fatInfo->FATSize,
                    fatInfo->FATSize);
            printf("Root directory sectors: 0x%08x / %u\n",
                    this->m_rootDirSectors, this->m_rootDirSectors);
            printf("Root entry count: 0x%08x / %u\n", fatInfo->rootEntryCount,
                    fatInfo->rootEntryCount);
#endif
        }

        inline PropWare::ErrorCode determine_fat_type (InitFATInfo *fatInfo) {
            // Determine and store FAT type
            if (SD::FAT12_CLSTR_CNT > fatInfo->clusterCount)
                return SD::INVALID_FILESYSTEM;
            else if (SD::FAT16_CLSTR_CNT > fatInfo->clusterCount) {
#ifdef SD_OPTION_VERBOSE
                printf("\n***FAT type is FAT16***\n");
#endif
                this->m_filesystem = SD::FAT_16;
                this->m_entriesPerFatSector_Shift = 8;
            } else {
#ifdef SD_OPTION_VERBOSE
                printf("\n***FAT type is FAT32***\n");
#endif
                this->m_filesystem = SD::FAT_32;
                this->m_entriesPerFatSector_Shift = 7;
            }

            return 0;
        }

        inline void store_root_info (InitFATInfo *fatInfo) {
            // Find start of FAT
            this->m_fatStart = fatInfo->bootSector + fatInfo->rsvdSectorCount;

            //    this->m_filesystem = SD::FAT_16;
            // Find root directory address
            switch (this->m_filesystem) {
                case SD::FAT_16:
                    this->m_rootAddr = fatInfo->FATSize * fatInfo->numFATs
                            + this->m_fatStart;
                    this->m_firstDataAddr = this->m_rootAddr
                            + this->m_rootDirSectors;
                    break;
                case SD::FAT_32:
                    this->m_firstDataAddr = this->m_rootAddr =
                            fatInfo->bootSector + fatInfo->rsvdSectorCount
                                    + fatInfo->FATSize * fatInfo->numFATs;
                    this->m_rootAllocUnit = this->read_rev_dat32(
                            &(this->m_buf.buf[SD::ROOT_CLUSTER_ADDR]));
                    break;
            }

#ifdef SD_OPTION_FILE_WRITE
            // If files will be writable, the second FAT must also be updated -
            // the first sector's address is stored here
            this->m_fatSize = fatInfo->FATSize;
#endif

#ifdef SD_OPTION_VERBOSE
            printf("Start of FAT: 0x%08x\n", this->m_fatStart);
            printf("Root directory alloc. unit: 0x%08x\n",
                    this->m_rootAllocUnit);
            printf("Root directory sector: 0x%08x\n", this->m_rootAddr);
            printf("Calculated root directory sector: 0x%08x\n",
                    this->find_sector_from_alloc(this->m_rootAllocUnit));
            printf("First data sector: 0x%08x\n", this->m_firstDataAddr);
#endif
        }

        inline PropWare::ErrorCode read_fat_and_root_sectors () {
            PropWare::ErrorCode err;

            // Store the first sector of the FAT
            check_errors(this->read_data_block(this->m_fatStart, this->m_fat));
            this->m_curFatSector = 0;

            // Print FAT if desired
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_VERBOSE_BLOCKS)
            printf("\n***First File Allocation Table***\n");
            this->print_hex_block(this->m_fat, SD::SECTOR_SIZE);
            putchar('\n');
#endif

            // Read in the root directory, set root as current
            check_errors(
                    this->read_data_block(this->m_rootAddr, this->m_buf.buf));
            this->m_buf.curClusterStartAddr = this->m_rootAddr;
            if (SD::FAT_16 == this->m_filesystem) {
                this->m_dir_firstAllocUnit = -1;
                this->m_buf.curAllocUnit = -1;
            } else {
                this->m_buf.curAllocUnit = this->m_dir_firstAllocUnit =
                        this->m_rootAllocUnit;
                check_errors(
                        this->get_fat_value(this->m_buf.curAllocUnit,
                                &this->m_buf.nextAllocUnit));
            }
            this->m_buf.curClusterStartAddr = this->m_rootAddr;
            this->m_buf.curSectorOffset = 0;

            // Print root directory
#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
            printf("***Root directory***\n");
            this->print_hex_block(this->m_buf.buf, SD::SECTOR_SIZE);
            putchar('\n');
#endif

            return 0;
        }

#if (defined SD_OPTION_VERBOSE)
        PropWare::ErrorCode print_init_debug_blocks (uint8_t response[]) {
            PropWare::ErrorCode err;

            // Request operating conditions register and ensure response begins
            // with R1
            check_errors(this->send_command(SD::CMD_READ_OCR, 0,
                            SD::CRC_OTHER));
            check_errors(this->get_response(SD::RESPONSE_LEN_R3, response));
            printf("Operating Conditions Register (OCR)...\n");
            this->print_hex_block(response, SD::RESPONSE_LEN_R3);

            // If debugging requested, print to the screen CSD and CID registers
            // from SD card
            printf("Requesting CSD...\n");
            check_errors(this->send_command(SD::CMD_RD_CSD, 0, SD::CRC_OTHER));
            check_errors(this->read_block(16, response));
            printf("CSD Contents:\n");
            this->print_hex_block(response, 16);
            putchar('\n');

            printf("Requesting CID...\n");
            check_errors(this->send_command(SD::CMD_RD_CID, 0, SD::CRC_OTHER));
            check_errors(this->read_block(16, response));
            printf("CID Contents:\n");
            this->print_hex_block(response, 16);
            putchar('\n');

            return 0;
        }
#endif

        /**
         * @brief       Send a command and argument over SPI to the SD card
         *
         * @param[in]   command     6-bit value representing the command sent to
         *                          the SD card
         * @param[in]   arg         Any argument applicable to the command
         * @param[in]   crc         CRC for the command and argument
         *
         * @return      Returns 0 for success, else error code
         */
        PropWare::ErrorCode send_command (const uint8_t cmd, const uint32_t arg,
                const uint8_t crc) {
            PropWare::ErrorCode err;

            // Send out the command
            check_errors(this->m_spi->shift_out(8, cmd));

            // Send argument
            check_errors(this->m_spi->shift_out(16, (arg >> 16)));
            check_errors(this->m_spi->shift_out(16, arg & WORD_0));

            // Send sixth byte - CRC
            check_errors(this->m_spi->shift_out(8, crc));

            return 0;
        }

        /**
         * @brief       receive response and data from SD card over SPI
         *
         * @param[in]   bytes   Number of bytes to receive
         * @param[out]  *data   Location in memory with enough space to store
         *                      `bytes` bytes of data
         *
         * @return      Returns 0 for success, else error code
         */
        PropWare::ErrorCode get_response (uint8_t numBytes, uint8_t *dat) {
            PropWare::ErrorCode err;
            uint32_t timeout;

            // Read first byte - the R1 response
            timeout = SD::RESPONSE_TIMEOUT + CNT;
            do {
                check_errors(
                        this->m_spi->shift_in(8, &this->m_firstByteResponse,
                                sizeof(this->m_firstByteResponse)));

                // Check for timeout
                if (abs(timeout - CNT) < SD::SINGLE_BYTE_WIGGLE_ROOM)
                    return SD::READ_TIMEOUT;

                // wait for transmission end
            } while (0xff == this->m_firstByteResponse);

            // First byte in a response should always be either IDLE or ACTIVE.
            // If this one wans't, throw an error. If it was, decrement the
            // bytes counter and read in all remaining bytes
            if ((SD::RESPONSE_IDLE == this->m_firstByteResponse)
                    || (SD::RESPONSE_ACTIVE == this->m_firstByteResponse)) {
                --numBytes;    // Decrement bytes counter

                // Read remaining bytes
                while (numBytes--)
                    check_errors(this->m_spi->shift_in(8, dat++, sizeof(*dat)));
            } else
                return SD::INVALID_RESPONSE;

            // Responses should always be followed up by outputting 8 clocks
            // with MOSI high
            check_errors(this->m_spi->shift_out(16, -1));
            check_errors(this->m_spi->shift_out(16, -1));
            check_errors(this->m_spi->shift_out(16, -1));
            check_errors(this->m_spi->shift_out(16, -1));

            return 0;
        }

        /**
         * @brief       Receive data from SD card via SPI
         *
         * @param[in]   bytes   Number of bytes to receive
         * @param[out]  *data   Location in memory with enough space to store
         *                      `bytes` bytes of data
         *
         * @return      Returns 0 for success, else error code
         */
        PropWare::ErrorCode read_block (uint16_t bytes, uint8_t *dat) {
            uint8_t i, err, checksum;
            uint32_t timeout;

            // Read first byte - the R1 response
            timeout = SD::RESPONSE_TIMEOUT + CNT;
            do {
                check_errors(
                        this->m_spi->shift_in(8, &this->m_firstByteResponse,
                                sizeof(this->m_firstByteResponse)));

                // Check for timeout
                if (abs(timeout - CNT) < SD::SINGLE_BYTE_WIGGLE_ROOM)
                    return SD::READ_TIMEOUT;

                // wait for transmission end
            } while (0xff == this->m_firstByteResponse);

            // Ensure this response is "active"
            if (SD::RESPONSE_ACTIVE == this->m_firstByteResponse) {
                // Ignore blank data again
                timeout = SD::RESPONSE_TIMEOUT + CNT;
                do {
                    check_errors(this->m_spi->shift_in(8, dat, sizeof(*dat)));

                    // Check for timeout
                    if (abs(timeout - CNT) < SD::SINGLE_BYTE_WIGGLE_ROOM)
                        return SD::READ_TIMEOUT;

                    // wait for transmission end
                } while (SD::DATA_START_ID != *dat);

                // Check for the data start identifier and continue reading data
                if (SD::DATA_START_ID == *dat) {
                    // Read in requested data bytes
#if (defined SPI_FAST_SECTOR)
                    if (SD::SECTOR_SIZE == bytes) {
                        this->m_spi->shift_in_sector(dat, 1);
                        bytes = 0;
                    }
#endif
                    while (bytes--) {
#ifdef SPI_OPTION_FAST
                        check_errors(
                                this->m_spi->shift_in_fast(8, dat++,
                                        sizeof(*dat)));
#else
                        check_errors(this->m_spi->shift_in(8, dat++,
                                        sizeof(*dat)));
#endif
                    }

                    // Read two more bytes for checksum - throw away data
                    for (i = 0; i < 2; ++i) {
                        timeout = SD::RESPONSE_TIMEOUT + CNT;
                        do {
                            check_errors(
                                    this->m_spi->shift_in(8, &checksum,
                                            sizeof(checksum)));

                            // Check for timeout
                            if ((timeout - CNT) < SD::SINGLE_BYTE_WIGGLE_ROOM)
                                return SD::READ_TIMEOUT;

                            // wait for transmission end
                        } while (0xff == checksum);
                    }

                    // Send final 0xff
                    check_errors(this->m_spi->shift_out(8, 0xff));
                } else {
                    return SD::INVALID_DAT_STRT_ID;
                }
            } else
                return SD::INVALID_RESPONSE;

            return 0;
        }

        /**
         * @brief       Write data to SD card via SPI
         *
         * @param[in]   bytes   Block address to read from SD card
         * @param[in]   *dat    Location in memory where data resides
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode write_block (uint16_t bytes, uint8_t *dat) {
            PropWare::ErrorCode err;
            uint32_t timeout;

            // Read first byte - the R1 response
            timeout = SD::RESPONSE_TIMEOUT + CNT;
            do {
                check_errors(
                        this->m_spi->shift_in(8, &this->m_firstByteResponse,
                                sizeof(this->m_firstByteResponse)));

                // Check for timeout
                if (abs(timeout - CNT) < SD::SINGLE_BYTE_WIGGLE_ROOM)
                    return SD::READ_TIMEOUT;

                // wait for transmission end
            } while (0xff == this->m_firstByteResponse);

            // Ensure this response is "active"
            if (SD::RESPONSE_ACTIVE == this->m_firstByteResponse) {
                // Received "active" response

                // Send data Start ID
                check_errors(this->m_spi->shift_out(8, SD::DATA_START_ID));

                // Send all bytes
                while (bytes--) {
#ifdef SPI_OPTION_FAST
                    check_errors(this->m_spi->shift_out_fast(8, *(dat++)));
#else
                    check_errors(this->m_spi->shift_out(8, *(dat++)));
#endif
                }

                // Receive and digest response token
                timeout = SD::RESPONSE_TIMEOUT + CNT;
                do {
                    check_errors(
                            this->m_spi->shift_in(8, &this->m_firstByteResponse,
                                    sizeof(this->m_firstByteResponse)));

                    // Check for timeout
                    if (abs(timeout - CNT) < SD::SINGLE_BYTE_WIGGLE_ROOM)
                        return SD::READ_TIMEOUT;

                    // wait for transmission end
                } while (0xff == this->m_firstByteResponse);
                if (SD::RSPNS_TKN_ACCPT
                        != (this->m_firstByteResponse
                                & (uint8_t) SD::RSPNS_TKN_BITS))
                    return SD::INVALID_RESPONSE;
            }

            return 0;
        }

        /**
         * @brief       Read SD_SECTOR_SIZE-byte data block from SD card
         *
         * @param[in]   address    Number of bytes to send
         * @param[out]  *dat       Location in chip memory to store data block
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode read_data_block (uint32_t address, uint8_t *dat) {
            /**
             * Special error handling is needed to ensure that, if an error is
             * thrown, chip select is set high again before returning the error
             */
            PropWare::ErrorCode err;
            uint8_t temp = 0;

            // Wait until the SD card is no longer busy
            while (!temp)
                this->m_spi->shift_in(8, &temp, sizeof(temp));

#ifdef SD_OPTION_VERBOSE
            printf("Reading block at sector address: 0x%08x / %u\n", address,
                    address);
#endif

            this->m_cs.clear();

            err = this->send_command(SD::CMD_RD_BLOCK, address, SD::CRC_OTHER);
            if (!err)
                err = this->read_block(SD::SECTOR_SIZE, dat);

            this->m_cs.set();

            if (err)
                return err;

            return 0;
        }

        /**
         * @brief       Write SD_SECTOR_SIZE-byte data block to SD card
         *
         * @param[in]   address     Block address to write to SD card
         * @param[in]   *dat        Location in chip memory to read data block
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode write_data_block (uint32_t address, uint8_t *dat) {
            PropWare::ErrorCode err;
            uint8_t temp = 0;

            // Wait until the SD card is no longer busy
            while (!temp)
                this->m_spi->shift_in(8, &temp, 1);

#ifdef SD_OPTION_VERBOSE
            printf("Writing block at address: 0x%08x / %u\n", address, address);
#endif

            this->m_cs.clear();
            check_errors(
                    this->send_command(SD::CMD_WR_BLOCK, address,
                            SD::CRC_OTHER));

            check_errors(this->write_block(SD::SECTOR_SIZE, dat));
            this->m_cs.set();

            return 0;
        }

        /**
         * @brief       Return byte-reversed 16-bit variable (SD cards store
         *              bytes little-endian therefore we must reverse them to
         *              use multi-byte variables)
         *
         * @param[in]   buf[]   Address of first byte of data
         *
         * @return      Returns a normal (big-endian) 16-bit word
         */
        uint16_t read_rev_dat16 (const uint8_t buf[]) {
            return (buf[1] << 8) + buf[0];
        }

        /**
         * @brief       Return byte-reversed 32-bit variable (SD cards store
         *              bytes little-endian therefore we must reverse them to
         *              use multi-byte variables)
         *
         * @param[in]   buf[]   Address of first byte of data
         *
         * @return      Returns a normal (big-endian) 32-bit word
         */
        uint32_t read_rev_dat32 (const uint8_t buf[]) {
            return (buf[3] << 24) + (buf[2] << 16) + (buf[1] << 8) + buf[0];
        }

#ifdef SD_OPTION_FILE_WRITE
        /**
         * @brief       Write a byte-reversed 16-bit variable (SD cards store
         *              bytes little-endian therefore we must reverse them to
         *              use multi-byte variables)
         *
         * @param[out]  buf[]   Address to store the first byte of data
         * @param[in]   dat     Normal, 16-bit variable to be written to RAM in
         *                      reverse endian
         */
        void write_rev_dat16 (uint8_t buf[], const uint16_t dat) {
            buf[1] = (uint8_t) (dat >> 8);
            buf[0] = (uint8_t) dat;
        }

        /**
         * @brief       Write a byte-reversed 32-bit variable (SD cards store
         *              bytes little-endian therefore we must reverse them to
         *              use multi-byte variables)
         *
         * @param[out]  buf[]   Address to store the first byte of data
         * @param[in]   dat     Normal, 32-bit variable to be written to RAM in
         *                      reverse endian
         */
        void write_rev_dat32 (uint8_t buf[], const uint32_t dat) {
            buf[3] = (uint8_t) (dat >> 24);
            buf[2] = (uint8_t) (dat >> 16);
            buf[1] = (uint8_t) (dat >> 8);
            buf[0] = (uint8_t) dat;
        }
#endif

        /**
         * @brief       Find and return the starting sector's address for a
         *              directory path given in a c-string. Use Unix-style path
         *              names (like /foo/bar/)
         *
         * @note        !!!Not yet implemented!!!
         *
         * @param[in]   *path   C-string representing Unix-style path
         *
         * @return      Returns sector address of desired path
         */
        // TODO: Implement this (more than simply returning root directory)
        // TODO: Allow for paths outside the current directory
        uint32_t find_sector_from_path (const char *path) {
            // TODO: Return an actual path

            /*if ('/' == path[0]) {
             } // Start from the root address
             else {
             } // Start from the current directory*/

            return this->m_rootAddr;
        }

        /**
         * @brief       Find and return the starting sector's address for a
         *              given allocation unit (note - not cluster)
         *
         * @param[in]   allocUnit   Allocation unit in FAT filesystem
         *
         * @return      Returns sector address of the desired allocation unit
         */
        uint32_t find_sector_from_alloc (uint32_t allocUnit) {
            if (SD::FAT_32 == this->m_filesystem)
                allocUnit -= this->m_rootAllocUnit;
            else
                allocUnit -= 2;
            allocUnit <<= this->m_sectorsPerCluster_shift;
            allocUnit += this->m_firstDataAddr;
            return allocUnit;
        }

        /**
         * @brief       Read an entry from the FAT
         *
         * @param[in]   fatEntry    Entry number (allocation unit) to read in
         *                          the FAT
         * @param[out]  *value      Address to store the value into (the next
         *                          allocation unit)
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode get_fat_value (const uint32_t fatEntry,
                uint32_t *value) {
            PropWare::ErrorCode err;
            uint32_t firstAvailableAllocUnit;

#ifdef SD_OPTION_VERBOSE
            printf("Reading from the FAT...\n");
            printf("\tLooking for entry: 0x%08x / %u\n", fatEntry, fatEntry);
#endif

            // Do we need to load a new fat sector?
            if ((fatEntry >> this->m_entriesPerFatSector_Shift)
                    != this->m_curFatSector) {
#ifdef SD_OPTION_FILE_WRITE
                // If the currently loaded FAT sector has been modified, save it
                if (this->m_fatMod) {
                    this->write_data_block(
                            this->m_curFatSector + this->m_fatStart,
                            this->m_fat);
                    this->write_data_block(
                            this->m_curFatSector + this->m_fatStart
                                    + this->m_fatSize, this->m_fat);
                    this->m_fatMod = false;
                }
#endif
                // Need new sector, load it
                this->m_curFatSector = fatEntry
                        >> this->m_entriesPerFatSector_Shift;
                check_errors(
                        this->read_data_block(
                                this->m_curFatSector + this->m_fatStart,
                                this->m_fat));
#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
                this->print_hex_block(this->m_fat, SD::SECTOR_SIZE);
#endif
            }
            firstAvailableAllocUnit = this->m_curFatSector
                    << this->m_entriesPerFatSector_Shift;

#ifdef SD_OPTION_VERBOSE
            printf("\tLooks like I need FAT sector: 0x%08x / %u\n",
                    this->m_curFatSector, this->m_curFatSector);
            printf("\tWith an offset of: 0x%04x / %u\n",
                    (fatEntry - firstAvailableAllocUnit) << 2,
                    (fatEntry - firstAvailableAllocUnit) << 2);
#endif

            // The necessary FAT sector has been loaded and the next allocation
            // unit is known, proceed with loading the next data sector and
            // incrementing the cluster variables

            // Retrieve the next allocation unit number
            if (SD::FAT_16 == this->m_filesystem)
                *value =
                        this->read_rev_dat16(
                                &this->m_fat[(fatEntry - firstAvailableAllocUnit)
                                        << 1]);

            else
                /* Implied check for (SD::FAT_32 == this->m_filesystem) */
                *value =
                        this->read_rev_dat32(
                                &this->m_fat[(fatEntry - firstAvailableAllocUnit)
                                        << 2]);
            // Clear the highest 4 bits - they are always reserved
            *value &= 0x0FFFFFFF;
#ifdef SD_OPTION_VERBOSE
            printf("\tReceived value: 0x%08x / %u\n", *value, *value);
#endif

            return 0;
        }

        /**
         * @brief       Find the next sector in the FAT, directory, or file.
         *              When it is found, load it into the appropriate global
         *              buffer
         *
         * @param[out]  *buf    Array of `SD_SECTOR_SIZE` bytes that can be
         *                      filled with the requested sector
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode load_next_sector (SD::Buffer *buf) {
#ifdef SD_OPTION_FILE_WRITE
            if (buf->mod)
                this->write_data_block(
                        buf->curClusterStartAddr + buf->curSectorOffset,
                        buf->buf);
#endif

            // Check for the end-of-chain marker (end of file)
            if (((uint32_t) SD::EOC_BEG) <= buf->nextAllocUnit)
                return SD::EOC_END;

            // Are we looking at the root directory of a FAT16 system?
            if (SD::FAT_16 == this->m_filesystem
                    && this->m_rootAddr == (buf->curClusterStartAddr)) {
                // Root dir of FAT16; Is it the last sector in the root
                // directory?
                if (this->m_rootDirSectors == (buf->curSectorOffset))
                    return SD::EOC_END;
                // Root dir of FAT16; Not last sector
                else
                    // Any error from reading the data block will be returned to
                    // calling function
                    return this->read_data_block(++(buf->curSectorOffset),
                            buf->buf);
            }
            // We are looking at a generic data cluster.
            else {
                // Generic data cluster; Have we reached the end of the cluster?
                if (((1 << this->m_sectorsPerCluster_shift) - 1)
                        > (buf->curSectorOffset)) {
                    // Generic data cluster; Not the end; Load next sector in
                    // the cluster

                    // Any error from reading the data block will be returned to
                    // calling function
                    return this->read_data_block(
                            ++(buf->curSectorOffset) + buf->curClusterStartAddr,
                            buf->buf);
                }
                // End of generic data cluster; Look through the FAT to find the
                // next cluster
                else
                    return this->inc_cluster(buf);
            }

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
            printf("New sector loaded:\n");
            this->print_hex_block(buf->buf, SD::SECTOR_SIZE);
            putchar('\n');
#endif

            return 0;
        }

        /**
         * @brief       Load a requested sector into the buffer independent of
         *              the current sector or cluster
         *
         * @param[out]  *f      Address of the sd_buffer object to be updated
         * @param[in]   offset  How many sectors past the first one should be
         *                      skipped (sector number of the file)
         *
         * @return      Returns 0 upon success, error code otherwise
         *
         */
        PropWare::ErrorCode load_sector_from_offset (SD::File *f,
                const uint32_t offset) {
            PropWare::ErrorCode err;
            uint32_t clusterOffset = offset >> this->m_sectorsPerCluster_shift;

#ifdef SD_OPTION_FILE_WRITE
            // If the buffer has been modified, write it before loading the next
            // sector
            if (f->buf->mod) {
                this->write_data_block(
                        f->buf->curClusterStartAddr + f->buf->curSectorOffset,
                        f->buf->buf);
                f->buf->mod = false;
            }
#endif

            // Find the correct cluster
            if (f->curCluster < clusterOffset) {
#ifdef SD_OPTION_VERBOSE
                printf("Need to fast-forward through the FAT to find the "
                        "cluster\n");
#endif
                // Desired cluster comes after the currently loaded one - this
                // is easy and requires continuing to look forward through the
                // FAT from the current position
                clusterOffset -= f->curCluster;
                while (clusterOffset--) {
                    ++(f->curCluster);
                    f->buf->curAllocUnit = f->buf->nextAllocUnit;
                    check_errors(
                            this->get_fat_value(f->buf->curAllocUnit,
                                    &(f->buf->nextAllocUnit)));
                }
                f->buf->curClusterStartAddr = this->find_sector_from_alloc(
                        f->buf->curAllocUnit);
            } else if (f->curCluster > clusterOffset) {
#ifdef SD_OPTION_VERBOSE
                printf("Need to backtrack through the FAT to find the "
                        "cluster\n");
#endif
                // Desired cluster is an earlier cluster than the currently
                // loaded one - this requires starting from the beginning and
                // working forward
                f->buf->curAllocUnit = f->firstAllocUnit;
                check_errors(
                        this->get_fat_value(f->buf->curAllocUnit,
                                &(f->buf->nextAllocUnit)));
                f->curCluster = 0;
                while (clusterOffset--) {
                    ++(f->curCluster);
                    f->buf->curAllocUnit = f->buf->nextAllocUnit;
                    check_errors(
                            this->get_fat_value(f->buf->curAllocUnit,
                                    &(f->buf->nextAllocUnit)));
                }
                f->buf->curClusterStartAddr = this->find_sector_from_alloc(
                        f->buf->curAllocUnit);
            }

            // Followed by finding the correct sector
            f->buf->curSectorOffset = offset
                    % (1 << this->m_sectorsPerCluster_shift);
            f->curSector = offset;
            this->read_data_block(
                    f->buf->curClusterStartAddr + f->buf->curSectorOffset,
                    f->buf->buf);

            return 0;
        }

        /**
         * @brief       Read the next sector from SD card into memory
         * @detailed    When the final sector of a cluster is finished,
         *              SDIncCluster can be called. The appropriate global
         *              variables will be set according (incremented or set by
         *              the FAT) and the first sector of the next cluster will
         *              be read into the desired buffer.
         *
         * @param[out]  *buf    Array of `SD_SECTOR_SIZE` bytes used to hold a
         *                      sector from the SD card
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode inc_cluster (SD::Buffer *buf) {
            PropWare::ErrorCode err;

#ifdef SD_OPTION_FILE_WRITE
            // If the sector has been modified, write it back to the SD card
            // before reading again
            if (buf->mod) {
                check_errors(
                        this->write_data_block(
                                buf->curClusterStartAddr + buf->curSectorOffset,
                                buf->buf));
            }
            buf->mod = false;
#endif

            // Update this->m_cur*
            if (((uint32_t) SD::EOC_BEG) <= buf->curAllocUnit
                    && ((uint32_t) SD::EOC_END) <= buf->curAllocUnit)
                return SD::READING_PAST_EOC;
            buf->curAllocUnit = buf->nextAllocUnit;
            // Only look ahead to the next allocation unit if the current alloc
            // unit is not EOC
            if (!(((uint32_t) SD::EOC_BEG) <= buf->curAllocUnit
                    && ((uint32_t) SD::EOC_END) <= buf->curAllocUnit))
                // Current allocation unit is not EOC, read the next one
                check_errors(
                        this->get_fat_value(buf->curAllocUnit,
                                &(buf->nextAllocUnit)));
            buf->curClusterStartAddr = this->find_sector_from_alloc(
                    buf->curAllocUnit);
            buf->curSectorOffset = 0;

#ifdef SD_OPTION_VERBOSE
            printf("Incrementing the cluster. New parameters are:\n");
            printf("\tCurrent allocation unit: 0x%08x / %u\n",
                    buf->curAllocUnit, buf->curAllocUnit);
            printf("\tNext allocation unit: 0x%08x / %u\n", buf->nextAllocUnit,
                    buf->nextAllocUnit);
            printf("\tCurrent cluster starting address: 0x%08x / %u\n",
                    buf->curClusterStartAddr, buf->curClusterStartAddr);
#endif

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
            check_errors(this->read_data_block(buf->curClusterStartAddr,
                            buf->buf));
            this->print_hex_block(buf->buf, SD::SECTOR_SIZE);
            return 0;
#else
            return this->read_data_block(buf->curClusterStartAddr, buf->buf);
#endif
        }

        /**
         * @brief       Read the standard length name of a file entry. If an
         *              extension exists, a period will be inserted before the
         *              extension. A null-terminator is always appended to the
         *              end
         *
         * @pre         *buf must point to the first byte in a FAT entry - no
         *              error checking is executed on buf
         * @pre         Errors may occur if at least 13 (8 + 1 + 3 + 1) bytes of
         *              memory are not allocated for filename
         *
         * @param[in]   *buf        First byte in local memory containing a FAT
         *                          entry
         * @param[out]  *filename   Address in memory where the filename string
         *                          will be stored
         */
        void get_filename (const uint8_t *buf, char *filename) {
            uint8_t i, j = 0;

            // Read in the first 8 characters - stop when a space is reached or
            // 8 characters have been read, whichever comes first
            for (i = 0; i < SD::FILE_NAME_LEN; ++i) {
                if (0x05 == buf[i])
                    filename[j++] = 0xe5;
                else if (' ' != buf[i])
                    filename[j++] = buf[i];
            }

            // Determine if there is more past the first 8 - Again, stop when a
            // space is reached
            if (' ' != buf[SD::FILE_NAME_LEN]) {
                filename[j++] = '.';
                for (i = SD::FILE_NAME_LEN;
                        i < SD::FILE_NAME_LEN + SD::FILE_EXTENSION_LEN; ++i) {
                    if (' ' != buf[i])
                        filename[j++] = buf[i];
                }
            }

            // Insert null-terminator
            filename[j] = 0;
        }

        /**
         * @brief       Find a file entry (file or sub-directory)
         *
         * @detailed    Find a file or directory that matches the name in
         *              *filename in the current directory; its relative
         *              location is communicated by placing it in the address of
         *              *fileEntryOffset
         *
         * @param[in]   *filename           C-string representing the short
         *                                  (standard) filename
         * @param[out]  *fileEntryOffset    The buffer offset will be returned
         *                                  via this address if the file is
         *                                  found
         *
         * @return      Returns 0 upon success, error code otherwise (common
         *              error code is SD_EOC_END for end-of-chain or
         *              file-not-found marker)
         */
        PropWare::ErrorCode find (const char *filename,
                uint16_t *fileEntryOffset) {
            PropWare::ErrorCode err;
            char readEntryName[SD::FILENAME_STR_LEN];

#ifdef SD_OPTION_FILE_WRITE
            // Save the current buffer
            if (this->m_buf.mod) {
                check_errors(
                        this->write_data_block(
                                this->m_buf.curClusterStartAddr
                                        + this->m_buf.curSectorOffset,
                                this->m_buf.buf));
                this->m_buf.mod = false;
            }
#endif

            *fileEntryOffset = 0;

            // If we aren't looking at the beginning of the directory cluster,
            // we must backtrack to the beginning and then begin listing files
            if (this->m_buf.curSectorOffset
                    || (this->find_sector_from_alloc(this->m_dir_firstAllocUnit)
                            != this->m_buf.curClusterStartAddr)) {
#ifdef SD_OPTION_VERBOSE
                printf("'find' requires a backtrack to beginning of "
                        "cluster\n");
#endif
                this->m_buf.curClusterStartAddr = this->find_sector_from_alloc(
                        this->m_dir_firstAllocUnit);
                this->m_buf.curSectorOffset = 0;
                this->m_buf.curAllocUnit = this->m_dir_firstAllocUnit;
                check_errors(
                        this->get_fat_value(this->m_buf.curAllocUnit,
                                &this->m_buf.nextAllocUnit));
                check_errors(
                        this->read_data_block(this->m_buf.curClusterStartAddr,
                                this->m_buf.buf));
            }
            this->m_buf.id = SD::FOLDER_ID;

            // Loop through all entries in the current directory until we find
            // the correct one
            // Function will exit normally with SD::EOC_END error code if the
            // file is not found
            while (this->m_buf.buf[*fileEntryOffset]) {
                // Check if file is valid, retrieve the name if it is
                if (!(SD::DELETED_FILE_MARK == this->m_buf.buf[*fileEntryOffset])) {
                    this->get_filename(&(this->m_buf.buf[*fileEntryOffset]),
                            readEntryName);
                    if (!strcmp(filename, readEntryName))
                        // File names match, return 0 to indicate a successful
                        // search
                        return 0;
                }

                // Increment to the next file
                *fileEntryOffset += SD::FILE_ENTRY_LENGTH;

                // If it was the last entry in this sector, proceed to the next
                // one
                if (SD::SECTOR_SIZE == *fileEntryOffset) {
                    // Last entry in the sector, attempt to load a new sector
                    // Possible error value includes end-of-chain marker
                    check_errors(this->load_next_sector(&this->m_buf));

                    *fileEntryOffset = 0;
                }
            }

            return FILENAME_NOT_FOUND;
        }

        /**
         * @brief       Reload the sector currently in use by a given file
         *
         * @param[in]   *f  Address of the file object requested
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode reload_buf (SD::File *f) {
            PropWare::ErrorCode err;

            // Function is only called if it has already been determined that
            // the buffer needs to be loaded - no checks need to be run

#ifdef SD_OPTION_FILE_WRITE
            // If the currently loaded buffer has been modified, save it
            if (f->buf->mod) {
                check_errors(
                        this->write_data_block(
                                f->buf->curClusterStartAddr
                                        + f->buf->curSectorOffset,
                                f->buf->buf));
                f->buf->mod = false;
            }
#endif

            // Set current values to show that the first sector of the file is
            // loaded. SDLoadSectorFromOffset() loads the sector unconditionally
            // before returning so we do not need to load the sector here
            f->buf->curAllocUnit = f->firstAllocUnit;
            f->buf->curClusterStartAddr = this->find_sector_from_alloc(
                    f->firstAllocUnit);
            f->buf->curSectorOffset = 0;
            check_errors(
                    this->get_fat_value(f->firstAllocUnit,
                            &(f->buf->nextAllocUnit)));

            // Proceed with loading the sector
            check_errors(this->load_sector_from_offset(f, f->curSector));
            f->buf->id = f->id;

            return 0;
        }

#ifdef SD_OPTION_FILE_WRITE
        /**
         * @brief       Find the first empty allocation unit in the FAT
         *
         * @detailed    The value of the first empty allocation unit is returned
         *              and its location will contain the end-of-chain marker,
         *              SD_EOC_END.
         *              NOTE: It is important to realize that, though the new
         *              entry now contains an EOC marker, this function does not
         *              know what cluster is being extended and therefore the
         *              calling function must modify the previous EOC to contain
         *              the return value
         *
         * @param[in]   restore     If non-zero, the original fat-sector will be
         *                          restored to m_fat before returning; if zero,
         *                          the last-used sector will remain loaded
         *
         * @return      Returns the number of the first unused allocation unit
         */
        uint32_t find_empty_space (const uint8_t restore) {
            uint16_t allocOffset = 0;
            uint32_t fatSectorAddr = this->m_curFatSector + this->m_fatStart;
            uint32_t retVal;
            // NOTE: this->m_curFatSector is not modified until end of function
            // - it is used throughout this function as the original starting
            // point

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
            printf("\n*** SDFindEmptySpace() initialized with FAT sector "
                    "0x%08x / %u loaded ***\n", this->m_curFatSector,
                    this->m_curFatSector);
            this->print_hex_block(this->m_fat, SD::SECTOR_SIZE);
#endif

            // Find the first empty allocation unit and write the EOC marker
            if (SD::FAT_16 == this->m_filesystem) {
                // Loop until we find an empty cluster
                while (this->read_rev_dat16(&(this->m_fat[allocOffset]))) {
#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
                    printf("Searching the following sector...\n");
                    this->print_hex_block(this->m_fat, SD::SECTOR_SIZE);
#endif
                    // Stop when we either reach the end of the current block or
                    // find an empty cluster
                    while (this->read_rev_dat16(&(this->m_fat[allocOffset]))
                            && (SD::SECTOR_SIZE > allocOffset))
                        allocOffset += SD::FAT_16;
                    // If we reached the end of a sector...
                    if (SD::SECTOR_SIZE <= allocOffset) {
                        // If the currently loaded FAT sector has been modified,
                        // save it
                        if (this->m_fatMod) {
#ifdef SD_OPTION_VERBOSE
                            printf("FAT sector has been modified; saving "
                                    "now... ");
#endif
                            this->write_data_block(this->m_curFatSector,
                                    this->m_fat);
                            this->write_data_block(
                                    this->m_curFatSector + this->m_fatSize,
                                    this->m_fat);
#ifdef SD_OPTION_VERBOSE
                            printf("done!\n");
#endif
                            this->m_fatMod = false;
                        }
                        // Read the next fat sector
#ifdef SD_OPTION_VERBOSE
                        printf("SDFindEmptySpace() is reading in sector "
                                "address: 0x%08x / %u\n", fatSectorAddr + 1,
                                fatSectorAddr + 1);
#endif
                        this->read_data_block(++fatSectorAddr, this->m_fat);
                    }
                }
                this->write_rev_dat16(this->m_fat + allocOffset,
                        (uint16_t) SD::EOC_END);
                this->m_fatMod = true;
            } else /* Implied: "if (SD::FAT_32 == this->m_filesystem)" */{
                // In FAT32, the first 7 usable clusters seem to be
                // un-officially reserved for the root directory
                if (0 == this->m_curFatSector)
                    allocOffset = 9 * this->m_filesystem;

                // Loop until we find an empty cluster
                while (this->read_rev_dat32(&(this->m_fat[allocOffset]))
                        & 0x0fffffff) {
#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
                    printf("Searching the following sector...\n");
                    this->print_hex_block(this->m_fat, SD::SECTOR_SIZE);
#endif
                    // Stop when we either reach the end of the current block or
                    // find an empty cluster
                    while ((this->read_rev_dat32(&(this->m_fat[allocOffset]))
                            & 0x0fffffff) && (SD::SECTOR_SIZE > allocOffset))
                        allocOffset += SD::FAT_32;

#ifdef SD_OPTION_VERBOSE
                    printf("Broke while loop... why? Offset = 0x%04x / %u\n",
                            allocOffset, allocOffset);
#endif
                    // If we reached the end of a sector...
                    if (SD::SECTOR_SIZE <= allocOffset) {
                        if (this->m_fatMod) {
#ifdef SD_OPTION_VERBOSE
                            printf("FAT sector has been modified; saving "
                                    "now... ");
#endif
                            this->write_data_block(
                                    this->m_curFatSector + this->m_fatStart,
                                    this->m_fat);
                            this->write_data_block(
                                    this->m_curFatSector + this->m_fatStart
                                            + this->m_fatSize, this->m_fat);
#ifdef SD_OPTION_VERBOSE
                            printf("done!\n");
#endif
                            this->m_fatMod = false;
                        }
                        // Read the next fat sector
#ifdef SD_OPTION_VERBOSE
                        printf("SDFindEmptySpace() is reading in sector "
                                "address: 0x%08x / %u\n", fatSectorAddr + 1,
                                fatSectorAddr + 1);
#endif
                        this->read_data_block(++fatSectorAddr, this->m_fat);
                        allocOffset = 0;
                    }
                }
                this->write_rev_dat32(&(this->m_fat[allocOffset]),
                        ((uint32_t) SD::EOC_END) & 0x0fffffff);
                this->m_fatMod = true;
            }

#ifdef SD_OPTION_VERBOSE
            printf("Available space found: 0x%08x / %u\n",
                    (this->m_curFatSector << this->m_entriesPerFatSector_Shift)
                    + allocOffset / this->m_filesystem,
                    (this->m_curFatSector << this->m_entriesPerFatSector_Shift)
                    + allocOffset / this->m_filesystem);
#endif

            // If we loaded a new fat sector (and then modified it directly
            // above), write the sector before re-loading the original
            if ((fatSectorAddr != (this->m_curFatSector + this->m_fatStart))
                    && this->m_fatMod) {
                this->write_data_block(fatSectorAddr, this->m_fat);
                this->write_data_block(fatSectorAddr + this->m_fatSize,
                        this->m_fat);
                this->m_fatMod = false;
                this->read_data_block(this->m_curFatSector + this->m_fatStart,
                        this->m_fat);
            } else
                this->m_curFatSector = fatSectorAddr - this->m_fatStart;

            // Return new address to end-of-chain
            retVal = this->m_curFatSector << this->m_entriesPerFatSector_Shift;
            retVal += allocOffset / this->m_filesystem;
            return retVal;
        }

        /**
         * @brief       Enlarge a file or directory by one cluster
         *
         * @param[in]   *buf    Address of the buffer (containing information
         *                      for a file or directory) to be enlarged
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode extend_fat (SD::Buffer *buf) {
            PropWare::ErrorCode err;
            uint32_t newAllocUnit;
#ifdef SD_OPTION_VERBOSE
            printf("Extending file or directory now...\n");
#endif

            // Do we need to load a different sector of the FAT or is the
            // correct one currently loaded? (Correct means the sector currently
            // containing the EOC marker)
            if ((buf->curAllocUnit >> this->m_entriesPerFatSector_Shift)
                    != this->m_curFatSector) {

#ifdef SD_OPTION_VERBOSE
                printf("Need new FAT sector. Loading: 0x%08x / %u\n",
                        buf->curAllocUnit >> this->m_entriesPerFatSector_Shift,
                        buf->curAllocUnit >> this->m_entriesPerFatSector_Shift);
                printf("... because the current allocation unit is: "
                        "0x%08x / %u\n",
                        buf->curAllocUnit, buf->curAllocUnit);
#endif
                // Need new sector, save the old one...
                if (this->m_fatMod) {
                    this->write_data_block(
                            this->m_curFatSector + this->m_fatStart,
                            this->m_fat);
                    this->write_data_block(
                            this->m_curFatSector + this->m_fatStart
                                    + this->m_fatSize, this->m_fat);
                    this->m_fatMod = false;
                }
                // And load the new one...
                this->m_curFatSector = buf->curAllocUnit
                        >> this->m_entriesPerFatSector_Shift;
                check_errors(
                        this->read_data_block(
                                this->m_curFatSector + this->m_fatStart,
                                this->m_fat));
            }

            // This function should only be called when a file or directory has
            // reached the end of its cluster chain
            uint16_t entriesPerFatSector = 1
                    << this->m_entriesPerFatSector_Shift;
            uint16_t allocUnitOffset = buf->curAllocUnit % entriesPerFatSector;
            uint16_t fatPointerAddress = allocUnitOffset * this->m_filesystem;
            uint32_t nxtSctr = this->read_rev_dat32(
                    &(this->m_fat[fatPointerAddress]));
            if ((uint32_t) SD::EOC_BEG <= nxtSctr)
                return SD::INVALID_FAT_APPEND;

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
            // Display the currently loaded FAT.... for no reason... not sure
            // why I wanted to do this...
            printf("This is the sector that *should* contain the EOC "
                    "marker...\n");
            this->print_hex_block(this->m_fat, SD::SECTOR_SIZE);
#endif

            // Find where the next cluster of the file should be stored...
            newAllocUnit = this->find_empty_space(1);

            // Now that we know the allocation unit, write it to the FAT buffer
            if (SD::FAT_16 == this->m_filesystem) {
                this->write_rev_dat16(
                        &(this->m_fat[(buf->curAllocUnit
                                % (1 << this->m_entriesPerFatSector_Shift))
                                * this->m_filesystem]),
                        (uint16_t) newAllocUnit);
            } else {
                this->write_rev_dat32(
                        &(this->m_fat[(buf->curAllocUnit
                                % (1 << this->m_entriesPerFatSector_Shift))
                                * this->m_filesystem]), newAllocUnit);
            }
            buf->nextAllocUnit = newAllocUnit;
            this->m_fatMod = true;  // And mark the buffer as modified

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
            printf("After modification, the FAT now looks like...\n");
            this->print_hex_block(this->m_fat, SD::SECTOR_SIZE);
#endif

            return 0;
        }

        /**
         * @brief       Allocate space for a new file
         *
         * @param[in]   *name               Character array for the new file
         * @param[in]   *fileEntryOffset    Offset from the currently loaded
         *                                  directory entry where the file's
         *                                  metadata should be written
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode create_file (const char *name,
                const uint16_t *fileEntryOffset) {
            uint8_t i, j;
            // *name is only checked for uppercase
            char uppercaseName[SD::FILENAME_STR_LEN];
            uint32_t allocUnit;

#ifdef SD_OPTION_VERBOSE
            printf("Creating new file: %s\n", name);
#endif

            // Parameter checking...
            if (SD::FILENAME_STR_LEN < strlen(name))
                return SD::INVALID_FILENAME;

            // Convert the name to uppercase
            for (i = 0; name[i]; ++i)
                if ('a' <= name[i] && 'z' >= name[i])
                    uppercaseName[i] = name[i] + 'A' - 'a';
                else
                    uppercaseName[i] = name[i];

            // Write the file fields in order...

            /* 1) Short file name */
            // Write first section
            for (i = 0; '.' != uppercaseName[i] && 0 != uppercaseName[i]; ++i)
                this->m_buf.buf[*fileEntryOffset + i] =
                        (uint8_t) uppercaseName[i];
            // Check if there is an extension
            if (uppercaseName[i]) {
                // There might be an extension - pad first name with spaces
                for (j = i; j < SD::FILE_NAME_LEN; ++j)
                    this->m_buf.buf[*fileEntryOffset + j] = ' ';
                // Check if there is a period, as one would expect for a file
                // name with an extension
                if ('.' == uppercaseName[i]) {
                    // Extension exists, write it
                    ++i;        // Skip the period
                    // Insert extension, character-by-character
                    for (j = SD::FILE_NAME_LEN; uppercaseName[i]; ++j)
                        this->m_buf.buf[*fileEntryOffset + j] =
                                (uint8_t) uppercaseName[i++];
                    // Pad extension with spaces
                    for (; j < SD::FILE_NAME_LEN + SD::FILE_EXTENSION_LEN; ++j)
                        this->m_buf.buf[*fileEntryOffset + j] = ' ';
                }
                // If it wasn't a period or null terminator, throw an error
                else
                    return SD::INVALID_FILENAME;
            }
            // No extension, pad with spaces
            else
                for (; i < (SD::FILE_NAME_LEN + SD::FILE_EXTENSION_LEN); ++i)
                    this->m_buf.buf[*fileEntryOffset + i] = ' ';

            /* 2) Write attribute field... */
            // TODO: Allow for file attribute flags to be set, such as
            //       SD::READ_ONLY, SD::SUB_DIR, etc
            // Archive flag should be set because the file is new
            this->m_buf.buf[*fileEntryOffset + SD::FILE_ATTRIBUTE_OFFSET] =
                    SD::ARCHIVE;
            this->m_buf.mod = true;

#ifdef SD_OPTION_VERBOSE
            SD::print_file_entry(&(this->m_buf.buf[*fileEntryOffset]),
                    uppercaseName);
#endif

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
            SD::print_hex_block(this->m_buf.buf, SD::SECTOR_SIZE);
#endif

            /**
             * 3) Find a spot in the FAT (do not check for a full FAT, assume
             *    space is available)
             */
            allocUnit = this->find_empty_space(0);
            this->write_rev_dat16(
                    &(this->m_buf.buf[*fileEntryOffset
                            + SD::FILE_START_CLSTR_LOW]), (uint16_t) allocUnit);
            if (SD::FAT_32 == this->m_filesystem)
                this->write_rev_dat16(
                        &(this->m_buf.buf[*fileEntryOffset
                                + SD::FILE_START_CLSTR_HIGH]),
                        (uint16_t) (allocUnit >> 16));

            /* 4) Write the size of the file (currently 0) */
            this->write_rev_dat32(
                    &(this->m_buf.buf[*fileEntryOffset + SD::FILE_LEN_OFFSET]),
                    0);

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
            printf("New file entry at offset 0x%08x / %u looks like...\n",
                    *fileEntryOffset, *fileEntryOffset);
            SD::print_hex_block(this->m_buf.buf, SD::SECTOR_SIZE);
#endif

            this->m_buf.mod = true;

            return 0;
        }
#endif

#if (defined SD_OPTION_SHELL || defined SD_OPTION_VERBOSE)
        /**
         * @brief       Print the attributes and name of a file entry
         *
         * @param[in]   *fileEntry  Address of the first byte of the file entry
         * @param[out]  *filename   Allocated space for the filename string to
         *                          be stored
         */
        void print_file_entry (const uint8_t *fileEntry, char filename[]) {
            print_file_attributes(fileEntry[SD::FILE_ATTRIBUTE_OFFSET]);
            this->get_filename(fileEntry, filename);
            printf("\t\t%s", filename);
            if (SD::SUB_DIR & fileEntry[SD::FILE_ATTRIBUTE_OFFSET])
                putchar((int) '/');
            putchar('\n');
        }

        /**
         * @brief       Print attributes of a file entry
         *
         * @param[in]
         */
        void print_file_attributes (const uint8_t flags) {
            // Print file attributes
            if (SD::READ_ONLY & flags)
                putchar(SD::READ_ONLY_CHAR);
            else
                putchar(SD::READ_ONLY_CHAR_);

            if (SD::HIDDEN_FILE & flags)
                putchar(SD::HIDDEN_FILE_CHAR);
            else
                putchar(SD::HIDDEN_FILE_CHAR_);

            if (SD::SYSTEM_FILE & flags)
                putchar(SD::SYSTEM_FILE_CHAR);
            else
                putchar(SD::SYSTEM_FILE_CHAR_);

            if (SD::VOLUME_ID & flags)
                putchar(SD::VOLUME_ID_CHAR);
            else
                putchar(SD::VOLUME_ID_CHAR_);

            if (SD::SUB_DIR & flags)
                putchar(SD::SUB_DIR_CHAR);
            else
                putchar(SD::SUB_DIR_CHAR_);

            if (SD::ARCHIVE & flags)
                putchar(SD::ARCHIVE_CHAR);
            else
                putchar(SD::ARCHIVE_CHAR_);
        }
#endif

        /**
         * @brief   Print to screen each status bit individually with
         *          human-readable descriptions
         */
        void first_byte_expansion () const {
            if (BIT_0 & this->m_firstByteResponse)
                printf("\t0: Idle\n");
            if (BIT_1 & this->m_firstByteResponse)
                printf("\t1: Erase reset\n");
            if (BIT_2 & this->m_firstByteResponse)
                printf("\t2: Illegal command\n");
            if (BIT_3 & this->m_firstByteResponse)
                printf("\t3: Communication CRC error\n");
            if (BIT_4 & this->m_firstByteResponse)
                printf("\t4: Erase sequence error\n");
            if (BIT_5 & this->m_firstByteResponse)
                printf("\t5: Address error\n");
            if (BIT_6 & this->m_firstByteResponse)
                printf("\t6: Parameter error\n");
            if (BIT_7 & this->m_firstByteResponse)
                printf("\t7: Something is really screwed up. This should "
                        "always be 0.\n");
        }

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
        static const uint32_t SEND_ACTIVE_TIMEOUT;
        static const uint32_t SINGLE_BYTE_WIGGLE_ROOM;
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
#define SD_FILENAME_STR_LEN     SD_FILE_NAME_LEN + SD_FILE_EXTENSION_LEN + 2
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

    private:
        /*******************************
         *** Private Member Variable ***
         *******************************/
        SPI *m_spi;
        PropWare::Pin m_cs;  // Chip select pin mask
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

#endif /* PROPWARE_SD_H_ */
