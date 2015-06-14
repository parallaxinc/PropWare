/**
 * @file        fatfile.h
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

#include <PropWare/filesystem/file.h>
#include <PropWare/filesystem/fat/fatfs.h>

namespace PropWare {

class FatFile : virtual public File {
    protected:
        static const uint8_t FILE_LEN_OFFSET = 0x1C;  // Length of a file in bytes

        // File/directory values
        static const uint8_t FILE_ENTRY_LENGTH     = 32;  // An entry in a directory uses 32 bytes
        static const uint8_t DELETED_FILE_MARK     = 0xE5;  // Marks that a file has been deleted here, continue to the next entry
        static const uint8_t FILE_NAME_LEN         = 8;  // 8 characters in the standard file name
        static const uint8_t FILE_EXTENSION_LEN    = 3;  // 3 character file name extension
        static const uint8_t FILENAME_STR_LEN      = FILE_NAME_LEN + FILE_EXTENSION_LEN + 2;
        static const uint8_t FILE_ATTRIBUTE_OFFSET = 0x0B;  // Byte of a file entry to store attribute flags
        static const uint8_t FILE_START_CLSTR_LOW  = 0x1A;  // Starting cluster number
        static const uint8_t FILE_START_CLSTR_HIGH = 0x14;  // High word (16-bits) of the starting cluster number (FAT32 only)

        // File attributes (definitions with trailing underscore represent character for a cleared attribute flag)
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

    public:
        const char * get_name () const {
            return this->m_name;
        }

        bool exists () const {
            uint16_t temp = 0;
            return NO_ERROR == this->find(this->get_name(), &temp);
        }

        bool exists (PropWare::ErrorCode &err) const {
            uint16_t temp = 0;
            err = this->find(this->get_name(), &temp);
            return NO_ERROR == err;
        }

    protected:
        FatFile (FatFS &fs, const char name[], BlockStorage::Buffer *buffer = NULL, const Printer &logger = pwOut)
                : File(fs, name, buffer, logger),
                  m_fs(&fs) {
            strcpy(this->m_name, name);
            Utility::to_upper(this->m_name);
        }

        const uint8_t get_file_attributes (uint16_t fileEntryOffset) const {
            return this->m_buf->buf[fileEntryOffset + FILE_ATTRIBUTE_OFFSET];
        }

        const bool is_directory (uint16_t fileEntryOffset) const {
            return SUB_DIR & this->get_file_attributes(fileEntryOffset);
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
         *              error code is FatFS::EOC_END for end-of-chain or
         *              file-not-found marker)
         */
        PropWare::ErrorCode find (const char *filename, uint16_t *fileEntryOffset) const {
            PropWare::ErrorCode err;
            char readEntryName[FILENAME_STR_LEN];

            *fileEntryOffset = 0;

            check_errors(this->reload_directory_start());

            // Loop through all entries in the current directory until we find the correct one
            // Function will exit normally with FatFS::EOC_END error code if the file is not found
            while (this->m_buf->buf[*fileEntryOffset]) {
                // Check if file is valid, retrieve the name if it is
                if (!this->file_deleted(*fileEntryOffset)) {
                    this->get_filename(&this->m_buf->buf[*fileEntryOffset], readEntryName);
                    if (!strcmp(filename, readEntryName))
                        // File names match, return 0 to indicate a successful search
                        return 0;
                }

                // Increment to the next file
                *fileEntryOffset += FILE_ENTRY_LENGTH;

                // If it was the last entry in this sector, proceed to the next
                // one
                if (this->m_driver->get_sector_size() == *fileEntryOffset) {
                    // Last entry in the sector, attempt to load a new sector
                    // Possible error value includes end-of-chain marker
                    check_errors(this->load_next_sector(this->m_buf));

                    *fileEntryOffset = 0;
                }
            }

            return FatFS::FILENAME_NOT_FOUND;
        }

        /**
         * @brief   Open a file that already has a slot in the current buffer
         */
        PropWare::ErrorCode open_existing_file (const uint16_t fileEntryOffset) {
            PropWare::ErrorCode err;

            if (this->is_directory(fileEntryOffset))
                return Filesystem::ENTRY_NOT_FILE;

            // Passed the file-not-directory test. Prepare the buffer for loading the file
            check_errors(this->m_driver->flush(this->m_buf));

            // Save the file entry's meta info
            this->m_dirEntryMeta = *(this->m_buf->meta);

            // Determine the file's first allocation unit
            if (FatFS::FAT_16 == this->m_fs->m_filesystem)
                this->firstTier3 = this->m_driver->get_short(fileEntryOffset + FILE_START_CLSTR_LOW,
                                                             this->m_buf->buf);
            else {
                this->firstTier3 = this->m_driver->get_short(fileEntryOffset + FILE_START_CLSTR_LOW,
                                                             this->m_buf->buf);
                const uint16_t highWord = this->m_driver->get_short(fileEntryOffset + FILE_START_CLSTR_HIGH,
                                                                    this->m_buf->buf);
                this->firstTier3 |= highWord << 16;

                // Clear the highest 4 bits - they are always reserved
                this->firstTier3 &= 0x0FFFFFFF;
            }

            // Compute some stuffs for the file
            this->m_curTier2      = 0;
            this->fileEntryOffset = fileEntryOffset;
            this->m_length        = this->m_driver->get_long(fileEntryOffset + FatFile::FILE_LEN_OFFSET,
                                                             this->m_buf->buf);

            // Claim this buffer as our own
            this->m_contentMeta.id = this->m_id;
            this->m_contentMeta.curTier1Offset = 0;
            this->m_contentMeta.curTier3 = this->firstTier3;
            this->m_contentMeta.curTier2StartAddr = this->m_fs->compute_tier1_from_tier3(this->firstTier3);
            check_errors(this->m_fs->get_fat_value(this->m_buf->meta->curTier3, &(this->m_buf->meta->nextTier3)));

            // Finally, read the first sector
            this->m_buf->meta = &this->m_contentMeta;
            return this->m_driver->reload_buffer(this->m_buf);
        }

        bool file_deleted (const uint16_t fileEntryOffset) const {
            return DELETED_FILE_MARK == this->m_buf->buf[fileEntryOffset];
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
        void get_filename (const uint8_t buf[], char filename[]) const {
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
        PropWare::ErrorCode load_next_sector (BlockStorage::Buffer *buf) const {
            PropWare::ErrorCode err;
            check_errors(this->m_fs->get_driver()->flush(buf));

            // Check for the end-of-chain marker (end of file)
            if (this->m_fs->is_eoc(buf->meta->curTier3))
                return FatFS::EOC_END;

            // Are we looking at the root directory of a FAT16 system?
            if (FatFS::FAT_16 == this->m_fs->m_filesystem && this->m_fs->m_rootAddr == (buf->meta->curTier2StartAddr)) {
                // Root dir of FAT16; Is it the last sector in the root directory?
                if (this->m_fs->m_rootDirSectors == buf->meta->curTier1Offset)
                    return FatFS::EOC_END;
                    // Root dir of FAT16; Not last sector
                else
                    // Any error from reading the data block will be returned to calling function
                    return this->m_fs->get_driver()->read_data_block(++(buf->meta->curTier1Offset), buf->buf);
            }
                // We are looking at a generic data cluster.
            else {
                // Generic data cluster; Have we reached the end of the cluster?
                if (((1 << this->m_fs->get_tier1s_per_tier2_shift()) - 1) > (buf->meta->curTier1Offset)) {
                    // Generic data cluster; Not the end; Load next sector in the cluster

                    // Any error from reading the data block will be returned to calling function
                    buf->meta->curTier1Offset++;
                    return this->m_fs->get_driver()->read_data_block(
                            buf->meta->curTier1Offset + buf->meta->curTier2StartAddr, buf->buf);
                }
                    // End of generic data cluster; Look through the FAT to find the next cluster
                else
                    return this->inc_cluster(buf);
            }
        }

        /**
         * @brief       Read the next sector from storage device into memory
         *
         * When the final sector of a cluster is finished, FatFile::inc_cluster can be called.
         * The appropriate global variables will be set according (incremented or set by the FAT) and the first sector
         * of the next cluster will be read into the desired buffer.
         *
         * @param[out]  *buf    Buffer that the next sector should be loaded into
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode inc_cluster (BlockStorage::Buffer *buf) const {
            PropWare::ErrorCode err;
            check_errors(this->m_fs->get_driver()->flush(buf));

            // If we're at the end already, fail
            if (this->m_fs->is_eoc(buf->meta->curTier3))
                return FatFS::READING_PAST_EOC;

            // Increment allocation unit
            buf->meta->curTier3 = buf->meta->nextTier3;
            // Only look ahead to the next allocation unit if the current alloc unit is not EOC
            if (!this->m_fs->is_eoc(buf->meta->curTier3)) {
                // Current allocation unit is not EOC, read the next one
                check_errors(this->m_fs->get_fat_value(buf->meta->curTier3, &(buf->meta->nextTier3)));
            }
            buf->meta->curTier2StartAddr = this->m_fs->compute_tier1_from_tier3(buf->meta->curTier3);
            buf->meta->curTier1Offset = 0;

            return this->m_fs->get_driver()->read_data_block(buf->meta->curTier2StartAddr, buf->buf);
        }

        const bool buffer_holds_directory_start () const {
            const bool bufferIsDirectory = &this->m_fs->m_dirMeta == this->m_buf->meta;
            const bool tier1AtStart = 0 == this->m_fs->m_dirMeta.curTier1Offset;
            const bool tier2StartAtTier3Start = this->m_fs->compute_tier1_from_tier3(this->m_fs->m_dir_firstAllocUnit)
                                                == this->m_fs->m_dirMeta.curTier2StartAddr;

            return bufferIsDirectory
                   && tier1AtStart
                   && tier2StartAtTier3Start;
        }

        const PropWare::ErrorCode reload_directory_start () const {
            if (!this->buffer_holds_directory_start()) {
                PropWare::ErrorCode err;

                this->m_driver->flush(m_buf);
                BlockStorage::MetaData *dirMeta = &this->m_fs->m_dirMeta;

                // Reset metadata to beginning of directory
                dirMeta->curTier2StartAddr = this->m_fs->compute_tier1_from_tier3(this->m_fs->m_dir_firstAllocUnit);
                dirMeta->curTier1Offset    = 0;
                dirMeta->curTier3          = this->m_fs->m_dir_firstAllocUnit;
                check_errors(this->m_fs->get_fat_value(this->m_fsBufMeta->curTier3, &this->m_fsBufMeta->nextTier3));

                this->m_buf->meta = dirMeta;
                check_errors(this->m_fs->get_driver()->reload_buffer(this->m_buf));
            }

            return NO_ERROR;
        }

        /**
         * @brief       Load a requested sector into the buffer independent of the current sector or cluster
         *
         * @param[in]   offset  How many sectors past the first one should be skipped (sector number of the file)
         *
         * @return      Returns 0 upon success, error code otherwise
         *
         */
        PropWare::ErrorCode load_sector_from_offset (const uint32_t sectorOffset) {
            PropWare::ErrorCode err;
            uint32_t clusterOffset = sectorOffset >> this->m_fs->m_tier1sPerTier2Shift;

            this->flush();

            // Find the correct cluster
            if (this->m_curTier2 < clusterOffset) {
                // Desired cluster comes after the currently loaded one - this is easy and requires continuing to look
                // forward through the FAT from the current position
                clusterOffset -= this->m_curTier2;
                while (clusterOffset--) {
                    ++(this->m_curTier2);
                    this->m_buf->meta->curTier3 = this->m_buf->meta->nextTier3;
                    check_errors(this->m_fs->get_fat_value(this->m_buf->meta->curTier3,
                                                           &(this->m_buf->meta->nextTier3)));
                }
                this->m_buf->meta->curTier2StartAddr = this->m_fs->compute_tier1_from_tier3(
                        this->m_buf->meta->curTier3);
            } else if (this->m_curTier2 > clusterOffset) {
                // Desired cluster is an earlier cluster than the currently
                // loaded one - this requires starting from the beginning and
                // working forward
                this->m_buf->meta->curTier3 = this->firstTier3;
                check_errors(
                        this->m_fs->get_fat_value(this->m_buf->meta->curTier3, &(this->m_buf->meta->nextTier3)));
                this->m_curTier2 = 0;
                while (clusterOffset--) {
                    ++(this->m_curTier2);
                    this->m_buf->meta->curTier3 = this->m_buf->meta->nextTier3;
                    check_errors(this->m_fs->get_fat_value(this->m_buf->meta->curTier3,
                                                           &(this->m_buf->meta->nextTier3)));
                }
                this->m_buf->meta->curTier2StartAddr = this->m_fs->compute_tier1_from_tier3(
                        this->m_buf->meta->curTier3);
            }

            // Followed by finding the correct sector
            this->m_buf->meta->curTier1Offset = (uint8_t) (sectorOffset % (1 << this->m_fs->m_tier1sPerTier2Shift));
            this->m_curTier1                  = sectorOffset;
            this->m_driver->read_data_block(
                    this->m_buf->meta->curTier2StartAddr + this->m_buf->meta->curTier1Offset,
                    this->m_buf->buf);

            return 0;
        }

        PropWare::ErrorCode load_sector_under_ptr () {
            PropWare::ErrorCode err;

            // Determine if the currently loaded sector is what we need
            const uint32_t sectorOffset = (uint32_t) this->m_ptr >> this->m_fs->get_driver()->get_sector_size_shift();

            bool badData = false;
            if (this->m_buf->meta != &this->m_contentMeta) {
                check_errors(this->m_driver->flush(this->m_buf));
                this->m_buf->meta = &this->m_contentMeta;
                badData = true;
            }

            if (sectorOffset != this->m_curTier1) {
                check_errors(this->load_sector_from_offset(sectorOffset));
                badData = false;
            }

            // Make sure the buffer gets reloaded
            if (badData) {
                check_errors(this->m_driver->reload_buffer(this->m_buf));
            }

            return NO_ERROR;
        }

        /**
         * @brief       Print the attributes and name of a file entry
         *
         * @param[in]   *fileEntry  Address of the first byte of the file entry
         * @param[out]  *filename   Allocated space for the filename string to be stored
         */
        void print_file_entry (const uint8_t *fileEntry, char filename[]) {
            this->print_file_attributes(fileEntry[FILE_ATTRIBUTE_OFFSET]);
            this->get_filename(fileEntry, filename);
            this->m_logger->printf("\t\t%s", filename);
            if (SUB_DIR & fileEntry[FILE_ATTRIBUTE_OFFSET])
                this->m_logger->print('/');
            this->m_logger->println();
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
                this->m_logger->print(READ_ONLY_CHAR);
            else
                this->m_logger->print(READ_ONLY_CHAR_);

            if (HIDDEN_FILE & flags)
                this->m_logger->print(HIDDEN_FILE_CHAR);
            else
                this->m_logger->print(HIDDEN_FILE_CHAR_);

            if (SYSTEM_FILE & flags)
                this->m_logger->print(SYSTEM_FILE_CHAR);
            else
                this->m_logger->print(SYSTEM_FILE_CHAR_);

            if (VOLUME_ID & flags)
                this->m_logger->print(VOLUME_ID_CHAR);
            else
                this->m_logger->print(VOLUME_ID_CHAR_);

            if (SUB_DIR & flags)
                this->m_logger->print(SUB_DIR_CHAR);
            else
                this->m_logger->print(SUB_DIR_CHAR_);

            if (ARCHIVE & flags)
                this->m_logger->print(ARCHIVE_CHAR);
            else
                this->m_logger->print(ARCHIVE_CHAR_);
        }

        void print_status (const bool printBlocks = false) const {
            this->File::print_status("FatFileReader", printBlocks);

            this->m_logger->println("FAT-specific");
            this->m_logger->println("------------");
            this->m_logger->printf("\tStarting allocation unit: 0x%08X/%u\n", this->firstTier3, this->firstTier3);
            this->m_logger->printf("\tCurrent sector (counting from first in file): 0x%08X/%u\n",
                                   this->m_curTier1, this->m_curTier1);
            this->m_logger->printf("\tCurrent cluster (counting from first in file): 0x%08X/%u\n",
                                   this->m_curTier2, this->m_curTier2);
            this->m_logger->printf("\tDirectory address (sector): 0x%08X/%u\n", this->m_dirTier1Addr,
                                   this->m_dirTier1Addr);
            this->m_logger->printf("\tFile entry offset: 0x%04X\n", this->fileEntryOffset);

        }

    protected:
        FatFS                  *m_fs;
        /** File's starting allocation unit */
        uint32_t               firstTier3;
        /** like curTier1Offset, but does not reset upon loading a new cluster */
        uint32_t               m_curTier1;
        /** like curTier1, but for clusters */
        uint32_t               m_curTier2;
        /** Which sector of the storage device contains this file's meta-data */
        uint32_t               m_dirTier1Addr;
        /** Address within the sector of this file's entry */
        uint16_t               fileEntryOffset;
    };

}
