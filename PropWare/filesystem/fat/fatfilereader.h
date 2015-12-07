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
            if ((err = this->find(this->get_name(), &fileEntryOffset)))
                // Find returned an error; ensure it was EOC...
                return FatFS::EOC_END == err ? FatFile::FILENAME_NOT_FOUND : err;

            // `name` was found successfully
            check_errors(this->open_existing_file(fileEntryOffset));
            this->m_open = true;
            return NO_ERROR;
        }

        PropWare::ErrorCode safe_get_char (char &c) {
            PropWare::ErrorCode err;

            if (this->m_open) {
                check_errors(this->load_sector_under_ptr());

                // Get the character
                const uint16_t bufferOffset = (uint16_t) (this->m_ptr % this->m_driver->get_sector_size());
                c = this->m_buf->buf[bufferOffset];

                // Finally done. Increment the pointer
                ++(this->m_ptr);

                return NO_ERROR;
            } else {
                return FILE_NOT_OPEN;
            }
        }
};

}
