/**
 * @file        PropWare/filesystem/fat/fatfs.h
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

#include <PropWare/filesystem/fat/readonlyfatfs.h>
#include <PropWare/filesystem/filesystem.h>

namespace PropWare {

class FatFS : virtual public ReadOnlyFatFS,
              virtual public Filesystem {
        friend class FatFileWriter;

    public:
        FatFS (const BlockStorageReader &readDriver, const BlockStorageWriter &writeDriver,
               const Printer &logger = pwOut)
                : ReadOnlyFilesystem(readDriver, logger),
                  ReadOnlyFatFS(readDriver, logger),
                  Filesystem(readDriver, writeDriver, logger),
                  m_fatMod(false) {
        }

        virtual ~FatFS () {
            this->unmount();
        }

        virtual PropWare::ErrorCode mount (const uint8_t partition = 0) {
            this->m_fatMod = false;
            return this->ReadOnlyFatFS::mount(partition);
        }

        /**
         * @see PropWare::Filesystem::unmount
         */
        PropWare::ErrorCode unmount () {
            if (this->m_mounted) {
                PropWare::ErrorCode err;

                if (NULL != this->m_buf.buf) {
                    check_errors(this->m_writeDriver->flush(&this->m_buf));
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

    private:

        /**
         * @brief   Enlarge the current directory
         */
        PropWare::ErrorCode extend_current_directory () {
            return this->extend_fat(this->m_buf.meta);
        }

        /**
         * @brief       Enlarge a file or directory by one cluster
         *
         * @param[in]   *bufferMetadata     Address of the buffer (containing information for a file or directory) to be
         *                                  enlarged
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode extend_fat (BlockStorage::MetaData *bufferMetadata) {
            PropWare::ErrorCode err;
            uint32_t            newAllocUnit;

            // Do we need to load a different sector of the FAT or is the correct one currently loaded? (Correct means
            // the sector currently containing the EOC marker)
            if ((bufferMetadata->curTier2 >> this->m_entriesPerFatSector_Shift) != this->m_curFatSector) {
                this->flush_fat();
                this->m_curFatSector = bufferMetadata->curTier2 >> this->m_entriesPerFatSector_Shift;
                check_errors(this->m_readDriver->read_data_block(this->m_curFatSector + this->m_fatStart, this->m_fat));
            }

            // This function should only be called when a file or directory has
            // reached the end of its cluster chain
            uint16_t entriesPerFatSector = (uint16_t) (1 << this->m_entriesPerFatSector_Shift);
            uint16_t allocUnitOffset     = (uint16_t) (bufferMetadata->curTier2 % entriesPerFatSector);
            uint16_t fatPointerAddress   = allocUnitOffset * this->m_filesystem;
            uint32_t nextSector          = this->m_readDriver->get_long(fatPointerAddress, this->m_fat);
            if (!this->is_eoc(nextSector))
                return INVALID_FAT_APPEND;

            // Find where the next cluster of the file should be stored...
            newAllocUnit = this->find_empty_space(1);

            // Now that we know the allocation unit, write it to the FAT buffer
            const uint16_t sectorOffset = (uint16_t) ((bufferMetadata->curTier2 %
                    (1 << this->m_entriesPerFatSector_Shift)) * this->m_filesystem);
            if (FAT_16 == this->m_filesystem)
                this->m_writeDriver->write_short(sectorOffset, this->m_fat, (uint16_t) newAllocUnit);
            else
                this->m_writeDriver->write_long(sectorOffset, this->m_fat, newAllocUnit);
            bufferMetadata->nextTier2 = newAllocUnit;
            this->m_fatMod            = true;  // And mark the buffer as modified

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
                while (this->m_readDriver->get_short(allocOffset, this->m_fat)) {
                    // Stop when we either reach the end of the current block or find an empty cluster
                    while (this->m_readDriver->get_short(allocOffset, this->m_fat)
                            && (this->m_sectorSize > allocOffset))
                        allocOffset += FAT_16;
                    // If we reached the end of a sector...
                    if (this->m_sectorSize <= allocOffset) {
                        // Read the next fat sector
                        this->flush_fat();
                        this->m_readDriver->read_data_block(++fatSectorAddr, this->m_fat);
                    }
                }
                this->m_writeDriver->write_short(allocOffset, this->m_fat, (uint16_t) EOC_END);
                this->m_fatMod = true;
            } else /* Implied: "if (FAT_32 == this->m_filesystem)" */{
                // In FAT32, the first 7 usable clusters seem to be un-officially reserved for the root directory
                if (0 == this->m_curFatSector)
                    // 9 comes from the 7 un-officially reserved + 2 for the standard reservation
                    allocOffset = (uint16_t) (9 * FAT_32);

                // Loop until we find an empty cluster
                while (this->m_readDriver->get_long(allocOffset, this->m_fat) & EOC_MASK) {
                    // Stop when we either reach the end of the current block or find an empty cluster
                    while ((this->m_readDriver->get_long(allocOffset, this->m_fat) & EOC_MASK)
                            && (this->m_sectorSize > allocOffset))
                        allocOffset += FAT_32;

                    // If we reached the end of a sector...
                    if (this->m_sectorSize <= allocOffset) {
                        // Read the next fat sector
                        this->flush_fat();
                        this->m_readDriver->read_data_block(++fatSectorAddr, this->m_fat);
                        allocOffset = 0;
                    }
                }

                this->m_writeDriver->write_long(allocOffset, this->m_fat, ((uint32_t) EOC_END) & EOC_MASK);
                this->m_fatMod = true;
            }

            // If we loaded a new fat sector (and then modified it directly
            // above), write the sector before re-loading the original
            if ((fatSectorAddr != (this->m_curFatSector + this->m_fatStart)) && this->m_fatMod) {
                this->flush_fat();
                this->m_readDriver->read_data_block(this->m_curFatSector + this->m_fatStart, this->m_fat);
            } else
                this->m_curFatSector = fatSectorAddr - this->m_fatStart;

            // Return new address to end-of-chain
            retVal = this->m_curFatSector << this->m_entriesPerFatSector_Shift;
            retVal += allocOffset / this->m_filesystem;
            return retVal;
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
                    this->m_writeDriver->write_short(sectorOffset << 1, this->m_fat, 0);
                else if (FAT_32 == this->m_filesystem)
                    this->m_writeDriver->write_long(sectorOffset << 2, this->m_fat, 0);
            } while (!this->is_eoc(next));

            this->m_fatMod = true;

            return NO_ERROR;
        }

        virtual PropWare::ErrorCode flush_fat () {
            PropWare::ErrorCode err;
            if (m_fatMod) {
                check_errors(m_writeDriver->write_data_block(this->m_fatStart + this->m_curFatSector, this->m_fat));
                check_errors(m_writeDriver->write_data_block(this->m_fatStart + this->m_curFatSector + this->m_fatSize,
                                                             m_fat));
                m_fatMod = false;
            }

            return NO_ERROR;
        }

    private:
        bool m_fatMod;
};

}
