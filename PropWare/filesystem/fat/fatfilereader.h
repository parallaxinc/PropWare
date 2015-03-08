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
        FatFileReader (FatFS &fs, const char name[], BlockStorage::Buffer *buffer = NULL, const Printer &logger = pwOut)
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

            // Passed the file-not-directory test. Prepare the buffer for loading the file
            check_errors(this->m_driver->flush(this->m_buf));

            // Save the file entry's sector address
            this->m_dirTier1Addr = this->m_buf->curTier2StartAddr + this->m_buf->curTier1Offset;

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
            this->m_length        = this->m_driver->get_long(fileEntryOffset + FatFileReader::FILE_LEN_OFFSET,
                                                             this->m_buf->buf);

            // Claim this buffer as our own
            this->m_buf->id = this->m_id;
            this->m_buf->curTier1Offset = 0;
            this->m_buf->curTier3 = this->firstTier3;
            this->m_buf->curTier2StartAddr = this->m_fs->compute_tier1_from_tier3(this->firstTier3);
            check_errors(this->m_fs->get_fat_value(this->m_buf->curTier3, &(this->m_buf->nextTier3)));

            // Finally, read the first sector
            check_errors(this->m_driver->read_data_block(this->m_buf->curTier2StartAddr, this->m_buf->buf));

            return NO_ERROR;
        }

        PropWare::ErrorCode safe_get_char (char &c) {
            PropWare::ErrorCode err;

            const uint16_t bufferOffset = (uint16_t) (this->m_ptr % this->m_fs->m_sectorSize);

            // Determine if the currently loaded sector is what we need
            const uint32_t sectorOffset = (this->m_ptr >> this->m_fs->m_driver->get_sector_size_shift());

            // Determine if the correct sector is loaded
            if (this->m_buf->id != this->m_id) {
                pwOut.println("Reloading buffer!");
                check_errors(this->reload_buf());
            } else if (sectorOffset != this->m_curTier1) {
                check_errors(this->load_sector_from_offset(sectorOffset));
            }
            ++(this->m_ptr);
            c = this->m_buf->buf[bufferOffset];

            return NO_ERROR;
        }
};

}
