/**
 * @file        filereader.h
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
#include <PropWare/scancapable.h>

namespace PropWare {

class FileReader : virtual public File, virtual public ScanCapable {
    public:
        PropWare::ErrorCode flush () {
            return NO_ERROR;
        }

        virtual PropWare::ErrorCode safe_get_char (char &c) = 0;

        char get_char () {
            char c;
            const PropWare::ErrorCode err = this->safe_get_char(c);
            if (err) {
                this->m_error = err;
                return '\0';
            } else
                return c;
        }

        /**
         * @brief       Determine whether the read pointer has reached the end of the file
         *
         * @return      Returns true if the pointer points to the end of the file, false otherwise
         */
        inline bool eof () const {
            return this->m_length == this->m_ptr;
        }

        inline int32_t tell () const {
            return this->m_ptr;
        }

        inline PropWare::ErrorCode seek (const int32_t pos, const SeekDir way) {
            return this->File::seek(this->m_ptr, pos, way);
        }

        PropWare::ErrorCode get_error () const {
            return this->m_error;
        }

    protected:
        FileReader (Filesystem &fs, const char name[], BlockStorage::Buffer *buffer = NULL,
                    const Printer &logger = pwOut)
                : File(fs, name, buffer, logger),
                  m_ptr(0) {
        }

    protected:
        int32_t m_ptr;
};

}
