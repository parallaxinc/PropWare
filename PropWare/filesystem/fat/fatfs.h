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
#include <PropWare/utility.h>
#include <PropWare/printer/printer.h>
#include <PropWare/filesystem/blockstorage.h>
#include <PropWare/filesystem/filesystem.h>
#include <PropWare/c++allocate.h>

#include <stdbool.h>

namespace PropWare {

/**
 * FAT 16/32 filesystem driver - can be used with SD cards or any other PropWare::BlockStorage device
 */
class FatFS : public Filesystem {
        friend class FatFile;

        friend class FatFileReader;

        friend class FatFileWriter;

    public:
        typedef enum {
                                   NO_ERROR        = 0,
                                   BEG_ERROR       = Filesystem::END_ERROR + 1,
            /** FatFS Error 0 */   EMPTY_FAT_ENTRY = BEG_ERROR,
            /** FatFS Error 1 */   INVALID_FAT_APPEND,
            /** FatFS Error 2 */   TOO_MANY_FATS,
            /** FatFS Error 3 */   BAD_SECTORS_PER_CLUSTER,
            /** FatFS Error 4 */   READING_PAST_EOC,
            /** FatFS Error 5 */   PARTITION_DOES_NOT_EXIST,
            /** FatFS Error 6 */   UNSUPPORTED_FILESYSTEM,
            /** Last FatFS error */END_ERROR       = UNSUPPORTED_FILESYSTEM
        }    ErrorCode;

    public:
        /**
         * @brief       Constructor
         *
         * @param[in]   *driver     Address of a the driver which is capable of reading the physical hardware. Commonly,
         *                          this would be an instance of PropWare::SD, but one could potentially write a driver
         *                          for a floppy disk or CD driver or any other block storage device
         * @param[in]   *logger     Useful for debugging, a logger can be given to help determine when something goes
         *                          wrong. All code using the logger will be optimized out by GCC so long as you only
         *                          call public method
         */
        FatFS (const BlockStorage *driver, const Printer *logger = &pwOut)
                : Filesystem(driver, logger),
                  m_fat(NULL),
                  m_fatMod(false) {
        }

        /**
         * @brief   Destructor. Unmounts the filesystem and flushes all buffers
         */
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
                return UNSUPPORTED_FILESYSTEM;

            // Start the driver
            check_errors(this->m_driver->start());
            this->m_fatMod     = false;
            this->m_nextFileId = 0;

            // Allocate the buffers
            if (NULL == this->m_buf.buf)
                this->m_buf.buf        = (uint8_t *) malloc(this->m_sectorSize);
            if (NULL == this->m_fat)
                this->m_fat            = (uint8_t *) malloc(this->m_sectorSize);
            if (Utility::empty(this->m_buf.meta->name))
                this->m_buf.meta->name = "FAT shared buffer";

            // Excellent information on determining FAT type can be found on page 14 of the following document,
            // starting with the section "FAT Type Determination"
            // https://staff.washington.edu/dittrich/misc/fatgen103.pdf
            check_errors(this->read_boot_sector(partition));
            check_errors(this->common_boot_sector_parser());
            this->partition_info_parser();
            check_errors(this->determine_fat_type());
            this->store_root_info();
            check_errors(this->read_fat_and_root_sectors());

            this->m_mounted = true;

            return 0;
        }

        /**
         * @see PropWare::Filesystem::unmount
         */
        PropWare::ErrorCode unmount () {
            if (this->m_mounted) {
                PropWare::ErrorCode err;

                if (NULL != this->m_buf.buf) {
                    check_errors(this->m_driver->flush(&this->m_buf));
                    free(this->m_buf.buf);
                    this->m_buf.buf = NULL;
                }

                if (NULL != this->m_fat) {
                    check_errors(this->flush_fat());
                    free(this->m_fat);
                    this->m_fat = NULL;
                }
            }

            return NO_ERROR;
        }

        /**
         * @brief   Determine whether the mounted filesystem is FAT16 or FAT32
         *
         * @return  2 (also known as PropWare::FatFS::FAT16) for FAT16, 4 (also known as PropWare::FatFS::FAT32) for
         *          FAT32
         */
        uint8_t get_fs_type () {
            return this->m_filesystem;
        }

    private:
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
        static const uint8_t  LABEL_ADDR             = 0x47;
        static const uint8_t  SEC_PER_CLSTR_ADDR     = 0x0D;
        static const uint8_t  TOT_SCTR_16_ADDR       = 0x13;
        static const uint8_t  FAT_SIZE_16_ADDR       = 0x16;
        static const uint8_t  TOT_SCTR_32_ADDR       = 0x20;
        static const uint8_t  FAT_SIZE_32_ADDR       = 0x24;
        static const uint8_t  ROOT_CLUSTER_ADDR      = 0x2c;
        static const uint16_t FAT12_CLSTR_CNT        = 4085;
        static const uint16_t FAT16_CLSTR_CNT        = UINT16_MAX - 10;

        static const int8_t   FREE_CLUSTER       = 0;  // Cluster is unused
        static const int8_t   RESERVED_CLUSTER   = 1;
        static const int8_t   RSVD_CLSTR_VAL_BEG = -15;  // First reserved cluster value
        static const int8_t   RSVD_CLSTR_VAL_END = -10;  // Last reserved cluster value
        static const int8_t   BAD_CLUSTER        = -9;  // Cluster is corrupt
        static const int32_t  EOC_BEG            = -8;  // First marker for end-of-chain (end of file entry within FAT)
        static const int32_t  EOC_END            = -1;  // Last marker for end-of-chain
        static const uint32_t EOC_MASK           = 0x0fffffff;

    private:
        typedef struct {
            uint8_t  numFATs;
            uint32_t rootEntryCount;
            uint32_t rootDirSectors;
            uint32_t rsvdSectorCount;
            uint32_t totalSectors;
            uint32_t FATSize;
            uint32_t dataSectors;
            uint32_t bootSector;
            uint32_t clusterCount;
        }                     InitFATInfo;

    private:

        /**
         * @brief       Read the master boot record and load in the boot sector for the requested partition
         *
         * @param[in]   partition   0-indexed description of the requested partition
         *
         * @return 0 upon success, error code otherwise
         */
        inline PropWare::ErrorCode read_boot_sector (const uint8_t partition) {
            PropWare::ErrorCode err;

            // Read first sector into the default buffer
            check_errors(this->m_driver->read_data_block(0, this->m_buf.buf));
            const uint8_t bootSectorId = this->m_driver->get_byte(BOOT_SECTOR_ID_ADDR, this->m_buf.buf);

            // If we don't have a master boot record, the job is simple...
            if (BOOT_SECTOR_ID == bootSectorId) {
                // Make sure the user requested partition 0 and then call it done
                if (0 == partition)
                    this->m_initFatInfo.bootSector = 0;
                else
                    // A boot sector at address 0 means only one partition exists on the entire device
                    return PARTITION_DOES_NOT_EXIST;
            }
                // Sector 0 is the master boot record - great. Parse the partition table and read in the boot sector
            else {
                const uint16_t partitionRow = PARTITION_TABLE_START + (partition << 4);
                check_errors(this->is_fat_volume(this->m_buf.buf[partitionRow + PARTITION_ID_OFFSET]));
                this->m_initFatInfo.bootSector = this->m_driver->get_long(partitionRow + PARTITION_START_OFFSET,
                                                                          this->m_buf.buf);
                check_errors(this->m_driver->read_data_block(this->m_initFatInfo.bootSector, this->m_buf.buf));
            }

            return 0;
        }

        inline PropWare::ErrorCode is_fat_volume (const uint8_t partitionId) const {
            const static uint8_t PARTITION_IDS[54] = {0x01, 0x04, 0x06, 0x07, 0x08, 0x0B, 0x0C, 0x0E, 0x11, 0x12, 0x14,
                                                      0x16, 0x17, 0x1B, 0x1C, 0x1E, 0x24, 0x27, 0x28, 0x56, 0x84, 0x86,
                                                      0x8B, 0x8D, 0x90, 0x92, 0x97, 0x98, 0x9A, 0xAA, 0xB6, 0xBB, 0xBC,
                                                      0xC0, 0xC1, 0xC6, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCE, 0xD0, 0xD1,
                                                      0xD4, 0xD6, 0xDB, 0xDE, 0xE1, 0xE4, 0xE5, 0xEF, 0xF2, 0xFE};

            for (uint8_t i = 0; i < sizeof(PARTITION_IDS); ++i)
                if (PARTITION_IDS[i] == partitionId) {
                    return NO_ERROR;
                }

            return UNSUPPORTED_FILESYSTEM;
        }

        inline PropWare::ErrorCode common_boot_sector_parser () {
            // This makes the code much easier to read
            InitFATInfo *i = &this->m_initFatInfo;

            // Number of entries in the root directory
            i->rootEntryCount = this->m_driver->get_short(ROOT_ENTRY_CNT_ADDR, this->m_buf.buf);

            // Number of sectors in the root directory
            i->rootDirSectors = (i->rootEntryCount * 32 + this->m_driver->get_sector_size() - 1) >>
                    this->m_driver->get_sector_size_shift();

            // Get the reserved sector count
            i->rsvdSectorCount = this->m_driver->get_short(RSVD_SCTR_CNT_ADDR, this->m_buf.buf);

            // Total number of FATs
            i->numFATs = this->m_driver->get_byte(NUM_FATS_ADDR, this->m_buf.buf);
            if (2 != i->numFATs)
                return TOO_MANY_FATS;

            memcpy(this->m_label, &this->m_buf.buf[LABEL_ADDR], 8);
            this->m_label[8] = '\0';

            uint8_t sectorsPerCluster = this->m_driver->get_byte(SEC_PER_CLSTR_ADDR, this->m_buf.buf);
            switch (sectorsPerCluster) {
                case 1:
                    this->m_tier1sPerTier2Shift = 0;
                    break;
                case 2:
                    this->m_tier1sPerTier2Shift = 1;
                    break;
                case 4:
                    this->m_tier1sPerTier2Shift = 2;
                    break;
                case 8:
                    this->m_tier1sPerTier2Shift = 3;
                    break;
                case 16:
                    this->m_tier1sPerTier2Shift = 4;
                    break;
                case 32:
                    this->m_tier1sPerTier2Shift = 5;
                    break;
                case 64:
                    this->m_tier1sPerTier2Shift = 6;
                    break;
                case 128:
                    this->m_tier1sPerTier2Shift = 7;
                    break;
                default:
                    pwOut.printf("Sectors per cluster (bad) = %u\n", sectorsPerCluster);
                    return BAD_SECTORS_PER_CLUSTER;
            }

            return 0;
        }

        inline void partition_info_parser () {
            // This makes the code much easier to read
            InitFATInfo *i = &this->m_initFatInfo;

            // Check if FAT size is valid in 16- or 32-bit location
            uint32_t fatSize = this->m_driver->get_short(FAT_SIZE_16_ADDR, this->m_buf.buf);
            if (!fatSize)
                fatSize = this->m_driver->get_long(FAT_SIZE_32_ADDR, this->m_buf.buf);
            i->FATSize = fatSize;

            // Check if FAT16 total sectors is valid
            uint32_t totalSectors = this->m_driver->get_short(TOT_SCTR_16_ADDR, this->m_buf.buf);
            if (!totalSectors)
                totalSectors = this->m_driver->get_long(TOT_SCTR_32_ADDR, this->m_buf.buf);
            i->totalSectors = totalSectors;



            // Compute necessary numbers to determine FAT type (12/16/32)
            i->dataSectors         = totalSectors - (i->rsvdSectorCount + i->numFATs * fatSize + i->rootDirSectors);
            i->clusterCount        = i->dataSectors >> this->m_tier1sPerTier2Shift;
            this->m_rootDirSectors = (i->rootEntryCount * 32) >> this->m_driver->get_sector_size_shift();
        }

        inline PropWare::ErrorCode determine_fat_type () {
            // Determine and store FAT type
            if (FAT12_CLSTR_CNT > this->m_initFatInfo.clusterCount)
                return UNSUPPORTED_FILESYSTEM;
            else if (FAT16_CLSTR_CNT > this->m_initFatInfo.clusterCount) {
                this->m_filesystem                = FAT_16;
                this->m_entriesPerFatSector_Shift = 8; // FAT sectors contain 256 entries (2 bytes each)
            } else {
                this->m_filesystem                = FAT_32;
                this->m_entriesPerFatSector_Shift = 7; // FAT sectors contain 128 entries (4 bytes each)
            }

            return 0;
        }

        inline void store_root_info () {
            // Find start of FAT
            this->m_fatStart = this->m_initFatInfo.bootSector + this->m_initFatInfo.rsvdSectorCount;

            //    this->m_filesystem = FAT_16;
            // Find root directory address
            switch (this->m_filesystem) {
                case FAT_16:
                    this->m_rootAddr      = this->m_initFatInfo.FATSize * this->m_initFatInfo.numFATs
                            + this->m_fatStart;
                    this->m_firstDataAddr = this->m_rootAddr + this->m_rootDirSectors;
                    break;
                case FAT_32:
                    this->m_firstDataAddr = this->m_rootAddr = this->m_initFatInfo.bootSector +
                            this->m_initFatInfo.rsvdSectorCount + this->m_initFatInfo.FATSize
                            * this->m_initFatInfo.numFATs;
                    this->m_rootAllocUnit = this->m_driver->get_long(ROOT_CLUSTER_ADDR, this->m_buf.buf);
                    break;
            }

            // If files will be writable, the second FAT must also be updated -
            // the first sector's address is stored here
            this->m_fatSize = this->m_initFatInfo.FATSize;
        }

        inline PropWare::ErrorCode read_fat_and_root_sectors () {
            PropWare::ErrorCode err;

            // Store the first sector of the FAT
            check_errors(this->m_driver->read_data_block(this->m_fatStart, this->m_fat));
            this->m_curFatSector = 0;

            // Read in the root directory, set root as current
            check_errors(this->m_driver->read_data_block(this->m_rootAddr, this->m_buf.buf));
            this->m_buf.meta->curTier2Addr = this->m_rootAddr;
            if (FAT_16 == this->m_filesystem) {
                this->m_dir_firstCluster   = (uint32_t) -1;
                this->m_buf.meta->curTier2 = (uint32_t) -1;
            } else {
                this->m_buf.meta->curTier2 = this->m_dir_firstCluster = this->m_rootAllocUnit;
                check_errors(this->get_fat_value(this->m_buf.meta->curTier2, &this->m_buf.meta->nextTier2));
            }
            this->m_buf.meta->curTier2Addr   = this->m_rootAddr;
            this->m_buf.meta->curTier1Offset = 0;

            return 0;
        }

        bool is_eoc (int32_t value) const {
            switch (this->m_filesystem) {
                case FAT_16:
                    return EOC_END == value;
                case FAT_32:
                    value |= 0xf0000000;
                    return EOC_BEG <= value && EOC_END <= value;
                default:
                    return false;
            }
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
            if ((fatEntry >> this->m_entriesPerFatSector_Shift) != this->m_curFatSector) {
                this->flush_fat();
                this->m_curFatSector = fatEntry >> this->m_entriesPerFatSector_Shift;
                check_errors(this->m_driver->read_data_block(this->m_curFatSector + this->m_fatStart, this->m_fat));
            }
            firstAvailableAllocUnit = this->m_curFatSector << this->m_entriesPerFatSector_Shift;

            // The necessary FAT sector has been loaded and the next allocation
            // unit is known, proceed with loading the next data sector and
            // incrementing the cluster variables

            // Retrieve the next allocation unit number
            if (FAT_16 == this->m_filesystem) {
                *value = this->m_driver->get_short((uint16_t) ((fatEntry - firstAvailableAllocUnit) << 1), this->m_fat);
                *value &= WORD_0;
            } else if (FAT_32 == this->m_filesystem) {
                *value = this->m_driver->get_long((uint16_t) ((fatEntry - firstAvailableAllocUnit) << 2), this->m_fat);
                // Clear the highest 4 bits - they are always reserved
                *value &= 0x0FFFFFFF;
            }

            return 0;
        }

        /**
         * @brief       Find and return the starting sector's address for a
         *              given allocation unit (note - not cluster)
         *
         * @param[in]   allocUnit   Allocation unit in FAT filesystem
         *
         * @return      Returns sector address of the desired allocation unit
         */
        uint32_t compute_tier1_from_tier2 (uint32_t tier2) const {
            if (FatFS::FAT_32 == this->m_filesystem)
                tier2 -= this->m_rootAllocUnit;
            else
                tier2 -= 2;
            tier2 <<= this->m_tier1sPerTier2Shift;
            tier2 += this->m_firstDataAddr;
            return tier2;
        }

        /**
         * @brief   Enlarge the current directory
         */
        PropWare::ErrorCode extend_current_directory () {
            return this->extend_fat(&this->m_buf);
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
            uint32_t            newAllocUnit;

            // Do we need to load a different sector of the FAT or is the correct one currently loaded? (Correct means
            // the sector currently containing the EOC marker)
            if ((buf->meta->curTier2 >> this->m_entriesPerFatSector_Shift) != this->m_curFatSector) {
                this->flush_fat();
                this->m_curFatSector = buf->meta->curTier2 >> this->m_entriesPerFatSector_Shift;
                check_errors(this->m_driver->read_data_block(this->m_curFatSector + this->m_fatStart, this->m_fat));
            }

            // This function should only be called when a file or directory has
            // reached the end of its cluster chain
            uint16_t entriesPerFatSector = (uint16_t) (1 << this->m_entriesPerFatSector_Shift);
            uint16_t allocUnitOffset     = (uint16_t) (buf->meta->curTier2 % entriesPerFatSector);
            uint16_t fatPointerAddress   = allocUnitOffset * this->m_filesystem;
            uint32_t nextSector          = this->m_driver->get_long(fatPointerAddress, this->m_fat);
            if (this->is_eoc(nextSector))
                return INVALID_FAT_APPEND;

            // Find where the next cluster of the file should be stored...
            newAllocUnit = this->find_empty_space(1);

            // Now that we know the allocation unit, write it to the FAT buffer
            const uint16_t sectorOffset = (uint16_t) ((buf->meta->curTier2 %
                    (1 << this->m_entriesPerFatSector_Shift)) * this->m_filesystem);
            if (FAT_16 == this->m_filesystem)
                this->m_driver->write_short(sectorOffset, this->m_fat, (uint16_t) newAllocUnit);
            else
                this->m_driver->write_long(sectorOffset, this->m_fat, newAllocUnit);
            buf->meta->nextTier2 = newAllocUnit;
            this->m_fatMod       = true;  // And mark the buffer as modified

            return 0;
        }

        /**
         * @brief       Find the first empty allocation unit in the FAT
         *
         * The value of the first empty allocation unit is returned and its location will contain the end-of-chain
         * marker, SD_EOC_END.
         *
         * NOTE: It is important to realize that, though the new entry now contains an EOC marker, this function
         * does not know what cluster is being extended and therefore the calling function must modify the previous
         * EOC to contain the return value
         *
         * @param[in]   restore     If non-zero, the original fat-sector will be restored to m_fat before returning;
         *                          if zero, the last-used sector will remain loaded
         *
         * @return      Returns the number of the first unused allocation unit
         */
        uint32_t find_empty_space (const uint8_t restore) {
            uint16_t allocOffset   = 0;
            uint32_t fatSectorAddr = this->m_curFatSector + this->m_fatStart;
            uint32_t retVal;
            // NOTE: this->m_curFatSector is not modified until end of function - it is used throughout this function as
            // the original starting point

            // Find the first empty allocation unit and write the EOC marker
            if (FAT_16 == this->m_filesystem) {
                // Loop until we find an empty cluster
                while (this->m_driver->get_short(allocOffset, this->m_fat)) {
                    // Stop when we either reach the end of the current block or find an empty cluster
                    while (this->m_driver->get_short(allocOffset, this->m_fat) && (this->m_sectorSize > allocOffset))
                        allocOffset += FAT_16;
                    // If we reached the end of a sector...
                    if (this->m_sectorSize <= allocOffset) {
                        // Read the next fat sector
                        this->flush_fat();
                        this->m_driver->read_data_block(++fatSectorAddr, this->m_fat);
                    }
                }
                this->m_driver->write_short(allocOffset, this->m_fat, (uint16_t) EOC_END);
                this->m_fatMod = true;
            } else /* Implied: "if (FAT_32 == this->m_filesystem)" */{
                // In FAT32, the first 7 usable clusters seem to be un-officially reserved for the root directory
                if (0 == this->m_curFatSector)
                    // 9 comes from the 7 un-officially reserved + 2 for the standard reservation
                    allocOffset = (uint16_t) (9 * FAT_32);

                // Loop until we find an empty cluster
                while (this->m_driver->get_long(allocOffset, this->m_fat) & EOC_MASK) {
                    // Stop when we either reach the end of the current block or find an empty cluster
                    while ((this->m_driver->get_long(allocOffset, this->m_fat) & EOC_MASK)
                            && (this->m_sectorSize > allocOffset))
                        allocOffset += FAT_32;

                    // If we reached the end of a sector...
                    if (this->m_sectorSize <= allocOffset) {
                        // Read the next fat sector
                        this->flush_fat();
                        this->m_driver->read_data_block(++fatSectorAddr, this->m_fat);
                        allocOffset = 0;
                    }
                }

                this->m_driver->write_long(allocOffset, this->m_fat, ((uint32_t) EOC_END) & EOC_MASK);
                this->m_fatMod = true;
            }

            // If we loaded a new fat sector (and then modified it directly
            // above), write the sector before re-loading the original
            if ((fatSectorAddr != (this->m_curFatSector + this->m_fatStart)) && this->m_fatMod) {
                this->flush_fat();
                this->m_driver->read_data_block(this->m_curFatSector + this->m_fatStart, this->m_fat);
            } else
                this->m_curFatSector = fatSectorAddr - this->m_fatStart;

            // Return new address to end-of-chain
            retVal = this->m_curFatSector << this->m_entriesPerFatSector_Shift;
            retVal += allocOffset / this->m_filesystem;
            return retVal;
        }

        PropWare::ErrorCode flush_fat () {
            PropWare::ErrorCode err;
            if (m_fatMod) {
                check_errors(m_driver->write_data_block(this->m_fatStart + this->m_curFatSector, this->m_fat));
                check_errors(m_driver->write_data_block(this->m_fatStart + this->m_curFatSector + this->m_fatSize,
                                                        m_fat));
                m_fatMod = false;
            }

            return NO_ERROR;
        }

        /**
         * @brief       Remove the linked list of allocation units from the FAT (clear space)
         *
         * @param[in]   head    First allocation unit
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode clear_chain (const uint32_t head) {
            PropWare::ErrorCode err;

            uint32_t next = head;
            do {
                const uint32_t current = next;
                check_errors(this->get_fat_value(current, &next));

                const uint32_t firstAvailableAllocUnit = this->m_curFatSector << this->m_entriesPerFatSector_Shift;
                const uint16_t sectorOffset            = (uint16_t) (current - firstAvailableAllocUnit);

                if (FAT_16 == this->m_filesystem)
                    this->m_driver->write_short(sectorOffset << 1, this->m_fat, 0);
                else if (FAT_32 == this->m_filesystem)
                    this->m_driver->write_long(sectorOffset << 2, this->m_fat, 0);
            } while (!this->is_eoc(next));

            this->m_fatMod = true;

            return NO_ERROR;
        }

        void print_status (const bool printBlocks = false, const uint8_t blockLineLength = 16) {
            this->m_logger->println("######################################################");
            this->m_logger->printf("# FAT Filesystem Status - PropWare::FatFS@0x%08X #\n", (unsigned int) this);
            // DRIVER
            this->m_logger->println("Driver");
            this->m_logger->println("======");
            this->m_logger->printf("Driver address: 0x%08X\n", (unsigned int) this->m_driver);
            this->m_logger->printf("Block size: %u\n", this->m_sectorSize);
            this->m_logger->printf("Blocks-per-cluster shift: %u\n", this->m_tier1sPerTier2Shift);
            this->m_logger->println();

            // FILESYSTEM
            this->m_logger->println("Filesystem Constants");
            this->m_logger->println("====================");
            if (!this->m_mounted) {
                this->m_logger->println("\nNot mounted");
                return;
            }

            // FILESYSTEM - FAT Initialization Constants
            this->m_logger->println("\tInitialization Numbers");
            this->m_logger->println("\t----------------------------");
            this->m_logger->printf("\tNumber of FATs: %u\n", this->m_initFatInfo.numFATs);
            this->m_logger->printf("\tRoot entry count: 0x%08X/%u\n", this->m_initFatInfo.rootEntryCount,
                                   this->m_initFatInfo.rootEntryCount);
            this->m_logger->printf("\tRoot dir sectors: 0x%08X/%u\n", this->m_initFatInfo.rootDirSectors,
                                   this->m_initFatInfo.rootDirSectors);
            this->m_logger->printf("\tReserved sector count: 0x%08X/%u\n", this->m_initFatInfo.rsvdSectorCount,
                                   this->m_initFatInfo.rsvdSectorCount);
            this->m_logger->printf("\tTotal sectors: 0x%08X/%u\n", this->m_initFatInfo.totalSectors,
                                   this->m_initFatInfo.totalSectors);
            this->m_logger->printf("\tFAT Start: 0x%08X/%u\n", this->m_fatStart, this->m_fatStart);
            this->m_logger->printf("\tFAT size: 0x%08X/%u\n", this->m_initFatInfo.FATSize, this->m_initFatInfo
                    .FATSize);
            this->m_logger->printf("\tData sectors: 0x%08X/%u\n", this->m_initFatInfo.dataSectors,
                                   this->m_initFatInfo.dataSectors);
            this->m_logger->printf("\tBoot sector: 0x%08X/%u\n", this->m_initFatInfo.bootSector,
                                   this->m_initFatInfo.bootSector);
            this->m_logger->printf("\tCluster count: 0x%08X/%u\n", this->m_initFatInfo.clusterCount,
                                   this->m_initFatInfo.clusterCount);
            this->m_logger->println();

            // FILESYSTEM - Partition
            this->m_logger->println("\tPartition");
            this->m_logger->println("\t=========");
            this->m_logger->printf("\tLabel: %s\n", this->m_label);
            switch (this->m_filesystem) {
                case FAT_32:
                    this->m_logger->printf("\tFilesystem: FAT 32\n");
                    break;
                case FAT_16:
                    this->m_logger->printf("\tFilesystem: FAT 16\n");
                    break;
                default:
                    this->m_logger->printf("\tFilesystem: unknown (%d)\n", this->m_filesystem);
            }
            this->m_logger->printf("\tFirst FAT sector: 0x%08X\n", this->m_fatStart);
            this->m_logger->printf("\tRoot directory alloc. unit: 0x%08X\n", this->m_rootAllocUnit);
            this->m_logger->printf("\tCalculated root directory sector: 0x%08X\n",
                                   this->compute_tier1_from_tier2(this->m_rootAllocUnit));
            this->m_logger->printf("\tRoot directory sector: 0x%08X\n", this->m_rootAddr);
            this->m_logger->printf("\tRoot directory size (in sectors): %u\n", this->m_rootDirSectors);
            this->m_logger->printf("\tFirst data sector: 0x%08X\n", this->m_firstDataAddr);
            this->m_logger->println();

            if (printBlocks) {
                this->m_logger->println("\tFAT Buffer");
                this->m_logger->println("\t----------");
                BlockStorage::print_block(*this->m_logger, this->m_fat, this->m_sectorSize, blockLineLength);
                this->m_logger->println();
            }

            this->m_logger->println("\tCommon Buffer");
            this->m_logger->println("\t=============");
            if (this->m_buf.buf == NULL)
                this->m_logger->println("\tEmpty");
            else {
                BlockStorage::MetaData *bufMeta = this->m_buf.meta;
                this->m_logger->printf("\tID: %d\n", bufMeta->id);
                this->m_logger->printf("\tModdified: %s\n", Utility::to_string(bufMeta->mod));
                this->m_logger->printf("\tCur. cluster's start sector: 0x%08X/%u\n", bufMeta->curTier2Addr,
                                       bufMeta->curTier2Addr);
                this->m_logger->printf("\tCur. sector offset from cluster start: %u\n", bufMeta->curTier1Offset);
                this->m_logger->printf("\tCurrent allocation unit: 0x%08X/%u\n", bufMeta->curTier2, bufMeta->curTier2);
                this->m_logger->printf("\tNext allocation unit: 0x%08X/%u\n", bufMeta->nextTier2, bufMeta->nextTier2);
                if (printBlocks)
                    BlockStorage::print_block(*this->m_logger, this->m_buf, this->m_sectorSize, blockLineLength);
            }
            this->m_logger->println();
        }

    private:
        InitFATInfo m_initFatInfo;
        uint8_t     m_filesystem;  // File system type - one of FAT_16 or FAT_32
        char        m_label[9]; // Filesystem label
        uint32_t    m_fatStart;  // Starting block address of the FAT
        uint32_t    m_rootAllocUnit;  // Allocation unit of root directory/first data sector (FAT32 only)
        uint32_t    m_rootAddr;  // Starting block address of the root directory
        uint32_t    m_rootDirSectors;  // Number of sectors for the root directory
        uint32_t    m_firstDataAddr;  // Starting block address of the first data cluster
        uint32_t    m_fatSize;
        uint16_t    m_entriesPerFatSector_Shift;  // How many FAT entries are in a single sector of the FAT
        uint8_t     *m_fat;  // Buffer for FAT entries only
        bool m_fatMod;

        uint32_t m_curFatSector;  // Store the current FAT sector loaded into m_fat
        uint32_t m_dir_firstCluster;  // Store the current directory's starting allocation unit
};

}
