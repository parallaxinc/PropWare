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
#include <PropWare/printer.h>

namespace PropWare {

/**
 * FAT 16/32 filesystem driver - can be used with SD cards, SPI memory, or any
 * class that implements the RAM interface
 */
class FatFS {
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

    public:
        FatFS (BlockStorage *driver) {
            this->m_driver = driver;
        }

        /**
         * @brief   Mount either FAT16 or FAT32 file system
         *
         * @return  Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode mount () {
            PropWare::ErrorCode err;

            // FAT system determination variables:
            FatFS::InitFATInfo fatInfo;
            fatInfo.bootSector = 0;

            // Allocate the buffers
            this->m_buf.buf = (uint8_t *) malloc(
                    this->m_driver->get_sector_size());
            this->m_fat = (uint8_t *) malloc(this->m_driver->get_sector_size());

            check_errors(this->read_boot_sector(&fatInfo));

            check_errors(this->common_boot_sector_parser(&fatInfo));

            this->partition_info_parser(&fatInfo);

            check_errors(this->determine_fat_type(&fatInfo));

            this->store_root_info(&fatInfo);

            check_errors(this->read_fat_and_root_sectors());

            this->m_mounted = true;

            return 0;
        }

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
        static const uint8_t FILE_ENTRY_LENGTH = 32;  // An entry in a directory uses 32 bytes
        static const uint8_t DELETED_FILE_MARK = 0xE5;  // Marks that a file has been deleted here, continue to the next entry
    #define SD_FILE_NAME_LEN        8
        static const uint8_t FILE_NAME_LEN = SD_FILE_NAME_LEN;  // 8 characters in the standard file name
    #define SD_FILE_EXTENSION_LEN   3
        static const uint8_t FILE_EXTENSION_LEN = SD_FILE_EXTENSION_LEN;  // 3 character file name extension
    #define SD_FILENAME_STR_LEN     SD_FILE_NAME_LEN + SD_FILE_EXTENSION_LEN + 2
        static const uint8_t FILENAME_STR_LEN      = SD_FILENAME_STR_LEN;
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

        inline PropWare::ErrorCode read_boot_sector (InitFATInfo *fatInfo) {
            PropWare::ErrorCode err;
            // Read in first sector (and use default buffer)
            check_errors(
                    this->m_driver->read_data_block(fatInfo->bootSector,
                                                    this->m_buf.buf));
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
            }

            // Print the boot sector if requested
#if (defined SD_OPTION_VERBOSE && defined SD_OPTION_VERBOSE_BLOCKS)
            pwOut.printf("***BOOT SECTOR***" CRLF);
            this->print_hex_block(this->m_buf.buf, SD::SECTOR_SIZE);
            pwOut.put_char('\n');
#endif

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

#ifdef SD_OPTION_VERBOSE
            pwOut.printf("Reading from the FAT..." CRLF);
            pwOut.printf("\tLooking for entry: 0x%08X / %u" CRLF, fatEntry, fatEntry);
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
                        this->m_driver->read_data_block(
                                this->m_curFatSector + this->m_fatStart,
                                this->m_fat));
#if (defined SD_OPTION_VERBOSE_BLOCKS && defined SD_OPTION_VERBOSE)
                this->print_hex_block(this->m_fat, SD::SECTOR_SIZE);
#endif
            }
            firstAvailableAllocUnit = this->m_curFatSector
                    << this->m_entriesPerFatSector_Shift;

#ifdef SD_OPTION_VERBOSE
            pwOut.printf("\tLooks like I need FAT sector: 0x%08X / %u" CRLF,
                    this->m_curFatSector, this->m_curFatSector);
            pwOut.printf("\tWith an offset of: 0x%04x / %u" CRLF,
                    (fatEntry - firstAvailableAllocUnit) << 2,
                    (fatEntry - firstAvailableAllocUnit) << 2);
#endif

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
#ifdef SD_OPTION_VERBOSE
            pwOut.printf("\tReceived value: 0x%08X / %u" CRLF, *value, *value);
#endif

            return 0;
        }

    public:
        /**
         * Buffer object used for storing SD data; Each instance uses 527 bytes
         * (526 if SD_OPTION_FILE_WRITE is disabled)
         */
        class FatBuffer: BlockStorage::Buffer {
            public:
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
        BlockStorage *m_driver;
        bool         m_mounted;

        uint8_t  m_filesystem;  // File system type - one of SD::FAT_16 or SD::FAT_32
        uint8_t  m_sectorsPerCluster_shift;  // Used as a quick multiply/divide; Stores log_2(Sectors per Cluster)
        uint32_t m_rootDirSectors;  // Number of sectors for the root directory
        uint32_t m_fatStart;  // Starting block address of the FAT
        uint32_t m_rootAddr;  // Starting block address of the root directory
        uint32_t m_rootAllocUnit;  // Allocation unit of root directory/first data sector (FAT32 only)
        uint32_t m_firstDataAddr;  // Starting block address of the first data cluster

        FatBuffer m_buf;
        uint8_t   *m_fat;  // Buffer for FAT entries only
        uint16_t  m_entriesPerFatSector_Shift;  // How many FAT entries are in a single sector of the FAT
        uint32_t  m_curFatSector;  // Store the current FAT sector loaded into m_fat

        uint32_t m_dir_firstAllocUnit;  // Store the current directory's starting allocation unit
};

}
