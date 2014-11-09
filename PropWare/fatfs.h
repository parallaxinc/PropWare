/**
 * @file        fatfs.h
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

#pragma once

#include <PropWare/PropWare.h>
#include <PropWare/blockstorage.h>
#include <PropWare/printer/printer.h>
#include <PropWare/filesystem.h>
#include <PropWare/fatfile.h>

namespace PropWare {

/**
 * FAT 16/32 filesystem driver - can be used with SD cards or any other
 * PropWare::BlockStorage device
 */
class FatFS : public Filesystem {
    public:
#define HD44780_MAX_ERROR    64
        typedef enum {
                                   NO_ERROR  = 0,
                                   BEG_ERROR = HD44780_MAX_ERROR + 1,
            /** FatFS Error 1 */   EMPTY_FAT_ENTRY,
            /** FatFS Error 2 */   INVALID_PTR_ORIGIN,
            /** FatFS Error 3 */   INVALID_FAT_APPEND,
            /** FatFS Error 4 */   TOO_MANY_FATS,
            /** FatFS Error 5 */   READING_PAST_EOC,
            /** FatFS Error 6 */   FILE_WITHOUT_BUFFER,
            /** FatFS Error 7 */   INVALID_FILESYSTEM,
            /** Last FatFS error */END_ERROR = INVALID_FILESYSTEM
        } ErrorCode;

        class FatBuffer;

    public:
        FatFS (BlockStorage *driver) {
            this->m_driver     = driver;
            this->m_mounted    = false;
            this->m_error      = NO_ERROR;
            this->m_fatMod     = false;
            this->m_nextFileId = 0;
            this->m_buf.buf    = NULL;
            this->m_fat        = NULL;
        }

        ~FatFS () {
            if (NULL != this->m_buf.buf)
                free(this->m_buf.buf);

            if (NULL != this->m_fat)
                free(this->m_fat);
        }

        /**
         * @see PropWare::Filesystem::mount
         *
         * @note    Does not yet support multiple partitions
         */
        PropWare::ErrorCode mount (const uint8_t partition = 0) {
            PropWare::ErrorCode err;

            if (this->m_mounted)
                return Filesystem::FILESYSTEM_ALREADY_MOUNTED;

            // Start the driver
            check_errors(this->m_driver->start());
            this->m_fatMod = false;
            this->m_nextFileId = 0;

            FatFS::InitFATInfo fatInfo;

            // Allocate the buffers
            if (NULL == this->m_buf.buf)
                this->m_buf.buf =
                        (uint8_t *) malloc(this->m_driver->get_sector_size());
            if (NULL == this->m_fat)
                this->m_fat =
                        (uint8_t *) malloc(this->m_driver->get_sector_size());

            check_errors(this->read_boot_sector(&fatInfo));
            check_errors(this->common_boot_sector_parser(&fatInfo));
            this->partition_info_parser(&fatInfo);
            check_errors(this->determine_fat_type(&fatInfo));
            this->store_root_info(&fatInfo);
            check_errors(this->read_fat_and_root_sectors());

            this->m_mounted = true;

            return 0;
        }

        PropWare::ErrorCode unmount () {
            // TODO: Lots more to do here!

            if (this->m_mounted) {
                if (NULL != this->m_buf.buf) {
                    free(this->m_buf.buf);
                    this->m_buf.buf = NULL;
                }

                if (NULL != this->m_fat) {
                    free(this->m_fat);
                    this->m_fat = NULL;
                }
            }

            return NO_ERROR;
        }

/*        File* fopen (const char *name, const char mode[], uint8_t *buf = 0) {
            PropWare::ErrorCode err;
            uint16_t            fileEntryOffset = 0;

            // If no file was passed, create one
            FatFile *f = (FatFile *) malloc(sizeof(FatFile));

            // Attach the buffer to the file
            if (NULL == buf)
                f->buf->buf =
                        (uint8_t *) malloc(this->m_driver->get_sector_size());
            else
                f->buf->buf = buf;

            Filesystem::set_file_id(f, this->m_nextFileId++);
            Filesystem::set_file_rPtr(f, 0);
            Filesystem::set_file_wPtr(f, 0);
            Filesystem::set_file_mode(f, File::get_mode(mode));

            // Attempt to find the file
            if ((err = this->find(name, &fileEntryOffset))) {
                // Find returned an error; ensure it was EOC...
                if (FatFS::EOC_END == err) {
                    // And return a FILE_NOT_FOUND error if using read only mode
                    if (File::READ == Filesystem::get_file_mode(f)) {
                        this->m_error = Filesystem::FILENAME_NOT_FOUND;
                        return NULL;
                    }
                    // Or create the file for any other mode
                    else {
                        // File wasn't found and the cluster is full; add another
                        // to the directory
                        check_fs_error(this->extend_fat(&this->m_buf));
                        check_fs_error(this->load_next_sector(&this->m_buf));
                    }
                }
                if (FatFS::EOC_END == err || FatFS::FILENAME_NOT_FOUND == err) {
                    // File wasn't found, but there is still room in this
                    // cluster (or a new cluster was just added)
                    check_fs_error(this->create_file(name, &fileEntryOffset));
                }
                // SD::find returned unknown error - throw it
                else {
                    this->m_error = err;
                    return NULL;
                }
            }

            // `name` was found successfully, determine if it is a file or
            // directory
            if (FatFS::SUB_DIR
                    & this->m_buf.buf[fileEntryOffset
                    + FatFS::FILE_ATTRIBUTE_OFFSET])
                return Filesystem::ENTRY_NOT_FILE;

            // Passed the file-not-directory test, load it into the buffer and
            // update status variables
            f->buf->id = Filesystem::get_file_id(f);
            f->curSector = 0;
            if (FatFS::FAT_16 == this->m_filesystem)
                f->buf->curAllocUnit = this->read_rev_dat16(
                        &(this->m_buf.buf[fileEntryOffset
                                + FatFS::FILE_START_CLSTR_LOW]));
            else {
                f->buf->curAllocUnit = this->read_rev_dat16(
                        &(this->m_buf.buf[fileEntryOffset
                                + FatFS::FILE_START_CLSTR_LOW]));
                f->buf->curAllocUnit |= this->read_rev_dat16(
                        &(this->m_buf.buf[fileEntryOffset
                                + FatFS::FILE_START_CLSTR_HIGH])) << 16;

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
            check_fs_error(
                    this->get_fat_value(f->buf->curAllocUnit,
                                        &(f->buf->nextAllocUnit)));
            f->buf->curSectorOffset = 0;
            Filesystem::set_file_length(f, this->read_rev_dat32(
                    &(this->m_buf.buf[
                            fileEntryOffset + FatFS::FILE_LEN_OFFSET])));
            // Determine the number of sectors currently allocated to this file;
            // useful in the case that the file needs to be extended
            f->maxSectors = Filesystem::get_file_length(f) >>
                    BlockStorage::SECTOR_SIZE_SHIFT;
            if (!(f->maxSectors))
                f->maxSectors = (uint32_t) (1 << this->m_sectorsPerCluster_shift);
            while (f->maxSectors % (1 << this->m_sectorsPerCluster_shift))
                ++(f->maxSectors);
            f->buf->mod = false;
            check_fs_error(
                    this->read_data_block(f->buf->curClusterStartAddr,
                                          f->buf->buf));

            return 0;
        }*/

    private:
        // Boot sector addresses/values
        static const uint8_t  FAT_16              = 2;  // A FAT entry in FAT16 is 2-bytes
        static const uint8_t  FAT_32              = -4;  // A FAT entry in FAT32 is 4-bytes
        static const uint8_t  BOOT_SECTOR_ID      = 0xEB;
        static const uint8_t  BOOT_SECTOR_ID_ADDR = 0;
        static const uint16_t BOOT_SECTOR_BACKUP  = 0x1C6;
        static const uint8_t  CLUSTER_SIZE_ADDR   = 0x0D;
        static const uint8_t  RSVD_SCTR_CNT_ADDR  = 0x0E;
        static const uint8_t  NUM_FATS_ADDR       = 0x10;
        static const uint8_t  ROOT_ENTRY_CNT_ADDR = 0x11;
        static const uint8_t  TOT_SCTR_16_ADDR    = 0x13;
        static const uint8_t  FAT_SIZE_16_ADDR    = 0x16;
        static const uint8_t  TOT_SCTR_32_ADDR    = 0x20;
        static const uint8_t  FAT_SIZE_32_ADDR    = 0x24;
        static const uint8_t  ROOT_CLUSTER_ADDR   = 0x2c;
        static const uint16_t FAT12_CLSTR_CNT     = 4085;
        static const uint16_t FAT16_CLSTR_CNT     = 65525;

        // FAT file/directory values
        static const uint8_t FILE_ENTRY_LENGTH     = 32;  // An entry in a directory uses 32 bytes
        static const uint8_t DELETED_FILE_MARK     = 0xE5;  // Marks that a file has been deleted here, continue to the next entry
        static const uint8_t FILE_NAME_LEN         = 8;  // 8 characters in the standard file name
        static const uint8_t FILE_EXTENSION_LEN    = 3;  // 3 character file name extension
        static const uint8_t FILENAME_STR_LEN      = FILE_NAME_LEN + FILE_EXTENSION_LEN + 2;
        static const uint8_t FILE_ATTRIBUTE_OFFSET = 0x0B;  // Byte of a file entry to store attribute flags
        static const uint8_t FILE_START_CLSTR_LOW  = 0x1A;  // Starting cluster number
        static const uint8_t FILE_START_CLSTR_HIGH = 0x14;  // High word (16-bits) of the starting cluster number (FAT32 only)
        static const uint8_t FILE_LEN_OFFSET       = 0x1C;  // Length of a file in bytes
        static const int8_t  FREE_CLUSTER          = 0;  // Cluster is unused
        static const int8_t  RESERVED_CLUSTER      = 1;
        static const int8_t  RSVD_CLSTR_VAL_BEG    = -15;  // First reserved cluster value
        static const int8_t  RSVD_CLSTR_VAL_END    = -9;  // Last reserved cluster value
        static const int8_t  BAD_CLUSTER           = -8;  // Cluster is corrupt
        static const int32_t EOC_BEG               = -7;  // First marker for end-of-chain (end of file entry within FAT)
        static const int32_t EOC_END               = -1;  // Last marker for end-of-chain

        // FAT file attributes (definitions with trailing underscore represent character for a cleared attribute flag)
        static const uint8_t READ_ONLY         = BIT_0;
        static const char    READ_ONLY_CHAR    = 'r';
        static const char    READ_ONLY_CHAR_   = 'w';
        static const uint8_t HIDDEN_FILE       = BIT_1;
        static const char    HIDDEN_FILE_CHAR  = 'h';
        static const char    HIDDEN_FILE_CHAR_ = '.';
        static const uint8_t SYSTEM_FILE       = BIT_2;
        static const char    SYSTEM_FILE_CHAR  = 's';
        static const char    SYSTEM_FILE_CHAR_ = '.';
        static const uint8_t VOLUME_ID         = BIT_3;
        static const char    VOLUME_ID_CHAR    = 'v';
        static const char    VOLUME_ID_CHAR_   = '.';
        static const uint8_t SUB_DIR           = BIT_4;
        static const char    SUB_DIR_CHAR      = 'd';
        static const char    SUB_DIR_CHAR_     = 'f';
        static const uint8_t ARCHIVE           = BIT_5;
        static const char    ARCHIVE_CHAR      = 'a';
        static const char    ARCHIVE_CHAR_     = '.';

    private:
        typedef struct {
            public:
                uint8_t  numFATs;
                uint32_t rsvdSectorCount;
                uint32_t rootEntryCount;
                uint32_t totalSectors;
                uint32_t FATSize;
                uint32_t dataSectors;
                uint32_t bootSector;
                uint32_t clusterCount;
        } InitFATInfo;

        /**
         * @brief       TODO: What's this do?
         *
         * @param[out]  fatInfo     Store information about the FAT here
         *
         * @return 0 upon success, error code otherwise
         */
        inline PropWare::ErrorCode read_boot_sector (InitFATInfo *fatInfo) {
            PropWare::ErrorCode err;
            // Read in first sector (and use default buffer)
            check_errors(
                    this->m_driver->read_data_block(0, this->m_buf.buf));
            // Check if sector 0 is boot sector or MBR; if MBR, skip to boot
            // sector at first partition
            if (FatFS::BOOT_SECTOR_ID
                    != this->m_driver->get_byte(FatFS::BOOT_SECTOR_ID_ADDR,
                                                this->m_buf.buf)) {
                fatInfo->bootSector = this->m_driver->get_long(
                        FatFS::BOOT_SECTOR_BACKUP, this->m_buf.buf);
                check_errors(
                        this->m_driver->read_data_block(fatInfo->bootSector,
                                                        this->m_buf.buf));
            } else
                fatInfo->bootSector = 0;

            return 0;
        }

        inline PropWare::ErrorCode common_boot_sector_parser (
                InitFATInfo *fatInfo) {
            uint8_t temp;

            // Determine number of sectors per cluster
            temp = this->m_driver->get_byte(FatFS::CLUSTER_SIZE_ADDR,
                                            this->m_buf.buf);
#ifdef SD_OPTION_VERBOSE
            pwOut.printf("Preliminary sectors per cluster: %u" CRLF, temp);
#endif
            this->m_sectorsPerCluster_shift = 0;
            while (temp) {
                temp >>= 1;
                ++this->m_sectorsPerCluster_shift;
            }
            --this->m_sectorsPerCluster_shift;

            // Get the reserved sector count
            fatInfo->rsvdSectorCount = this->m_driver->get_short(
                    FatFS::RSVD_SCTR_CNT_ADDR, this->m_buf.buf);

            // Total number of FATs
            fatInfo->numFATs = this->m_driver->get_byte(FatFS::NUM_FATS_ADDR,
                                                        this->m_buf.buf);
#ifdef SD_OPTION_FILE_WRITE
            if (2 != fatInfo->numFATs)
            return FatFS::TOO_MANY_FATS;
#endif

            // Number of entries in the root directory
            fatInfo->rootEntryCount = this->m_driver->get_short(
                    FatFS::ROOT_ENTRY_CNT_ADDR, this->m_buf.buf);

            return 0;
        }

        inline void partition_info_parser (InitFATInfo *fatInfo) {
            // Check if FAT size is valid in 16- or 32-bit location
            fatInfo->FATSize = this->m_driver->get_short(
                    FatFS::FAT_SIZE_16_ADDR, this->m_buf.buf);
            if (!(fatInfo->FATSize))
                fatInfo->FATSize = this->m_driver->get_long(
                        FatFS::FAT_SIZE_32_ADDR, this->m_buf.buf);

            // Check if FAT16 total sectors is valid
            fatInfo->totalSectors = this->m_driver->get_short(
                    FatFS::TOT_SCTR_16_ADDR, this->m_buf.buf);
            if (!(fatInfo->totalSectors))
                fatInfo->totalSectors = this->m_driver->get_long(
                        FatFS::TOT_SCTR_32_ADDR, this->m_buf.buf);

            // Compute necessary numbers to determine FAT type (12/16/32)
            this->m_rootDirSectors = (fatInfo->rootEntryCount * 32)
                    >> this->m_driver->get_sector_size_shift();
            fatInfo->dataSectors = fatInfo->totalSectors
                    - (fatInfo->rsvdSectorCount
                            + fatInfo->numFATs * fatInfo->FATSize
                            + fatInfo->rootEntryCount);
            fatInfo->clusterCount = fatInfo->dataSectors
                    >> this->m_sectorsPerCluster_shift;

#ifdef SD_OPTION_VERBOSE
            pwOut.printf("Sectors per cluster: %u" CRLF,
                    1 << this->m_sectorsPerCluster_shift);
            pwOut.printf("Reserved sector count: 0x%08X / %u" CRLF,
                    fatInfo->rsvdSectorCount, fatInfo->rsvdSectorCount);
            pwOut.printf("Number of FATs: 0x%02X / %u" CRLF, fatInfo->numFATs,
                    fatInfo->numFATs);
            pwOut.printf("Total sector count: 0x%08X / %u" CRLF,
                    fatInfo->totalSectors,
                    fatInfo->totalSectors);
            pwOut.printf("Total cluster count: 0x%08X / %u" CRLF,
                    fatInfo->clusterCount,
                    fatInfo->clusterCount);
            pwOut.printf("Total data sectors: 0x%08X / %u" CRLF, fatInfo->dataSectors,
                    fatInfo->dataSectors);
            pwOut.printf("FAT Size: 0x%04x / %u" CRLF, fatInfo->FATSize,
                    fatInfo->FATSize);
            pwOut.printf("Root directory sectors: 0x%08X / %u" CRLF,
                    this->m_rootDirSectors, this->m_rootDirSectors);
            pwOut.printf("Root entry count: 0x%08X / %u" CRLF, fatInfo->rootEntryCount,
                    fatInfo->rootEntryCount);
#endif
        }

        inline PropWare::ErrorCode determine_fat_type (InitFATInfo *fatInfo) {
            // Determine and store FAT type
            if (FatFS::FAT12_CLSTR_CNT > fatInfo->clusterCount)
                return FatFS::INVALID_FILESYSTEM;
            else if (FatFS::FAT16_CLSTR_CNT > fatInfo->clusterCount) {
#ifdef SD_OPTION_VERBOSE
                pwOut.printf("\n***FAT type is FAT16***" CRLF);
#endif
                this->m_filesystem                = FatFS::FAT_16;
                this->m_entriesPerFatSector_Shift = 8;
            } else {
#ifdef SD_OPTION_VERBOSE
                pwOut.printf("\n***FAT type is FAT32***" CRLF);
#endif
                this->m_filesystem                = FatFS::FAT_32;
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
                case FatFS::FAT_16:
                    this->m_rootAddr      = fatInfo->FATSize * fatInfo->numFATs
                            + this->m_fatStart;
                    this->m_firstDataAddr = this->m_rootAddr
                            + this->m_rootDirSectors;
                    break;
                case FatFS::FAT_32:
                    this->m_firstDataAddr = this->m_rootAddr =
                            fatInfo->bootSector + fatInfo->rsvdSectorCount
                                    + fatInfo->FATSize * fatInfo->numFATs;
                    this->m_rootAllocUnit = this->m_driver->get_long(
                            FatFS::ROOT_CLUSTER_ADDR, this->m_buf.buf);
                    break;
            }

#ifdef SD_OPTION_FILE_WRITE
            // If files will be writable, the second FAT must also be updated -
            // the first sector's address is stored here
            this->m_fatSize = fatInfo->FATSize;
#endif

#ifdef SD_OPTION_VERBOSE
            pwOut.printf("Start of FAT: 0x%08X" CRLF, this->m_fatStart);
            pwOut.printf("Root directory alloc. unit: 0x%08X" CRLF,
                    this->m_rootAllocUnit);
            pwOut.printf("Root directory sector: 0x%08X" CRLF, this->m_rootAddr);
            pwOut.printf("Calculated root directory sector: 0x%08X" CRLF,
                    this->find_sector_from_alloc(this->m_rootAllocUnit));
            pwOut.printf("First data sector: 0x%08X" CRLF, this->m_firstDataAddr);
#endif
        }

        inline PropWare::ErrorCode read_fat_and_root_sectors () {
            PropWare::ErrorCode err;

            // Store the first sector of the FAT
            check_errors(
                    this->m_driver->read_data_block(this->m_fatStart,
                                                    this->m_fat));
            this->m_curFatSector = 0;

            // Print FAT if desired
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_VERBOSE_BLOCKS)
            pwOut.printf("\n***First File Allocation Table***" CRLF);
            this->print_hex_block(this->m_fat, SD::SECTOR_SIZE);
            pwOut.put_char('\n');
#endif

            // Read in the root directory, set root as current
            check_errors(
                    this->m_driver->read_data_block(this->m_rootAddr,
                                                    this->m_buf.buf));
            this->m_buf.curClusterStartAddr = this->m_rootAddr;
            if (FatFS::FAT_16 == this->m_filesystem) {
                this->m_dir_firstAllocUnit = (uint32_t) -1;
                this->m_buf.curAllocUnit   = (uint32_t) -1;
            } else {
                this->m_buf.curAllocUnit = this->m_dir_firstAllocUnit =
                        this->m_rootAllocUnit;
                check_errors(
                        this->get_fat_value(this->m_buf.curAllocUnit,
                                            &this->m_buf.nextAllocUnit));
            }
            this->m_buf.curClusterStartAddr = this->m_rootAddr;
            this->m_buf.curSectorOffset     = 0;

            // Print root directory
#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
            pwOut.printf("***Root directory***" CRLF);
            this->print_hex_block(this->m_buf.buf, SD::SECTOR_SIZE);
            pwOut.put_char('\n');
#endif

            return 0;
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
            uint32_t            firstAvailableAllocUnit;

            // Do we need to load a new fat sector?
            if ((fatEntry >> this->m_entriesPerFatSector_Shift)
                    != this->m_curFatSector) {
                // If the currently loaded FAT sector has been modified, save it
                if (this->m_fatMod) {
                    this->m_driver->write_data_block(
                            this->m_curFatSector + this->m_fatStart,
                            this->m_fat);
                    this->m_driver->write_data_block(
                            this->m_curFatSector + this->m_fatStart
                            + this->m_fatSize, this->m_fat);
                    this->m_fatMod = false;
                }
                // Need new sector, load it
                this->m_curFatSector = fatEntry
                        >> this->m_entriesPerFatSector_Shift;
                check_errors(
                        this->m_driver->read_data_block(
                                this->m_curFatSector + this->m_fatStart,
                                this->m_fat));
            }
            firstAvailableAllocUnit = this->m_curFatSector
                    << this->m_entriesPerFatSector_Shift;

            // The necessary FAT sector has been loaded and the next allocation
            // unit is known, proceed with loading the next data sector and
            // incrementing the cluster variables

            // Retrieve the next allocation unit number
            if (FatFS::FAT_16 == this->m_filesystem)
                *value = this->m_driver->get_short(
                        (fatEntry - firstAvailableAllocUnit) << 1, this->m_fat);
            else
                /* Implied check for (SD::FAT_32 == this->m_filesystem) */
                *value = this->m_driver->get_long(
                        (fatEntry - firstAvailableAllocUnit) << 2, this->m_fat);
            // Clear the highest 4 bits - they are always reserved
            *value &= 0x0FFFFFFF;

            return 0;
        }

        /**
         * @brief       Find a file entry (file or sub-directory)
         *
         * Find a file or directory that matches the name in *filename in the
         * current directory; its relative location is communicated by
         * placing it in the address of *fileEntryOffset
         *
         * @param[out]  *fileEntryOffset    The buffer offset will be returned
         *                                  via this address if the file is
         *                                  found
         * @param[in]   *filename           C-string representing the short
         *                                  (standard) filename
         *
         * @return      Returns 0 upon success, error code otherwise (common
         *              error code is SD_EOC_END for end-of-chain or
         *              file-not-found marker)
         */
        PropWare::ErrorCode find (uint16_t *fileEntryOffset,
                                  const char *filename) {
            PropWare::ErrorCode err;
            char readEntryName[FatFS::FILENAME_STR_LEN];

                // Save the current buffer
                if (this->m_buf.mod) {
                    check_errors(
                            this->m_driver->write_data_block(
                                    this->m_buf.curClusterStartAddr
                                            + this->m_buf.curSectorOffset,
                                    this->m_buf.buf));
                    this->m_buf.mod = false;
                }

            *fileEntryOffset = 0;

            // If we aren't looking at the beginning of the directory cluster,
            // we must backtrack to the beginning and then begin listing files
            if (this->m_buf.curSectorOffset
                    || (this->find_sector_from_alloc(this->m_dir_firstAllocUnit)
                    != this->m_buf.curClusterStartAddr)) {
                this->m_buf.curClusterStartAddr = this->find_sector_from_alloc(
                        this->m_dir_firstAllocUnit);
                this->m_buf.curSectorOffset = 0;
                this->m_buf.curAllocUnit = this->m_dir_firstAllocUnit;
                check_fs_error(
                        this->get_fat_value(this->m_buf.curAllocUnit,
                                            &this->m_buf.nextAllocUnit));
                check_fs_error(
                        this->m_driver->read_data_block(
                                this->m_buf.curClusterStartAddr, this->m_buf.buf));
            }
            this->m_buf.id = File::FOLDER_ID;

            // Loop through all entries in the current directory until we find
            // the correct one
            // Function will exit normally with SD::EOC_END error code if the
            // file is not found
            while (this->m_buf.buf[*fileEntryOffset]) {
                // Check if file is valid, retrieve the name if it is
                if (FatFS::DELETED_FILE_MARK != this->m_buf.buf[*fileEntryOffset]) {
                    this->get_filename(&(this->m_buf.buf[*fileEntryOffset]),
                                       readEntryName);
                    if (!strcmp(filename, readEntryName))
                        // File names match, return 0 to indicate a successful
                        // search
                        return 0;
                }

                // Increment to the next file
                *fileEntryOffset += FatFS::FILE_ENTRY_LENGTH;

                // If it was the last entry in this sector, proceed to the next
                // one
                if (this->m_driver->get_sector_size() == *fileEntryOffset) {
                    // Last entry in the sector, attempt to load a new sector
                    // Possible error value includes end-of-chain marker
                    check_fs_error(this->load_next_sector(&this->m_buf));

                    *fileEntryOffset = 0;
                }
            }

            return FILENAME_NOT_FOUND;
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
            if (FatFS::FAT_32 == this->m_filesystem)
                allocUnit -= this->m_rootAllocUnit;
            else
                allocUnit -= 2;
            allocUnit <<= this->m_sectorsPerCluster_shift;
            allocUnit += this->m_firstDataAddr;
            return allocUnit;
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
            for (i = 0; i < FatFS::FILE_NAME_LEN; ++i) {
                if (0x05 == buf[i])
                    filename[j++] = (char) 0xe5;
                else if (' ' != buf[i])
                    filename[j++] = buf[i];
            }

            // Determine if there is more past the first 8 - Again, stop when a
            // space is reached
            if (' ' != buf[FatFS::FILE_NAME_LEN]) {
                filename[j++] = '.';
                for (i = FatFS::FILE_NAME_LEN;
                     i < FatFS::FILE_NAME_LEN + FatFS::FILE_EXTENSION_LEN; ++i) {
                    if (' ' != buf[i])
                        filename[j++] = buf[i];
                }
            }

            // Insert null-terminator
            filename[j] = 0;
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
        PropWare::ErrorCode load_next_sector (FatFS::FatBuffer *buf) {
                if (buf->mod)
                    this->m_driver->write_data_block(
                            buf->curClusterStartAddr + buf->curSectorOffset,
                            buf->buf);

            // Check for the end-of-chain marker (end of file)
            if (((uint32_t) FatFS::EOC_BEG) <= buf->nextAllocUnit)
                return FatFS::EOC_END;

            // Are we looking at the root directory of a FAT16 system?
            if (FatFS::FAT_16 == this->m_filesystem
                    && this->m_rootAddr == (buf->curClusterStartAddr)) {
                // Root dir of FAT16; Is it the last sector in the root
                // directory?
                if (this->m_rootDirSectors == (buf->curSectorOffset))
                    return FatFS::EOC_END;
                    // Root dir of FAT16; Not last sector
                else
                    // Any error from reading the data block will be returned to
                    // calling function
                    return this->m_driver->read_data_block(++(buf->curSectorOffset),
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
                    return this->m_driver->read_data_block(
                            ++(buf->curSectorOffset) + buf->curClusterStartAddr,
                            buf->buf);
                }
                    // End of generic data cluster; Look through the FAT to find the
                    // next cluster
                else
                    return this->inc_cluster(buf);
            }

            return 0;
        }


        /**
         * @brief       Read the next sector from SD card into memory
         *
         * When the final sector of a cluster is finished,
         * SDIncCluster can be called. The appropriate global variables will
         * be set according (incremented or set by the FAT) and the first
         * sector of the next cluster will be read into the desired buffer.
         *
         * @param[out]  *buf    Array of `SD_SECTOR_SIZE` bytes used to hold a
         *                      sector from the SD card
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode inc_cluster (FatFS::FatBuffer *buf) {
            PropWare::ErrorCode err;

                // If the sector has been modified, write it back to the SD card
                // before reading again
                if (buf->mod) {
                    check_errors(
                            this->m_driver->write_data_block(
                                    buf->curClusterStartAddr + buf->curSectorOffset,
                                    buf->buf));
                }
                buf->mod = false;

            // Update this->m_cur*
            if (((uint32_t) FatFS::EOC_BEG) <= buf->curAllocUnit
                    && ((uint32_t) FatFS::EOC_END) <= buf->curAllocUnit)
                return FatFS::READING_PAST_EOC;
            buf->curAllocUnit = buf->nextAllocUnit;
            // Only look ahead to the next allocation unit if the current alloc
            // unit is not EOC
            if (!(((uint32_t) FatFS::EOC_BEG) <= buf->curAllocUnit
                    && ((uint32_t) FatFS::EOC_END) <= buf->curAllocUnit))
                // Current allocation unit is not EOC, read the next one
            check_errors(
                    this->get_fat_value(buf->curAllocUnit,
                                        &(buf->nextAllocUnit)));
            buf->curClusterStartAddr = this->find_sector_from_alloc(
                    buf->curAllocUnit);
            buf->curSectorOffset = 0;

            return this->m_driver->read_data_block(buf->curClusterStartAddr, buf->buf);
        }

    public:
        /**
         * Buffer object used for storing SD data; Each instance uses 527 bytes
         * (526 if SD_OPTION_FILE_WRITE is disabled)
         */
        class FatBuffer: public BlockStorage::Buffer {
            friend class FatFS;

            protected:
                /** Store the current cluster's starting sector number */
                uint32_t curClusterStartAddr;
                /**
                * Store the current sector offset from the beginning of the
                * cluster
                */
                uint8_t  curSectorOffset;
                /** Store the current allocation unit */
                uint32_t curAllocUnit;
                /** Look-ahead at the next FAT entry */
                uint32_t nextAllocUnit;
        };

    private:
        BlockStorage        *m_driver;
        bool                m_mounted;
        int                 m_nextFileId;

        uint8_t  m_filesystem;  // File system type - one of SD::FAT_16 or SD::FAT_32
        uint8_t  m_sectorsPerCluster_shift;  // Used as a quick multiply/divide; Stores log_2(Sectors per Cluster)
        uint32_t m_rootDirSectors;  // Number of sectors for the root directory
        uint32_t m_fatStart;  // Starting block address of the FAT
        uint32_t m_rootAddr;  // Starting block address of the root directory
        uint32_t m_rootAllocUnit;  // Allocation unit of root directory/first data sector (FAT32 only)
        uint32_t m_firstDataAddr;  // Starting block address of the first data cluster

        FatBuffer m_buf;
        uint8_t   *m_fat;  // Buffer for FAT entries only
        bool      m_fatMod;
        uint32_t  m_fatSize;
        uint16_t  m_entriesPerFatSector_Shift;  // How many FAT entries are in a single sector of the FAT
        uint32_t  m_curFatSector;  // Store the current FAT sector loaded into m_fat

        uint32_t m_dir_firstAllocUnit;  // Store the current directory's starting allocation unit
};

}
