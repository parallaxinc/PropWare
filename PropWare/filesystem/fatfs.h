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
#include <PropWare/printer/printer.h>
#include <PropWare/filesystem/blockstorage.h>
#include <PropWare/filesystem/filesystem.h>
#include <PropWare/filesystem/fatfile.h>

namespace PropWare {

/**
 * FAT 16/32 filesystem driver - can be used with SD cards or any other PropWare::BlockStorage device
 */
class FatFS : public Filesystem {
    public:
        typedef enum {
                                   NO_ERROR  = 0,
                                   BEG_ERROR = Filesystem::ERROR_BEG + 1,
            /** FatFS Error 0 */   EMPTY_FAT_ENTRY = BEG_ERROR,
            /** FatFS Error 1 */   INVALID_PTR_ORIGIN,
            /** FatFS Error 2 */   INVALID_FAT_APPEND,
            /** FatFS Error 3 */   TOO_MANY_FATS,
            /** FatFS Error 4 */   READING_PAST_EOC,
            /** FatFS Error 5 */   FILE_WITHOUT_BUFFER,
            /** FatFS Error 6 */   PARTITION_DOES_NOT_EXIST,
            /** FatFS Error 7 */   INVALID_FILENAME,
            /** FatFS Error 8 */   INVALID_FILESYSTEM,
            /** Last FatFS error */END_ERROR = INVALID_FILESYSTEM
        } ErrorCode;

    public:

        FatFS (const BlockStorage *driver) : m_driver(driver),
                                             m_sectorSize(driver->get_sector_size()),
                                             m_sectorsPerCluster_shift(driver->get_sector_size_shift()),
                                             m_mounted(false),
                                             m_nextFileId(0),
                                             m_fat(NULL),
                                             m_fatMod(false) {
            this->m_error = NO_ERROR;
            this->m_buf.buf = NULL;
        }

        ~FatFS () {
            if (this->m_mounted)
                this->unmount();

            if (NULL != this->m_buf.buf)
                free(this->m_buf.buf);

            if (NULL != this->m_fat)
                free(this->m_fat);
        }

        /**
         * @see PropWare::Filesystem::mount
         */
        PropWare::ErrorCode mount (const uint8_t partition = 0) {
            PropWare::ErrorCode err;

            if (this->m_mounted)
                return Filesystem::FILESYSTEM_ALREADY_MOUNTED;
            else if (3 < partition)
                return INVALID_FILESYSTEM;

            // Start the driver
            check_errors(this->m_driver->start());
            this->m_fatMod = false;
            this->m_nextFileId = 0;

            InitFATInfo fatInfo;

            // Allocate the buffers
            if (NULL == this->m_buf.buf)
                this->m_buf.buf = (uint8_t *) malloc(this->m_sectorSize);
            if (NULL == this->m_fat)
                this->m_fat = (uint8_t *) malloc(this->m_sectorSize);

            check_errors(this->read_boot_sector(fatInfo, partition));
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

        File *fopen (const char name[], const File::Mode mode, BlockStorage::Buffer *buffer = NULL) {
            PropWare::ErrorCode err;
            uint16_t            fileEntryOffset = 0;

            // Attempt to find the file
            if ((err = this->find(name, &fileEntryOffset))) {
                // Find returned an error; ensure it was EOC...
                if (EOC_END == err) {
                    // And return a FILE_NOT_FOUND error if using read only mode
                    if (File::READ == mode) {
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

                if (EOC_END == err || FILENAME_NOT_FOUND == err) {
                    // File wasn't found, but there is still room in this
                    // cluster (or a new cluster was just added)
                    check_fs_error(this->create_file(name, &fileEntryOffset));
                }
                // find returned unknown error - throw it
                else {
                    this->m_error = err;
                    return NULL;
                }
            }

            // `name` was found successfully, determine if it is a file or
            // directory
            if (SUB_DIR & this->m_buf.buf[fileEntryOffset + FILE_ATTRIBUTE_OFFSET]) {
                this->m_error = Filesystem::ENTRY_NOT_FILE;
                return NULL;
            }

            FatFile *f;
            {
                BlockStorage::Buffer *fileBuffer;
                if (NULL == buffer)
                    fileBuffer = &this->m_buf;
                else
                    fileBuffer = buffer;
                f = FatFile::build(fileBuffer, this->m_nextFileId++, mode);
            }

            // Passed the file-not-directory test, load it into the buffer and
            // update status variables
            if (FAT_16 == this->m_filesystem)
                f->buf->curTier3 = this->m_driver->get_short(fileEntryOffset + FILE_START_CLSTR_LOW, this->m_buf.buf);
            else {
                f->buf->curTier3 = this->m_driver->get_short(fileEntryOffset + FILE_START_CLSTR_LOW, this->m_buf.buf);
                uint16_t highWord = this->m_driver->get_short(fileEntryOffset + FILE_START_CLSTR_HIGH, this->m_buf.buf);
                f->buf->curTier3 |= highWord << 16;

                // Clear the highest 4 bits - they are always reserved
                f->buf->curTier3 &= 0x0FFFFFFF;
            }
            f->firstAllocUnit           = f->buf->curTier3;
            f->curCluster               = 0;
            f->buf->curTier2StartAddr = this->find_sector_from_alloc(f->buf->curTier3);
            f->dirSectorAddr            = this->m_buf.curTier2StartAddr
                    + this->m_buf.curTier1Offset;
            f->fileEntryOffset          = fileEntryOffset;
            check_fs_error(this->get_fat_value(f->buf->curTier3, &(f->buf->nextTier3)));
            f->buf->curTier1Offset = 0;
            Filesystem::set_file_length(f, this->m_driver->get_long(fileEntryOffset + FILE_LEN_OFFSET,
                                                                    this->m_buf.buf));
            // Determine the number of sectors currently allocated to this file;
            // useful in the case that the file needs to be extended
            f->maxSectors     = Filesystem::get_file_length(f) >> this->m_driver->get_sector_size_shift();
            if (!(f->maxSectors))
                f->maxSectors = (uint32_t) (1 << this->m_sectorsPerCluster_shift);
            while (f->maxSectors % (1 << this->m_sectorsPerCluster_shift))
                ++(f->maxSectors);
            f->buf->mod = false;
            check_fs_error(this->m_driver->read_data_block(f->buf->curTier2StartAddr, f->buf->buf));

            return 0;
        }

    private:
        // Valid partition IDs for FAT volumes
        static const uint8_t PARTITION_IDS[54];

        // Boot sector addresses/values
        static const uint8_t  FAT_16                 = 2;  // A FAT entry in FAT16 is 2-bytes
        static const uint8_t  FAT_32                 = 4;  // A FAT entry in FAT32 is 4-bytes
        static const uint8_t  BOOT_SECTOR_ID         = 0xEB;
        static const uint8_t  BOOT_SECTOR_ID_ADDR    = 0;
        static const uint16_t PARTITION_TABLE_START  = 0x1BE;
        static const uint8_t  PARTITION_ID_OFFSET    = 0x04;
        static const uint8_t  PARTITION_START_OFFSET = 0x08;
        static const uint8_t  CLUSTER_SIZE_ADDR      = 0x0D;
        static const uint8_t  RSVD_SCTR_CNT_ADDR     = 0x0E;
        static const uint8_t  NUM_FATS_ADDR          = 0x10;
        static const uint8_t  ROOT_ENTRY_CNT_ADDR    = 0x11;
        static const uint8_t  TOT_SCTR_16_ADDR       = 0x13;
        static const uint8_t  FAT_SIZE_16_ADDR       = 0x16;
        static const uint8_t  TOT_SCTR_32_ADDR       = 0x20;
        static const uint8_t  FAT_SIZE_32_ADDR       = 0x24;
        static const uint8_t  ROOT_CLUSTER_ADDR      = 0x2c;
        static const uint16_t FAT12_CLSTR_CNT        = 4085;
        static const uint16_t FAT16_CLSTR_CNT        = 65525;

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
         * @brief       Read the master boot record and load in the boot sector for the requested partition
         *
         * @param[out]  fatInfo     Store information about the FAT here
         * @param[in]   partition   0-indexed description of the requested partition
         *
         * @return 0 upon success, error code otherwise
         */
        inline PropWare::ErrorCode read_boot_sector (InitFATInfo &fatInfo, const uint8_t partition) {
            PropWare::ErrorCode err;

            // Read first sector into the default buffer
            check_errors(this->m_driver->read_data_block(0, this->m_buf.buf));
            const uint8_t bootSectorId = this->m_driver->get_byte(BOOT_SECTOR_ID_ADDR, this->m_buf.buf);

            // If we don't have a master boot record, the job is simple...
            if (BOOT_SECTOR_ID == bootSectorId) {
                // Make sure the user requested partition 0 and then call it done
                if (0 == partition)
                    fatInfo.bootSector = 0;
                else
                    // A boot sector at address 0 means only one partition exists on the entire device
                    return PARTITION_DOES_NOT_EXIST;
            }
            // Sector 0 is the master boot record - great. Parse the partition table and read in the boot sector
            else {
                const uint16_t partitionRow = PARTITION_TABLE_START + (partition << 4);
                check_errors(this->is_fat_volume(this->m_buf.buf[partitionRow + PARTITION_ID_OFFSET]));
                fatInfo.bootSector = this->m_driver->get_long(partitionRow + PARTITION_START_OFFSET, this->m_buf.buf);
                check_errors(this->m_driver->read_data_block(fatInfo.bootSector, this->m_buf.buf));
            }

            return 0;
        }

        inline PropWare::ErrorCode is_fat_volume (const uint8_t partitionId) {
            for (uint8_t i = 0; i < sizeof(PARTITION_IDS); ++i)
                if (PARTITION_IDS[i] == partitionId) {
                    return NO_ERROR;
                }

            return INVALID_FILESYSTEM;
        }

        inline PropWare::ErrorCode common_boot_sector_parser (InitFATInfo *fatInfo) {
            uint8_t temp;

            // Get the reserved sector count
            fatInfo->rsvdSectorCount = this->m_driver->get_short(RSVD_SCTR_CNT_ADDR, this->m_buf.buf);

            // Total number of FATs
            fatInfo->numFATs = this->m_driver->get_byte(NUM_FATS_ADDR, this->m_buf.buf);
#ifdef SD_OPTION_FILE_WRITE
            if (2 != fatInfo->numFATs)
            return TOO_MANY_FATS;
#endif

            // Number of entries in the root directory
            fatInfo->rootEntryCount = this->m_driver->get_short(ROOT_ENTRY_CNT_ADDR, this->m_buf.buf);

            return 0;
        }

        inline void partition_info_parser (InitFATInfo *fatInfo) {
            // Check if FAT size is valid in 16- or 32-bit location
            fatInfo->FATSize = this->m_driver->get_short(FAT_SIZE_16_ADDR, this->m_buf.buf);
            if (!(fatInfo->FATSize))
                fatInfo->FATSize = this->m_driver->get_long(FAT_SIZE_32_ADDR, this->m_buf.buf);

            // Check if FAT16 total sectors is valid
            fatInfo->totalSectors = this->m_driver->get_short(TOT_SCTR_16_ADDR, this->m_buf.buf);
            if (!(fatInfo->totalSectors))
                fatInfo->totalSectors = this->m_driver->get_long(TOT_SCTR_32_ADDR, this->m_buf.buf);

            // Compute necessary numbers to determine FAT type (12/16/32)
            this->m_rootDirSectors = (fatInfo->rootEntryCount * 32) >> this->m_driver->get_sector_size_shift();
            fatInfo->dataSectors   = fatInfo->totalSectors - (fatInfo->rsvdSectorCount + fatInfo->numFATs *
                    fatInfo->FATSize + fatInfo->rootEntryCount);
            fatInfo->clusterCount  = fatInfo->dataSectors >> this->m_sectorsPerCluster_shift;
        }

        inline PropWare::ErrorCode determine_fat_type (InitFATInfo *fatInfo) {
            // Determine and store FAT type
            if (FAT12_CLSTR_CNT > fatInfo->clusterCount) {
                return INVALID_FILESYSTEM;}
            else if (FAT16_CLSTR_CNT > fatInfo->clusterCount) {
#ifdef SD_OPTION_VERBOSE
                pwOut.printf(CRLF "***FAT type is FAT16***" CRLF);
#endif
                this->m_filesystem                = FAT_16;
                this->m_entriesPerFatSector_Shift = 8;
            } else {
#ifdef SD_OPTION_VERBOSE
                pwOut.printf(CRLF "***FAT type is FAT32***" CRLF);
#endif
                this->m_filesystem                = FAT_32;
                this->m_entriesPerFatSector_Shift = 7;
            }

            return 0;
        }

        inline void store_root_info (InitFATInfo *fatInfo) {
            // Find start of FAT
            this->m_fatStart = fatInfo->bootSector + fatInfo->rsvdSectorCount;

            //    this->m_filesystem = FAT_16;
            // Find root directory address
            switch (this->m_filesystem) {
                case FAT_16:
                    this->m_rootAddr      = fatInfo->FATSize * fatInfo->numFATs + this->m_fatStart;
                    this->m_firstDataAddr = this->m_rootAddr + this->m_rootDirSectors;
                    break;
                case FAT_32:
                    this->m_firstDataAddr = this->m_rootAddr = fatInfo->bootSector + fatInfo->rsvdSectorCount
                            + fatInfo->FATSize * fatInfo->numFATs;
                    this->m_rootAllocUnit = this->m_driver->get_long(ROOT_CLUSTER_ADDR, this->m_buf.buf);
                    break;
            }

            // If files will be writable, the second FAT must also be updated -
            // the first sector's address is stored here
            this->m_fatSize = fatInfo->FATSize;

#ifdef SD_OPTION_VERBOSE
            pwOut.printf("Start of FAT: 0x%08X" CRLF, this->m_fatStart);
            pwOut.printf("Root directory alloc. unit: 0x%08X" CRLF, this->m_rootAllocUnit);
            pwOut.printf("Root directory sector: 0x%08X" CRLF, this->m_rootAddr);
            pwOut.printf("Calculated root directory sector: 0x%08X" CRLF, this->find_sector_from_alloc(this->m_rootAllocUnit));
            pwOut.printf("First data sector: 0x%08X" CRLF, this->m_firstDataAddr);
#endif
        }

        inline PropWare::ErrorCode read_fat_and_root_sectors () {
            PropWare::ErrorCode err;

            // Store the first sector of the FAT
            check_errors(this->m_driver->read_data_block(this->m_fatStart, this->m_fat));
            this->m_curFatSector = 0;

            // Print FAT if desired
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_VERBOSE_BLOCKS)
            pwOut.printf(CRLF "***First File Allocation Table***" CRLF);
            BlockStorage::print_block(pwOut, this->m_fat);
            pwOut.print(CRLF);
#endif

            // Read in the root directory, set root as current
            check_errors(this->m_driver->read_data_block(this->m_rootAddr, this->m_buf.buf));
            this->m_buf.curTier2StartAddr = this->m_rootAddr;
            if (FAT_16 == this->m_filesystem) {
                this->m_dir_firstAllocUnit = (uint32_t) -1;
                this->m_buf.curTier3 = (uint32_t) -1;
            } else {
                this->m_buf.curTier3 = this->m_dir_firstAllocUnit = this->m_rootAllocUnit;
                check_errors(this->get_fat_value(this->m_buf.curTier3, &this->m_buf.nextTier3));
            }
            this->m_buf.curTier2StartAddr = this->m_rootAddr;
            this->m_buf.curTier1Offset = 0;

            // Print root directory
#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
            pwOut.printf("***Root directory***" CRLF);
            BlockStorage::print_block(pwOut, this->m_buf.buf);
            pwOut.print(CRLF);
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
        PropWare::ErrorCode get_fat_value (const uint32_t fatEntry, uint32_t *value) {
            PropWare::ErrorCode err;
            uint32_t            firstAvailableAllocUnit;

            // Do we need to load a new fat sector?
            if ((fatEntry >> this->m_entriesPerFatSector_Shift)
                    != this->m_curFatSector) {
                // If the currently loaded FAT sector has been modified, save it
                if (this->m_fatMod) {
                    this->m_driver->write_data_block(this->m_curFatSector + this->m_fatStart, this->m_fat);
                    this->m_driver->write_data_block(this->m_curFatSector + this->m_fatStart + this->m_fatSize,
                                                     this->m_fat);
                    this->m_fatMod = false;
                }
                // Need new sector, load it
                this->m_curFatSector = fatEntry
                        >> this->m_entriesPerFatSector_Shift;
                check_errors(this->m_driver->read_data_block(this->m_curFatSector + this->m_fatStart, this->m_fat));
            }
            firstAvailableAllocUnit = this->m_curFatSector << this->m_entriesPerFatSector_Shift;

            // The necessary FAT sector has been loaded and the next allocation
            // unit is known, proceed with loading the next data sector and
            // incrementing the cluster variables

            // Retrieve the next allocation unit number
            if (FAT_16 == this->m_filesystem)
                *value = this->m_driver->get_short((fatEntry - firstAvailableAllocUnit) << 1, this->m_fat);
            else
                /* Implied check for (FAT_32 == this->m_filesystem) */
                *value = this->m_driver->get_long((fatEntry - firstAvailableAllocUnit) << 2, this->m_fat);
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
        PropWare::ErrorCode find (const char *filename, uint16_t *fileEntryOffset) {
            PropWare::ErrorCode err;
            char readEntryName[FILENAME_STR_LEN];

                // Save the current buffer
                if (this->m_buf.mod) {
                    check_errors(this->m_driver->write_data_block(
                            this->m_buf.curTier2StartAddr + this->m_buf.curTier1Offset, this->m_buf.buf));
                    this->m_buf.mod = false;
                }

            *fileEntryOffset = 0;

            // If we aren't looking at the beginning of the directory cluster,
            // we must backtrack to the beginning and then begin listing files
            if (this->m_buf.curTier1Offset
                    || (this->find_sector_from_alloc(this->m_dir_firstAllocUnit) != this->m_buf.curTier2StartAddr)) {
                this->m_buf.curTier2StartAddr = this->find_sector_from_alloc(this->m_dir_firstAllocUnit);
                this->m_buf.curTier1Offset = 0;
                this->m_buf.curTier3 = this->m_dir_firstAllocUnit;
                check_fs_error(this->get_fat_value(this->m_buf.curTier3, &this->m_buf.nextTier3));
                check_fs_error(this->m_driver->read_data_block(this->m_buf.curTier2StartAddr, this->m_buf.buf));
            }
            this->m_buf.id = File::FOLDER_ID;

            // Loop through all entries in the current directory until we find
            // the correct one
            // Function will exit normally with EOC_END error code if the
            // file is not found
            while (this->m_buf.buf[*fileEntryOffset]) {
                // Check if file is valid, retrieve the name if it is
                if (DELETED_FILE_MARK != this->m_buf.buf[*fileEntryOffset]) {
                    this->get_filename(&(this->m_buf.buf[*fileEntryOffset]), readEntryName);
                    if (!strcmp(filename, readEntryName))
                        // File names match, return 0 to indicate a successful
                        // search
                        return 0;
                }

                // Increment to the next file
                *fileEntryOffset += FILE_ENTRY_LENGTH;

                // If it was the last entry in this sector, proceed to the next
                // one
                if (this->m_sectorSize == *fileEntryOffset) {
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
            if (FAT_32 == this->m_filesystem)
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
            for (i = 0; i < FILE_NAME_LEN; ++i) {
                if (0x05 == buf[i])
                    filename[j++] = (char) 0xe5;
                else if (' ' != buf[i])
                    filename[j++] = buf[i];
            }

            // Determine if there is more past the first 8 - Again, stop when a
            // space is reached
            if (' ' != buf[FILE_NAME_LEN]) {
                filename[j++] = '.';
                for (i = FILE_NAME_LEN;
                     i < FILE_NAME_LEN + FILE_EXTENSION_LEN; ++i) {
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
         * @param[out]  *buf    Buffer that the sector should be loaded into
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode load_next_sector (BlockStorage::Buffer *buf) {
                if (buf->mod)
                    this->m_driver->write_data_block(buf->curTier2StartAddr + buf->curTier1Offset, buf->buf);

            // Check for the end-of-chain marker (end of file)
            if (((uint32_t) EOC_BEG) <= buf->nextTier3)
                return EOC_END;

            // Are we looking at the root directory of a FAT16 system?
            if (FAT_16 == this->m_filesystem
                    && this->m_rootAddr == (buf->curTier2StartAddr)) {
                // Root dir of FAT16; Is it the last sector in the root
                // directory?
                if (this->m_rootDirSectors == (buf->curTier1Offset))
                    return EOC_END;
                    // Root dir of FAT16; Not last sector
                else
                    // Any error from reading the data block will be returned to
                    // calling function
                    return this->m_driver->read_data_block(++(buf->curTier1Offset), buf->buf);
            }
                // We are looking at a generic data cluster.
            else {
                // Generic data cluster; Have we reached the end of the cluster?
                if (((1 << this->m_sectorsPerCluster_shift) - 1) > (buf->curTier1Offset)) {
                    // Generic data cluster; Not the end; Load next sector in
                    // the cluster

                    // Any error from reading the data block will be returned to
                    // calling function
                    buf->curTier1Offset++;
                    return this->m_driver->read_data_block(buf->curTier1Offset + buf->curTier2StartAddr, buf->buf);
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
         * @param[out]  *buf    Buffer that the next sector should be loaded into
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode inc_cluster (BlockStorage::Buffer *buf) {
            PropWare::ErrorCode err;

            // If the sector has been modified, write it back to the SD card
            // before reading again
            if (buf->mod) {
                check_errors(
                        this->m_driver->write_data_block(
                                buf->curTier2StartAddr + buf->curTier1Offset,
                                buf->buf));
            }
            buf->mod = false;

            // Update this->m_cur*
            if (((uint32_t) EOC_BEG) <= buf->curTier3
                    && ((uint32_t) EOC_END) <= buf->curTier3)
                return READING_PAST_EOC;
            buf->curTier3 = buf->nextTier3;
            // Only look ahead to the next allocation unit if the current alloc
            // unit is not EOC
            if (!(((uint32_t) EOC_BEG) <= buf->curTier3
                    && ((uint32_t) EOC_END) <= buf->curTier3))
                // Current allocation unit is not EOC, read the next one
            check_errors(this->get_fat_value(buf->curTier3, &(buf->nextTier3)));
            buf->curTier2StartAddr = this->find_sector_from_alloc(buf->curTier3);
            buf->curTier1Offset = 0;

            return this->m_driver->read_data_block(buf->curTier2StartAddr, buf->buf);
        }

        /**
         * @brief       Enlarge a file or directory by one cluster
         *
         * @param[in]   *buf    Address of the buffer (containing information for a file or directory) to be enlarged
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode extend_fat (BlockStorage::Buffer *buf) {
            PropWare::ErrorCode err;
            uint32_t newAllocUnit;
#ifdef SD_OPTION_VERBOSE
                pwOut.printf("Extending file or directory now..." CRLF);
#endif

            // Do we need to load a different sector of the FAT or is the
            // correct one currently loaded? (Correct means the sector currently
            // containing the EOC marker)
            if ((buf->curTier3 >> this->m_entriesPerFatSector_Shift) != this->m_curFatSector) {
#ifdef SD_OPTION_VERBOSE
                pwOut.printf("Need new FAT sector. Loading: 0x%08X / %u" CRLF,
                             buf->curTier3 >> this->m_entriesPerFatSector_Shift,
                             buf->curTier3 >> this->m_entriesPerFatSector_Shift);
                pwOut.printf("... because the current allocation unit is: ""0x%08X / %u" CRLF, buf->curTier3,
                             buf->curTier3);
#endif

                // Need new sector, save the old one...
                if (this->m_fatMod) {
                    this->m_driver->write_data_block(this->m_curFatSector + this->m_fatStart, this->m_fat);
                    this->m_driver->write_data_block(this->m_curFatSector + this->m_fatStart + this->m_fatSize,
                                                     this->m_fat);
                    this->m_fatMod = false;
                }
                // And load the new one...
                this->m_curFatSector = buf->curTier3 >> this->m_entriesPerFatSector_Shift;
                check_errors(this->m_driver->read_data_block(this->m_curFatSector + this->m_fatStart, this->m_fat));
            }

            // This function should only be called when a file or directory has
            // reached the end of its cluster chain
            uint16_t entriesPerFatSector = (uint16_t) (1 << this->m_entriesPerFatSector_Shift);
            uint16_t allocUnitOffset = (uint16_t) (buf->curTier3 % entriesPerFatSector);
            uint16_t fatPointerAddress = allocUnitOffset * this->m_filesystem;
            uint32_t nextSector = this->m_driver->get_long(fatPointerAddress, this->m_fat);
            if ((uint32_t) EOC_BEG <= nextSector)
                return INVALID_FAT_APPEND;

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
            // Display the currently loaded FAT.... for no reason... not sure
            // why I wanted to do this...
            pwOut.printf("This is the sector that *should* contain the EOC marker..." CRLF);
            BlockStorage::print_block(pwOut, this->m_fat);
#endif

            // Find where the next cluster of the file should be stored...
            newAllocUnit = this->find_empty_space(1);

            // Now that we know the allocation unit, write it to the FAT buffer
            const uint16_t x = (buf->curTier3 % (1 << this->m_entriesPerFatSector_Shift)) * this->m_filesystem;
            if (FAT_16 == this->m_filesystem)
                this->m_driver->write_short(x, this->m_fat, newAllocUnit);
            else
                this->m_driver->write_long(x, this->m_fat, newAllocUnit);
            buf->nextTier3 = newAllocUnit;
            this->m_fatMod = true;  // And mark the buffer as modified

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
            pwOut.printf("After modification, the FAT now looks like..." CRLF);
            BlockStorage::print_block(pwOut, this->m_fat);
#endif

            return 0;
        }

        /**
         * @brief       Find the first empty allocation unit in the FAT
         *
         * The value of the first empty allocation unit is returned and its
         * location will contain the end-of-chain marker, SD_EOC_END.
         *
         * NOTE: It is important to realize that, though the new entry now
         * contains an EOC marker, this function does not know what cluster is
         * being extended and therefore the calling function must modify the
         * previous EOC to contain the return value
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
            pwOut.printf(CRLF "*** SDFindEmptySpace() initialized with FAT sector ""0x%08X / %u loaded ***" CRLF,
                         this->m_curFatSector, this->m_curFatSector);
            BlockStorage::print_block(pwOut, this->m_fat);
#endif

            // Find the first empty allocation unit and write the EOC marker
            if (FAT_16 == this->m_filesystem) {
                // Loop until we find an empty cluster
                while (this->m_driver->get_short(allocOffset, this->m_fat)) {
#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
                    pwOut.printf("Searching the following sector..." CRLF);
                    BlockStorage::print_block(pwOut, this->m_fat);
#endif
                    // Stop when we either reach the end of the current block or
                    // find an empty cluster
                    while (this->m_driver->get_short(allocOffset, this->m_fat) && (this->m_sectorSize > allocOffset))
                        allocOffset += FAT_16;
                    // If we reached the end of a sector...
                    if (this->m_sectorSize <= allocOffset) {
                        // If the currently loaded FAT sector has been modified,
                        // save it
                        if (this->m_fatMod) {
#ifdef SD_OPTION_VERBOSE
                                pwOut.printf("FAT sector has been modified; saving now... ");
#endif
                            this->m_driver->write_data_block(this->m_curFatSector, this->m_fat);
                            this->m_driver->write_data_block(this->m_curFatSector + this->m_fatSize, this->m_fat);
#ifdef SD_OPTION_VERBOSE
                                pwOut.printf("done!" CRLF);
#endif
                            this->m_fatMod = false;
                        }
                        // Read the next fat sector
#ifdef SD_OPTION_VERBOSE
                            pwOut.printf("SDFindEmptySpace() is reading in sector address: 0x%08X / %u" CRLF, fatSectorAddr + 1,
                                    fatSectorAddr + 1);
#endif
                        this->m_driver->read_data_block(++fatSectorAddr, this->m_fat);
                    }
                }
                this->m_driver->write_short(allocOffset, this->m_fat, EOC_END);
                this->m_fatMod = true;
            } else /* Implied: "if (FAT_32 == this->m_filesystem)" */{
                // In FAT32, the first 7 usable clusters seem to be
                // un-officially reserved for the root directory
                if (0 == this->m_curFatSector)
                    allocOffset = (uint16_t) (9 * this->m_filesystem);

                // Loop until we find an empty cluster
                while (this->m_driver->get_long(allocOffset, this->m_fat) & 0x0fffffff) {
#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
                    pwOut.printf("Searching the following sector..." CRLF);
                    BlockStorage::print_block(pwOut, this->m_fat);
#endif
                    // Stop when we either reach the end of the current block or
                    // find an empty cluster
                    while ((this->m_driver->get_long(allocOffset, this->m_fat) & 0x0fffffff)
                            && (this->m_sectorSize > allocOffset))
                        allocOffset += FAT_32;

#ifdef SD_OPTION_VERBOSE
                        pwOut.printf("Broke while loop... why? Offset = 0x%04x / %u" CRLF, allocOffset, allocOffset);
#endif
                    // If we reached the end of a sector...
                    if (this->m_sectorSize <= allocOffset) {
                        if (this->m_fatMod) {
#ifdef SD_OPTION_VERBOSE
                            pwOut.printf("FAT sector has been modified; saving now... ");
#endif
                            this->m_driver->write_data_block(this->m_curFatSector + this->m_fatStart, this->m_fat);
                            this->m_driver->write_data_block(this->m_curFatSector + this->m_fatStart + this->m_fatSize,
                                                             this->m_fat);
#ifdef SD_OPTION_VERBOSE
                                pwOut.printf("done!" CRLF);
#endif
                            this->m_fatMod = false;
                        }
                        // Read the next fat sector
#ifdef SD_OPTION_VERBOSE
                            pwOut.printf("SDFindEmptySpace() is reading in sector address: 0x%08X / %u" CRLF,
                                         fatSectorAddr + 1, fatSectorAddr + 1);
#endif
                        this->m_driver->read_data_block(++fatSectorAddr, this->m_fat);
                        allocOffset = 0;
                    }
                }

                this->m_driver->write_long(allocOffset,this->m_fat, ((uint32_t) EOC_END) & 0x0fffffff);
                this->m_fatMod = true;
            }

#ifdef SD_OPTION_VERBOSE
            const uint32_t addr = this->m_curFatSector << this->m_entriesPerFatSector_Shift + allocOffset / this->m_filesystem;
            pwOut.printf("Available space found: 0x%08X / %u" CRLF, addr, addr);
#endif

            // If we loaded a new fat sector (and then modified it directly
            // above), write the sector before re-loading the original
            if ((fatSectorAddr != (this->m_curFatSector + this->m_fatStart)) && this->m_fatMod) {
                this->m_driver->write_data_block(fatSectorAddr, this->m_fat);
                this->m_driver->write_data_block(fatSectorAddr + this->m_fatSize, this->m_fat);
                this->m_fatMod = false;
                this->m_driver->read_data_block(this->m_curFatSector + this->m_fatStart, this->m_fat);
            } else
                this->m_curFatSector = fatSectorAddr - this->m_fatStart;

            // Return new address to end-of-chain
            retVal = this->m_curFatSector << this->m_entriesPerFatSector_Shift;
            retVal += allocOffset / this->m_filesystem;
            return retVal;
        }

        /**
         * @brief       Allocate space for a new file
         *
         * @param[in]   *name               Character array for the new file
         * @param[in]   *fileEntryOffset    Offset from the currently loaded directory entry where the file's metadata
         *                                  should be written
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode create_file (const char *name, const uint16_t *fileEntryOffset) {
            uint8_t i, j;
            // *name is only checked for uppercase
            char uppercaseName[FILENAME_STR_LEN];
            uint32_t allocUnit;

#ifdef SD_OPTION_VERBOSE
            pwOut.printf("Creating new file: %s" CRLF, name);
#endif

            // Parameter checking...
            if (FILENAME_STR_LEN < strlen(name))
                return INVALID_FILENAME;

            // Convert the name to uppercase
            strcpy(uppercaseName, name);
            Utility::to_upper(uppercaseName);

            // Write the file fields in order...

            /* 1) Short file name */
            // Write first section
            for (i = 0; '.' != uppercaseName[i] && 0 != uppercaseName[i]; ++i)
                this->m_buf.buf[*fileEntryOffset + i] = (uint8_t) uppercaseName[i];
            // Check if there is an extension
            if (uppercaseName[i]) {
                // There might be an extension - pad first name with spaces
                for (j = i; j < FILE_NAME_LEN; ++j)
                    this->m_buf.buf[*fileEntryOffset + j] = ' ';
                // Check if there is a period, as one would expect for a file
                // name with an extension
                if ('.' == uppercaseName[i]) {
                    // Extension exists, write it
                    ++i;        // Skip the period
                    // Insert extension, character-by-character
                    for (j = FILE_NAME_LEN; uppercaseName[i]; ++j)
                        this->m_buf.buf[*fileEntryOffset + j] = (uint8_t) uppercaseName[i++];
                    // Pad extension with spaces
                    for (; j < FILE_NAME_LEN + FILE_EXTENSION_LEN; ++j)
                        this->m_buf.buf[*fileEntryOffset + j] = ' ';
                }
                    // If it wasn't a period or null terminator, throw an error
                else
                    return INVALID_FILENAME;
            }
            // No extension, pad with spaces
            else
                for (; i < (FILE_NAME_LEN + FILE_EXTENSION_LEN); ++i)
                    this->m_buf.buf[*fileEntryOffset + i] = ' ';

            /* 2) Write attribute field... */
            // TODO: Allow for file attribute flags to be set, such as
            //       READ_ONLY, SUB_DIR, etc
            // Archive flag should be set because the file is new
            this->m_buf.buf[*fileEntryOffset + FILE_ATTRIBUTE_OFFSET] = ARCHIVE;
            this->m_buf.mod = true;

#ifdef SD_OPTION_VERBOSE
            print_file_entry(&(this->m_buf.buf[*fileEntryOffset]), uppercaseName);
#endif

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
            BlockStorage::print_block(pwOut, this->m_buf.buf);
#endif

            /**
             * 3) Find a spot in the FAT (do not check for a full FAT, assume
             *    space is available)
             */
            allocUnit = this->find_empty_space(0);
            this->m_driver->write_short(*fileEntryOffset + FILE_START_CLSTR_LOW, this->m_buf.buf, allocUnit);
            if (FAT_32 == this->m_filesystem)
                this->m_driver->write_short(*fileEntryOffset + FILE_START_CLSTR_HIGH, this->m_buf.buf, allocUnit >> 16);

            /* 4) Write the size of the file (currently 0) */
            this->m_driver->write_long(*fileEntryOffset + FILE_LEN_OFFSET, this->m_buf.buf, 0);

#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
            pwOut.printf("New file entry at offset 0x%08X / %u looks like..." CRLF, *fileEntryOffset, *fileEntryOffset);
            BlockStorage::print_block(pwOut, this->m_buf.buf);
#endif

            this->m_buf.mod = true;

            return 0;
        }

        /**
         * @brief       Print the attributes and name of a file entry
         *
         * @param[in]   *fileEntry  Address of the first byte of the file entry
         * @param[out]  *filename   Allocated space for the filename string to be stored
         */
        void print_file_entry (const uint8_t *fileEntry, char filename[]) {
            print_file_attributes(fileEntry[FILE_ATTRIBUTE_OFFSET]);
            this->get_filename(fileEntry, filename);
            pwOut.printf("\t\t%s", filename);
            if (SUB_DIR & fileEntry[FILE_ATTRIBUTE_OFFSET])
                pwOut.print('/');
            pwOut.print(CRLF);
        }
    
        /**
         * @brief       Print attributes of a file entry
         *
         * @param[in]   flags   Flags that are set - each bit in this parameter corresponds to a line that will be
         *                      printed
         */
        void print_file_attributes (const uint8_t flags) {
            // Print file attributes
            if (READ_ONLY & flags)
                pwOut.print(READ_ONLY_CHAR);
            else
                pwOut.print(READ_ONLY_CHAR_);

            if (HIDDEN_FILE & flags)
                pwOut.print(HIDDEN_FILE_CHAR);
            else
                pwOut.print(HIDDEN_FILE_CHAR_);

            if (SYSTEM_FILE & flags)
                pwOut.print(SYSTEM_FILE_CHAR);
            else
                pwOut.print(SYSTEM_FILE_CHAR_);

            if (VOLUME_ID & flags)
                pwOut.print(VOLUME_ID_CHAR);
            else
                pwOut.print(VOLUME_ID_CHAR_);

            if (SUB_DIR & flags)
                pwOut.print(SUB_DIR_CHAR);
            else
                pwOut.print(SUB_DIR_CHAR_);

            if (ARCHIVE & flags)
                pwOut.print(ARCHIVE_CHAR);
            else
                pwOut.print(ARCHIVE_CHAR_);
        }

    private:
        const BlockStorage *m_driver;
        const uint16_t     m_sectorSize;
        const uint8_t      m_sectorsPerCluster_shift;  // Used as a quick multiply/divide; Stores log_2(Sectors per Cluster)
        bool               m_mounted;
        int                m_nextFileId;

        uint8_t m_filesystem;  // File system type - one of FAT_16 or FAT_32
        uint32_t m_rootDirSectors;  // Number of sectors for the root directory
        uint32_t m_fatStart;  // Starting block address of the FAT
        uint32_t m_rootAddr;  // Starting block address of the root directory
        uint32_t m_rootAllocUnit;  // Allocation unit of root directory/first data sector (FAT32 only)
        uint32_t m_firstDataAddr;  // Starting block address of the first data cluster

        BlockStorage::Buffer m_buf;
        uint8_t              *m_fat;  // Buffer for FAT entries only
        bool                 m_fatMod;
        uint32_t             m_fatSize;
        uint16_t             m_entriesPerFatSector_Shift;  // How many FAT entries are in a single sector of the FAT
        uint32_t             m_curFatSector;  // Store the current FAT sector loaded into m_fat

        uint32_t m_dir_firstAllocUnit;  // Store the current directory's starting allocation unit
};

}
