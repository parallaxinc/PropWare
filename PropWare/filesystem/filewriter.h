/**
 * @file        PropWare/filesystem/filewriter.h
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
#include <PropWare/printcapable.h>

namespace PropWare {

class FileWriter : virtual public File, public PrintCapable {
    public:
        FileWriter (Filesystem &fs, const char name[], BlockStorage::Buffer *buffer = NULL,
                    const Printer &logger = pwOut)
                : File(fs, name, buffer, logger),
                  m_fileMetadataModified(false) {
        }

        /**
         * @brief       Write a character to the file
         *
         * @param[in]   c   Character that should be written
         *
         * @return      0 upon success, error code otherwise
         */
        virtual PropWare::ErrorCode safe_put_char (const char c) = 0;

        /**
         * @brief       Write a character to the file
         *
         * @param[in]   c   Character that should be written
         */
        void put_char (const char c) {
            this->safe_put_char(c);
        }

        /**
         * @brief       Write a character array to the file
         *
         * @param[in]   string  Null-terminated character array that should be written
         *
         * @return      0 upon success, error code otherwise
         */
        virtual PropWare::ErrorCode safe_puts (const char string[]) {
            PropWare::ErrorCode err;

            char *s = (char *) string;
            while (*s++) {
                check_errors(this->safe_put_char(*s));
            }

            return NO_ERROR;
        }

        /**
         * @brief       Write a character array to the file
         *
         * @param[in]   string  Null-terminated character array that should be written
         */
        void puts (const char string[]) {
            this->safe_puts(string);
        }

        void print_status (const bool printBlocks = false, const bool printParentStatus = true) const {
            if (printParentStatus)
                this->File::print_status("FileWriter", printBlocks);

            this->m_logger->println("FileWriter-specific");
            this->m_logger->println("-------------------");
            this->m_logger->printf("\tModified: %s\n", Utility::to_string(this->m_fileMetadataModified));
        }

    protected:
        /** When the length of a file is changed, this variable will be set, otherwise cleared */
        bool m_fileMetadataModified;
};

}
