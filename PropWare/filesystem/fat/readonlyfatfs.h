/**
 * @file        PropWare/filesystem/fat/readonlyfatfs.h
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
#include <PropWare/c++allocate.h>
#include <PropWare/utility/utility.h>
#include <PropWare/hmi/output/printer.h>
#include <PropWare/filesystem/readonlyfilesystem.h>

namespace PropWare {

/**
 * FAT 16/32 filesystem driver - can be used with SD cards or any other PropWare::BlockStorage device
 */
class ReadOnlyFatFS : virtual public ReadOnlyFilesystem {
        friend class FatFile;

        friend class FatFileReader;

        friend class FatFileWriter;

    public:
        typedef enum {
                                   NO_ERROR        = 0,
                                   BEG_ERROR       = ReadOnlyFilesystem::END_ERROR + 1,
            /** FatFS Error 0 */   EMPTY_FAT_ENTRY = BEG_ERROR,
            /** FatFS Error 1 */   INVALID_FAT_APPEND,
            /** FatFS Error 2 */   TOO_MANY_FATS,
            /** FatFS Error 3 */   BAD_SECTORS_PER_CLUSTER,
            /** FatFS Error 4 */   READING_PAST_EOC,
            /** FatFS Error 5 */   PARTITION_DOES_NOT_EXIST,
            /** FatFS Error 6 */   UNSUPPORTED_FILESYSTEM,
            /** Last FatFS error */END_ERROR       = UNSUPPORTED_FILESYSTEM
        } ErrorCode;

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
        ReadOnlyFatFS (const BlockStorageReader &driver, const Printer &logger = pwOut)
                : ReadOnlyFilesystem(driver, logger),
                  m_fat(NULL) {
        }

        /**
         * @brief   Destructor. Unmounts the filesystem and flushes all buffers
         */
        virtual ~ReadOnlyFatFS () {
            if (NULL != this->m_buf.buf)
                free(this->m_buf.buf);

            if (NULL != this->m_fat)
                free(this->m_fat);
        }

        /**
         * @see PropWare::Filesystem::mount
         */
        virtual PropWare::ErrorCode mount (const uint8_t partition = 0) {
            PropWare::ErrorCode err;

            if (this->m_mounted)
                return ReadOnlyFatFS::FILESYSTEM_ALREADY_MOUNTED;
            else if (3 < partition)
                return UNSUPPORTED_FILESYSTEM;

            // Start the driver
            check_errors(this->m_readDriver->start());
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
         * @brief   Determine whether the mounted filesystem is FAT16 or FAT32
         *
         * @return  2 (also known as PropWare::FatFS::FAT16) for FAT16, 4 (also known as PropWare::FatFS::FAT32) for
         *          FAT32
         */
        uint8_t get_fs_type () {
            return this->m_filesystem;
        }

    protected:
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
         * @param[in]   fatEntry    Entry number (cluster) to read in
         *                          the FAT
         * @param[out]  *value      Address to store the value into (the next cluster)
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode get_fat_value (const uint32_t fatEntry, uint32_t *value) {
            PropWare::ErrorCode err;
            uint32_t            firstAvailableCluster;

            // Do we need to load a new fat sector?
            if ((fatEntry >> this->m_entriesPerFatSector_Shift) != this->m_curFatSector) {
                this->flush_fat();
                this->m_curFatSector = fatEntry >> this->m_entriesPerFatSector_Shift;
                check_errors(this->m_readDriver->read_data_block(this->m_curFatSector + this->m_fatStart, this->m_fat));
            }
            firstAvailableCluster = this->m_curFatSector << this->m_entriesPerFatSector_Shift;

            // The necessary FAT sector has been loaded and the next cluster is known, proceed with loading
            // the next data sector and incrementing the cluster variables

            // Retrieve the next cluster number
            if (FAT_16 == this->m_filesystem) {
                *value = this->m_readDriver->get_short((uint16_t) ((fatEntry - firstAvailableCluster) << 1), this->m_fat);
                *value &= WORD_0;
            } else if (FAT_32 == this->m_filesystem) {
                *value = this->m_readDriver->get_long((uint16_t) ((fatEntry - firstAvailableCluster) << 2), this->m_fat);
                // Clear the highest 4 bits - they are always reserved
                *value &= 0x0FFFFFFF;
            }

            return 0;
        }

    protected:
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
            check_errors(this->m_readDriver->read_data_block(0, this->m_buf.buf));
            const uint8_t bootSectorId = this->m_readDriver->get_byte(BOOT_SECTOR_ID_ADDR, this->m_buf.buf);

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
                this->m_initFatInfo.bootSector = this->m_readDriver->get_long(partitionRow + PARTITION_START_OFFSET,
                                                                          this->m_buf.buf);
                check_errors(this->m_readDriver->read_data_block(this->m_initFatInfo.bootSector, this->m_buf.buf));
            }

            return 0;
        }

        inline PropWare::ErrorCode is_fat_volume (const uint8_t partitionId) const {
            const static uint8_t PARTITION_IDS[54] = {0x01, 0x04, 0x06, 0x07, 0x08, 0x0B, 0x0C, 0x0E, 0x11, 0x12, 0x14,
                                                      0x16, 0x17, 0x1B, 0x1C, 0x1E, 0x24, 0x27, 0x28, 0x56, 0x84, 0x86,
                                                      0x8B, 0x8D, 0x90, 0x92, 0x97, 0x98, 0x9A, 0xAA, 0xB6, 0xBB, 0xBC,
                                                      0xC0, 0xC1, 0xC6, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCE, 0xD0, 0xD1,
                                                      0xD4, 0xD6, 0xDB, 0xDE, 0xE1, 0xE4, 0xE5, 0xEF, 0xF2, 0xFE};
            for (auto            id : PARTITION_IDS)
                if (id == partitionId)
                    return NO_ERROR;
            return UNSUPPORTED_FILESYSTEM;
        }

        inline PropWare::ErrorCode common_boot_sector_parser () {
            // This makes the code much easier to read
            InitFATInfo *i = &this->m_initFatInfo;

            // Number of entries in the root directory
            i->rootEntryCount = this->m_readDriver->get_short(ROOT_ENTRY_CNT_ADDR, this->m_buf.buf);

            // Number of sectors in the root directory
            i->rootDirSectors = (i->rootEntryCount * 32 + this->m_readDriver->get_sector_size() - 1) >>
                    this->m_readDriver->get_sector_size_shift();

            // Get the reserved sector count
            i->rsvdSectorCount = this->m_readDriver->get_short(RSVD_SCTR_CNT_ADDR, this->m_buf.buf);

            // Total number of FATs
            i->numFATs = this->m_readDriver->get_byte(NUM_FATS_ADDR, this->m_buf.buf);
            if (2 != i->numFATs)
                return TOO_MANY_FATS;

            memcpy(this->m_label, &this->m_buf.buf[LABEL_ADDR], 8);
            this->m_label[8] = '\0';

            uint8_t sectorsPerCluster = this->m_readDriver->get_byte(SEC_PER_CLSTR_ADDR, this->m_buf.buf);
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
            uint32_t fatSize = this->m_readDriver->get_short(FAT_SIZE_16_ADDR, this->m_buf.buf);
            if (!fatSize)
                fatSize = this->m_readDriver->get_long(FAT_SIZE_32_ADDR, this->m_buf.buf);
            i->FATSize = fatSize;

            // Check if FAT16 total sectors is valid
            uint32_t totalSectors = this->m_readDriver->get_short(TOT_SCTR_16_ADDR, this->m_buf.buf);
            if (!totalSectors)
                totalSectors = this->m_readDriver->get_long(TOT_SCTR_32_ADDR, this->m_buf.buf);
            i->totalSectors = totalSectors;



            // Compute necessary numbers to determine FAT type (12/16/32)
            i->dataSectors         = totalSectors - (i->rsvdSectorCount + i->numFATs * fatSize + i->rootDirSectors);
            i->clusterCount        = i->dataSectors >> this->m_tier1sPerTier2Shift;
            this->m_rootDirSectors = (i->rootEntryCount * 32) >> this->m_readDriver->get_sector_size_shift();
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
            // Create some variables for legibility's sake
            const uint_fast32_t bootSector      = this->m_initFatInfo.bootSector;
            const uint_fast32_t reservedSectors = this->m_initFatInfo.rsvdSectorCount;
            const uint_fast8_t  numFATs         = this->m_initFatInfo.numFATs;
            this->m_fatSize = this->m_initFatInfo.FATSize;

            // Find start of FAT
            this->m_fatStart = bootSector + reservedSectors;

            // Find root directory address
            this->m_firstDataAddr = this->m_fatStart + this->m_fatSize * numFATs;
            switch (this->m_filesystem) {
                case FAT_16:
                    this->m_rootAddr = this->m_firstDataAddr;
                    this->m_firstDataAddr += this->m_rootDirSectors;
                    break;
                case FAT_32:
                    this->m_rootCluster = this->m_readDriver->get_long(ROOT_CLUSTER_ADDR, this->m_buf.buf);
                    this->m_rootAddr    = this->compute_tier1_from_tier2(this->m_rootCluster);
                    break;
            }
        }

        inline PropWare::ErrorCode read_fat_and_root_sectors () {
            PropWare::ErrorCode err;

            // Store the first sector of the FAT
            check_errors(this->m_readDriver->read_data_block(this->m_fatStart, this->m_fat));
            this->m_curFatSector = 0;

            // Read in the root directory, set root as current
            check_errors(this->m_readDriver->read_data_block(this->m_rootAddr, this->m_buf.buf));
            this->m_buf.meta->curTier2Addr = this->m_rootAddr;
            if (FAT_16 == this->m_filesystem) {
                this->m_buf.meta->curTier2 = (uint32_t) -1;
            } else {
                this->m_buf.meta->curTier2 = this->m_rootCluster;
                check_errors(this->get_fat_value(this->m_buf.meta->curTier2, &this->m_buf.meta->nextTier2));
            }
            this->m_buf.meta->curTier2Addr   = this->m_rootAddr;
            this->m_buf.meta->curTier1Offset = 0;

            return 0;
        }

        /**
         * @brief       Find and return the starting sector's address for a given cluster
         *
         * @param[in]   tier2   Cluster in FAT filesystem
         *
         * @return      Returns sector address of the desired cluster
         */
        uint32_t compute_tier1_from_tier2 (uint32_t tier2) const {
            tier2 -= 2; // Magical offset, as described in section 6.7 of the MS FAT32 spec
            tier2 <<= this->m_tier1sPerTier2Shift;
            tier2 += this->m_firstDataAddr;
            return tier2;
        }

        virtual PropWare::ErrorCode flush_fat () {
            return NO_ERROR;
        }

        void print_status (const bool printBlocks = false) const {
            this->m_logger->println("######################################################");
            this->m_logger->printf("# FAT Filesystem Status - PropWare::FatFS@0x%08X #\n", (unsigned int) this);
            // DRIVER
            this->m_logger->println("Driver");
            this->m_logger->println("======");
            this->m_logger->printf("Driver address: 0x%08X\n", (unsigned int) this->m_readDriver);
            this->m_logger->printf("Block size: %u\n", this->m_sectorSize);
            this->m_logger->printf("Blocks-per-cluster shift: %u\n", this->m_tier1sPerTier2Shift);
            this->m_logger->println();

            // FILESYSTEM
            this->m_logger->println("Filesystem Constants");
            this->m_logger->println("====================");
            if (this->m_mounted) {
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
                this->m_logger->printf("\tFAT size: 0x%08X/%u\n", this->m_initFatInfo.FATSize,
                                       this->m_initFatInfo.FATSize);
                this->m_logger->printf("\tData sectors: 0x%08X/%u\n", this->m_initFatInfo.dataSectors,
                                       this->m_initFatInfo.dataSectors);
                this->m_logger->printf("\tBoot sector: 0x%08X/%u\n", this->m_initFatInfo.bootSector,
                                       this->m_initFatInfo.bootSector);
                this->m_logger->printf("\tCluster count: 0x%08X/%u\n", this->m_initFatInfo.clusterCount,
                                       this->m_initFatInfo.clusterCount);
                this->m_logger->println();

                // FILESYSTEM - Partition
                this->m_logger->println("Partition");
                this->m_logger->println("=========");
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
                this->m_logger->printf("\tRoot directory alloc. unit: 0x%08X\n", this->m_rootCluster);
                this->m_logger->printf("\tCalculated root directory sector: 0x%08X\n",
                                       this->compute_tier1_from_tier2(this->m_rootCluster));
                this->m_logger->printf("\tRoot directory sector: 0x%08X\n", this->m_rootAddr);
                this->m_logger->printf("\tRoot directory size (in sectors): %u\n", this->m_rootDirSectors);
                this->m_logger->printf("\tFirst data sector: 0x%08X\n", this->m_firstDataAddr);
                this->m_logger->println();
            } else {
                this->m_logger->println("\nNot mounted");
            }

            if (printBlocks) {
                this->m_logger->println("FAT Buffer");
                this->m_logger->println("==========");
                BlockStorage::print_block(*this->m_logger, this->m_fat, this->m_sectorSize);
                this->m_logger->println();
            }

            this->m_logger->println("Common Buffer");
            this->m_logger->println("=============");
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
                    BlockStorage::print_block(*this->m_logger, this->m_buf, this->m_sectorSize);
            }
            this->m_logger->println();
        }

    protected:
        InitFATInfo m_initFatInfo;
        uint8_t     m_filesystem;  // File system type - one of FAT_16 or FAT_32
        char        m_label[9]; // Filesystem label
        uint32_t    m_fatStart;  // Starting block address of the FAT
        uint32_t    m_rootCluster;  // Cluster of root directory/first data sector (FAT32 only)
        uint32_t    m_rootAddr;  // Starting block address of the root directory
        uint32_t    m_rootDirSectors;  // Number of sectors for the root directory
        uint32_t    m_firstDataAddr;  // Starting block address of the first data cluster
        uint32_t    m_fatSize;
        uint16_t    m_entriesPerFatSector_Shift;  // How many FAT entries are in a single sector of the FAT
        uint8_t     *m_fat;  // Buffer for FAT entries only

        uint32_t m_curFatSector;  // Store the current FAT sector loaded into m_fat
        uint32_t m_dir_firstCluster; // Used by FatFile
};

}
