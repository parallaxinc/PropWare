/**
 * @file        fatfilereader.h
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

#include <PropWare/filesystem/filereader.h>
#include <PropWare/filesystem/fat/fatfile.h>

namespace PropWare {

class FatFileReader : virtual public FatFile, virtual public FileReader {
    public:
        FatFileReader (FatFS &fs, const char name[], BlockStorage::Buffer *buffer = NULL,
                       const Printer &logger = pwOut)
                : File(fs, name, buffer, logger),
                  FatFile(fs, name, buffer, logger),
                  FileReader(fs, name, buffer, logger) {
        }

        PropWare::ErrorCode open () {
            PropWare::ErrorCode err;
            uint16_t            fileEntryOffset = 0;

            // Attempt to find the file
            if ((err = this->find(this->m_name, &fileEntryOffset)))
                // Find returned an error; ensure it was EOC...
                return FatFS::EOC_END == err ? Filesystem::FILENAME_NOT_FOUND : err;

            // `name` was found successfully
            if (this->is_directory(fileEntryOffset))
                return Filesystem::ENTRY_NOT_FILE;

            check_errors(this->m_driver->flush(this->m_buf));

            // Passed the file-not-directory test, load it into the buffer and
            // update status variables
            if (FatFS::FAT_16 == this->m_fs->m_filesystem)
                this->m_buf->curTier3 = this->m_driver->get_short(
                        fileEntryOffset + FILE_START_CLSTR_LOW, this->m_buf->buf);
            else {
                this->m_buf->curTier3 = this->m_driver->get_short(
                        fileEntryOffset + FILE_START_CLSTR_LOW, this->m_buf->buf);
                uint16_t highWord = this->m_driver->get_short(fileEntryOffset + FILE_START_CLSTR_HIGH,
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
            this->m_length              = this->m_driver->get_long(fileEntryOffset + FatFileReader::FILE_LEN_OFFSET,
                                                                             this->m_buf->buf);

            // Determine the number of sectors currently allocated to this file;
            // useful in the case that the file needs to be extended
            this->m_maxTier1s     = this->m_length >> this->m_driver->get_sector_size_shift();
            if (!(this->m_maxTier1s))
                this->m_maxTier1s = (uint32_t) (1 << this->m_fs->get_tier1s_per_tier2_shift());
            while (this->m_maxTier1s % (1 << this->m_fs->get_tier1s_per_tier2_shift()))
                ++(this->m_maxTier1s);
            check_errors(this->m_driver->read_data_block(this->m_buf->curTier2StartAddr, this->m_buf->buf));

            return NO_ERROR;
        }

        char get_char () {
            return '\0';
        }
};

}
