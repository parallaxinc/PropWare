/**
 * @file        PropWare/filesystem/fat/fatfilereader.h
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

/**
 * @brief   Read a file on a FAT 16 or FAT 32 storage device
 *
 * A file can be echoed to the terminal with a simple program such as
 *
 * @code
 * int main () {
 *     const SD driver;
 *     FatFS filesystem(&driver);
 *     filesystem.mount();
 *
 *     FatFileReader reader(filesystem, "fat_test.txt");
 *     reader.open();
 *
 *     while (!reader.eof())
 *         pwOut << reader.get_char();
 *
 *     return 0;
 * }
 * @endcode
 *
 * It can also be hooked up to the `PropWare::Scanner` class for line-by-line or formatted reading:
 *
 * @code
 * int main () {
 *     const SD driver;
 *     FatFS filesystem(&driver);
 *     filesystem.mount();
 *
 *     FatFileReader reader(filesystem, "fat_test.txt");
 *     reader.open();
 *
 *     Scanner fileScanner(&reader);
 *     char buffer[256];
 *     while (!reader.eof()) {
 *         fileScanner.gets(buffer, 256);
 *         pwOut.println(buffer);
 *     }
 *
 *     return 0;
 * }
 * @endcode
 *
 */
class FatFileReader : virtual public FatFile, virtual public FileReader {
    public:
        /**
         * @brief       Construct a new file instance
         *
         * @param[in]   fs          The filesystem is needed for opening the file
         * @param[in]   name        Name of the file to open - it must exist in the current working directory (see
         *                          [issue 55](https://github.com/parallaxinc/PropWare/issues/55) for opening from a
         *                          relative or absolute path)
         * @param[in]   *buffer     Address of a dedicated buffer that should be used for this file. If left as the
         *                          NULL (the default), a shared buffer will be used.
         * @param[in]   logger      This is only used for printing debug statements. Use of the logger is limited
         *                          such that all references will be optimized out in normal application code
         */
        FatFileReader (FatFS &fs, const char name[], BlockStorage::Buffer &buffer, const Printer &logger = pwOut)
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
