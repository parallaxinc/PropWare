/**
 * @file        PropWare/filesystem/fat/fatfilewriter.h
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

#include <PropWare/filesystem/filewriter.h>
#include <PropWare/filesystem/fat/fatfile.h>

namespace PropWare {

extern BlockStorage::Buffer SHARED_BUFFER;

/**
 * @brief   Concrete class for writing or modifying a FAT 16/32 file
 */
class FatFileWriter : public virtual FatFile, public virtual FileWriter {

    public:
        /**
         * @brief   Standard constructor
         *
         * @param[in]   fs          A mounted FAT 16/32 filesystem
         * @param[in]   name[]      Character array with the file name
         * @param[in]   *buffer     If you don't want to use the globally shared buffer, a different buffer address can
         *                          be provided here
         * @param[in]   logger      This is only used for printing debug statements. Use of the logger is limited
         *                          such that all references will be optimized out in normal application code
         */
        FatFileWriter (FatFS &fs, const char name[], BlockStorage::Buffer &buffer = SHARED_BUFFER,
                       const Printer &logger = pwOut)
                : File(fs, name, buffer, logger),
                  FatFile(fs, name, buffer, logger),
                  FileWriter(fs, name, buffer, logger) {
        }

        /**
         * @brief   All content will be saved to the physical device and the file will be safely closed
         */
        virtual ~FatFileWriter () {
            this->close();
        }

        PropWare::ErrorCode open () {
            PropWare::ErrorCode err;
            uint16_t            fileEntryOffset = 0;

            if ((err = this->find(this->get_name(), &fileEntryOffset))) {
                switch (err) {
                    case FatFS::EOC_END:
                        check_errors(this->m_fs->extend_current_directory());
                    case FatFile::FILENAME_NOT_FOUND:
                        check_errors(this->create_new_file(fileEntryOffset));
                        break;
                    default:
                        return err;
                }
            }

            check_errors(this->open_existing_file(fileEntryOffset));
            this->m_open = true;
            return NO_ERROR;
        }

        /**
         * @brief   Mark a file as delete and free its clusters in the FAT. File content will not be cleared unless
         *          overwritten by another file. File does not have to be opened prior to deleting
         */
        PropWare::ErrorCode remove () {
            PropWare::ErrorCode err;

            // If the file hasn't been opened yet, open it
            if (0 == this->m_dirTier1Addr) {
                uint16_t fileEntryOffset = 0;
                if ((err = this->find(this->m_name, &fileEntryOffset))) {
                    if (FatFS::EOC_END == err)
                        return FatFile::FILENAME_NOT_FOUND;
                    else
                        return err;
                } else {
                    check_errors(this->open_existing_file(fileEntryOffset));
                }
            }

            check_errors(this->load_directory_sector());

            this->m_buf->buf[this->fileEntryOffset] = DELETED_FILE_MARK;
            this->m_buf->meta->mod = true;

            check_errors(this->m_fs->clear_chain(this->firstTier2));

            this->m_fileMetadataModified = false; // This guy is for file length, not the directory entry or FAT

            return NO_ERROR;
        }

        PropWare::ErrorCode flush () {
            PropWare::ErrorCode err;

            // Flush the file contents
            if (this->m_buf->meta == &this->m_contentMeta) {
                check_errors(this->m_driver->flush(this->m_buf));
            }

            // If we modified any metadata for the file...
            if (this->m_fileMetadataModified) {
                this->load_directory_sector();

                // Finally, edit the length of the file
                this->m_buf->meta->mod = true;
                this->m_driver->write_long(this->fileEntryOffset + FILE_LEN_OFFSET, this->m_buf->buf,
                                           (const uint32_t) this->m_length);

                check_errors(this->m_driver->flush(this->m_buf));
                this->m_fileMetadataModified = false;
            }

            return NO_ERROR;
        }

        PropWare::ErrorCode safe_put_char (const char c) {
            PropWare::ErrorCode err;

            if (this->m_open) {
                if (this->need_to_extend_fat()) {
                    check_errors(this->m_fs->extend_fat(&this->m_contentMeta));
                }

                check_errors(this->load_sector_under_ptr());

                // Get the character
                const uint16_t bufferOffset = (uint16_t) (this->m_ptr % this->m_driver->get_sector_size());
                this->m_buf->buf[bufferOffset] = (uint8_t) c;
                this->m_buf->meta->mod = true;

                // If we extended the file, be sure to increment the length counter and make a note that the length changed
                if (this->m_length == this->m_ptr) {
                    ++this->m_length;
                    this->m_fileMetadataModified = true;
                }

                // Finally done. Increment the pointer
                ++this->m_ptr;

                return NO_ERROR;
            } else {
                return FILE_NOT_OPEN;
            }
        }

        void print_status (const bool printBlocks = false) const {
            this->File::print_status("FatFileWriter", printBlocks);
            this->FatFile::print_status(printBlocks, false);
            this->FileWriter::print_status(printBlocks, false);
        }

    protected:
        static inline bool not_period_or_end (const char c) {
            return '.' != c && c;
        }

        bool need_to_extend_fat () {
            const uint8_t  sectorsPerCluster = this->m_fs->m_tier1sPerTier2Shift;

            const uint32_t requiredSector    = (uint32_t) this->m_ptr >> this->m_driver->get_sector_size_shift();
            unsigned int   requiredCluster   = requiredSector >> sectorsPerCluster;

            if (this->m_curTier2 < requiredCluster)
                return this->m_fs->is_eoc(this->m_contentMeta.nextTier2);
            else
                return false;
        }

    protected:

        PropWare::ErrorCode create_new_file (const uint16_t fileEntryOffset) {
            PropWare::ErrorCode err;

            // Write the file fields in order...

            /* 1) Short file name */
            check_errors(this->write_filename(fileEntryOffset));

            /* 2) Write attribute field... */
            // TODO: Allow for file attribute flags to be set, such as READ_ONLY, SUB_DIR, etc
            // Archive flag should be set because the file is new
            this->m_buf->buf[fileEntryOffset + FILE_ATTRIBUTE_OFFSET] = ARCHIVE;

            /* 3) Find a spot in the FAT (do not check for a full FAT, assume space is available) */
            this->get_fat_location(fileEntryOffset);

            /* 4) Write the size of the file (currently 0) */
            this->m_driver->write_long(fileEntryOffset + FILE_LEN_OFFSET, this->m_buf->buf, 0);

            this->m_buf->meta->mod = true;
            return NO_ERROR;
        }

        inline PropWare::ErrorCode write_filename (const uint16_t fileEntryOffset) {
            PropWare::ErrorCode err;
            uint8_t             i;

            // Insert the base
            for (i = 0; not_period_or_end(this->m_name[i]); ++i)
                this->m_buf->buf[fileEntryOffset + i] = (uint8_t) this->m_name[i];

            // Check if there is an extension
            if (this->m_name[i]) {
                check_errors(this->write_filename_extension(fileEntryOffset, i));
            } else
                this->pad_with_spaces(fileEntryOffset, i);

            return NO_ERROR;
        }

        inline PropWare::ErrorCode write_filename_extension (const uint16_t fileEntryOffset, uint8_t &i) {
            uint8_t j;

            // There might be an extension - pad first name with spaces
            for (j = i; j < FILE_NAME_LEN; ++j)
                this->m_buf->buf[fileEntryOffset + j] = ' ';

            // Check if there is a period, as one would expect for a file name with an extension
            if ('.' == this->m_name[i]) {
                // Skip the period
                ++i;

                // Insert extension
                while (this->m_name[i]) {
                    this->m_buf->buf[fileEntryOffset + j] = (uint8_t) this->m_name[i];
                    ++i;
                    ++j;
                }

                // Pad extension with spaces
                while (FILE_NAME_LEN + FILE_EXTENSION_LEN > j) {
                    this->m_buf->buf[fileEntryOffset + j] = ' ';
                    ++j;
                }

                return NO_ERROR;
            } else
                // If it wasn't a period or null terminator, throw an error
                return INVALID_FILENAME;
        }

        inline void pad_with_spaces (uint16_t const fileEntryOffset, uint8_t i) {
            for (; i < (FILE_NAME_LEN + FILE_EXTENSION_LEN); ++i)
                this->m_buf->buf[fileEntryOffset + i] = ' ';
        }

        inline void get_fat_location (const uint16_t fileEntryOffset) {
            const uint32_t allocUnit = this->m_fs->find_empty_space(0);
            this->m_driver->write_short(fileEntryOffset + FILE_START_CLSTR_LOW, this->m_buf->buf, (uint16_t) allocUnit);
            if (FatFS::FAT_32 == this->m_fs->get_fs_type())
                this->m_driver->write_short(fileEntryOffset + FILE_START_CLSTR_HIGH, this->m_buf->buf,
                                            (uint16_t) (allocUnit >> 16));
        }
};

}
