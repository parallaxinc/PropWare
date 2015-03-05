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
#include <PropWare/filesystem/fatfs.h>

namespace PropWare {

class FatFile : public File {
    public:
        FatFile(FatFS &fs, const char name[], const Mode mode, const Printer *logger = &pwOut)
                : File(fs, name, mode, logger),
                  m_fs(&fs) {
        }

        ~FatFile () {
            this->close();
        }

        PropWare::ErrorCode open (BlockStorage::Buffer *buffer = NULL) {
            PropWare::ErrorCode err;
            uint16_t            fileEntryOffset = 0;

            // Attempt to find the file
            if ((err = this->m_fs->find(this->m_name, &fileEntryOffset))) {
                // Find returned an error; ensure it was EOC...
                if (FatFS::EOC_END == err) {
                    // And throw a FILE_NOT_FOUND error if using read only mode
                    if (File::READ == this->m_mode)
                        return Filesystem::FILENAME_NOT_FOUND;
                    else {
                        // File wasn't found and the cluster is full; add another
                        // to the directory
                        check_errors(this->m_fs->extend_fat(this->m_buf));
                        check_errors(this->m_fs->load_next_sector(this->m_buf));
                    }
                }

                if (FatFS::FILENAME_NOT_FOUND == err) {
                    // File wasn't found, but there is still room in this
                    // cluster (or a new cluster was just added)
                    check_errors(this->create_file(this->m_name, &fileEntryOffset));
                }
                    // find returned unknown error - throw it
                else
                    return err;
            }

            // `name` was found successfully
            if (this->is_directory(fileEntryOffset))
                return Filesystem::ENTRY_NOT_FILE;

            // Instantiate the file
            if (NULL == buffer)
                this->m_buf = this->m_fs->get_buffer();
            else
                this->m_buf = buffer;
            check_errors(this->m_fs->get_driver()->flush(this->m_buf));

            // Passed the file-not-directory test, load it into the buffer and
            // update status variables
            if (FatFS::FAT_16 == this->m_fs->m_filesystem)
                this->m_buf->curTier3 = this->m_fs->get_driver()->get_short(
                        fileEntryOffset + FatFS::FILE_START_CLSTR_LOW, this->m_buf->buf);
            else {
                this->m_buf->curTier3 = this->m_fs->get_driver()->get_short(
                        fileEntryOffset + FatFS::FILE_START_CLSTR_LOW, this->m_buf->buf);
                uint16_t highWord = this->m_fs->get_driver()->get_short(fileEntryOffset + FatFS::FILE_START_CLSTR_HIGH,
                                                                        this->m_buf->buf);
                this->m_buf->curTier3 |= highWord << 16;

                // Clear the highest 4 bits - they are always reserved
                this->m_buf->curTier3 &= 0x0FFFFFFF;
            }

            this->firstTier3               = this->m_buf->curTier3;
            this->m_curTier2               = 0;
            this->m_buf->curTier2StartAddr = this->m_fs->compute_tier1_from_tier3(this->m_buf->curTier3);
            this->m_dirTier1Addr           = this->m_buf->curTier2StartAddr + this->m_buf->curTier1Offset;
            this->fileEntryOffset          = fileEntryOffset;
            check_errors(this->m_fs->get_fat_value(this->m_buf->curTier3, &(this->m_buf->nextTier3)));
            this->m_buf->curTier1Offset = 0;
            this->m_length              = this->m_fs->get_driver()->get_long(fileEntryOffset + FatFile::FILE_LEN_OFFSET,
                                                                             this->m_buf->buf);

            // Determine the number of sectors currently allocated to this file;
            // useful in the case that the file needs to be extended
            this->m_maxTier1s     = this->m_length >> this->m_fs->get_driver()->get_sector_size_shift();
            if (!(this->m_maxTier1s))
                this->m_maxTier1s = (uint32_t) (1 << this->m_fs->get_tier1s_per_tier2_shift());
            while (this->m_maxTier1s % (1 << this->m_fs->get_tier1s_per_tier2_shift()))
                ++(this->m_maxTier1s);
            check_errors(this->m_fs->get_driver()->read_data_block(this->m_buf->curTier2StartAddr, this->m_buf->buf));

            return NO_ERROR;
        }

        char get_char () {
                    return '\0';
                }

        PropWare::ErrorCode flush () {
            PropWare::ErrorCode err;

            if (this->m_mode == File::READ) {
                return NO_ERROR;
            } else {
                // If the currently loaded sector has been modified, save the changes
                check_errors(this->m_fs->get_driver()->flush(this->m_buf));

                // If we modified the length of the file...
                if (this->m_mod) {
                    // Then check if the directory sector is loaded...
                    if ((this->m_buf->curTier2StartAddr + this->m_buf->curTier1Offset) != this->m_dirTier1Addr)
                        // And load it if it isn't
                    check_errors(this->m_fs->get_driver()->read_data_block(this->m_dirTier1Addr, this->m_buf));

                    // Finally, edit the length of the file
                    this->m_fs->get_driver()->write_long(this->fileEntryOffset + FILE_LEN_OFFSET, this->m_buf->buf,
                                                         this->m_length);
                    this->m_buf->mod = true;
                    check_errors(this->m_fs->get_driver()->flush(this->m_buf));
                }

                return NO_ERROR;
            }
        }

    private:
        static const uint8_t FILE_LEN_OFFSET = 0x1C;  // Length of a file in bytes

    private:
        /**
         * @brief       Allocate space for a new file
         *
         * @param[in]   *name               Character array for the new file
         * @param[in]   *fileEntryOffset    Offset from the currently loaded directory entry where the file's metadata
         *                                  should be written
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        const PropWare::ErrorCode create_file (const char *name, const uint16_t *fileEntryOffset) {
            // TODO: The whole method
            /*uint8_t i, j;
            // *name is only checked for uppercase
            char uppercaseName[FILENAME_STR_LEN];
            uint32_t allocUnit;

            // Parameter checking...
            if (FILENAME_STR_LEN < strlen(name))
                return INVALID_FILENAME;

            // Convert the name to uppercase
            strcpy(uppercaseName, name);
            Utility::to_upper(uppercaseName);

            // Write the file fields in order...

            *//* 1) Short file name *//*
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

            *//* 2) Write attribute field... *//*
            // TODO: Allow for file attribute flags to be set, such as
            //       READ_ONLY, SUB_DIR, etc
            // Archive flag should be set because the file is new
            this->m_buf.buf[*fileEntryOffset + FILE_ATTRIBUTE_OFFSET] = ARCHIVE;
            this->m_buf.mod = true;

            *//**
            * 3) Find a spot in the FAT (do not check for a full FAT, assume
            *    space is available)
            *//*
            allocUnit = this->find_empty_space(0);
            this->m_driver->write_short(*fileEntryOffset + FILE_START_CLSTR_LOW, this->m_buf.buf, allocUnit);
            if (FAT_32 == this->m_filesystem)
                this->m_driver->write_short(*fileEntryOffset + FILE_START_CLSTR_HIGH, this->m_buf.buf, allocUnit >> 16);

            *//* 4) Write the size of the file (currently 0) *//*
            this->m_driver->write_long(*fileEntryOffset + FatFile::FILE_LEN_OFFSET, this->m_buf.buf, 0);

            this->m_buf.mod = true;*/

            return 0;
        }

        const uint8_t get_file_attributes (uint16_t fileEntryOffset) const {
            return this->m_buf->buf[fileEntryOffset + FatFS::FILE_ATTRIBUTE_OFFSET];
        }

        const bool is_directory (uint16_t fileEntryOffset) const {
            return FatFS::SUB_DIR & this->get_file_attributes(fileEntryOffset);
        }

        void print_status () const {
            this->File::print_status("FatFile");

            this->m_logger->println("\tFAT-specific");
            this->m_logger->println("\t============");
            this->m_logger->printf("\t\tSectors allocated to file: 0x%08X/%u" CRLF, this->m_maxTier1s,
                                   this->m_maxTier1s);
            this->m_logger->printf("\t\tStarting allocation unit: 0x%08X/%u" CRLF, this->firstTier3, this->firstTier3);
            this->m_logger->printf("\t\tCurrent sector (counting from first in file): 0x%08X/%u" CRLF,
                                   this->m_curTier1, this->m_curTier1);
            this->m_logger->printf("\t\tCurrent cluster (counting from first in file): 0x%08X/%u" CRLF,
                                   this->m_curTier2, this->m_curTier2);
            this->m_logger->printf("\t\tDirectory address (sector): 0x%08X/%u" CRLF, this->m_dirTier1Addr,
                                   this->m_dirTier1Addr);
            this->m_logger->printf("\t\tFile entry offset: 0x%04X" CRLF, this->fileEntryOffset);

        }

    protected:
        FatFS    *m_fs;
        /** Maximum number of sectors currently allocated to a file */
        uint32_t m_maxTier1s;
        /** File's starting allocation unit */
        uint32_t firstTier3;
        /** like curTier1Offset, but does not reset upon loading a new cluster */
        uint32_t m_curTier1;
        /** like curTier2, but for allocation units */
        uint32_t m_curTier2;
        /** Which sector of the storage device contains this file's meta-data */
        uint32_t m_dirTier1Addr;
        /** Address within the sector of this file's entry */
        uint16_t fileEntryOffset;
};

}
