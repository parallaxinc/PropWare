/**
 * @file        PropWare/filesystem/filereader.h
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
#include <PropWare/hmi/input/scancapable.h>

namespace PropWare {

/**
 * @brief   A read-only file interface
 */
class FileReader : virtual public File, virtual public ScanCapable {
    public:
        virtual ~FileReader () { }

        PropWare::ErrorCode flush () {
            return NO_ERROR;
        }

        /**
         * @brief       Read a character from the file
         *
         * @param[out]  c   Character from file will be stored into `c`
         *
         * @return      0 upon success, error code otherwise
         */
        virtual PropWare::ErrorCode safe_get_char (char &c) = 0;

        /**
         * @brief   Read a character from the file
         *
         * @post    If an error occurs, you can retrieve the error code via `FileReader::get_error()`
         *
         * @return  Character upon success, -1 otherwise
         */
        char get_char () {
            char                      c;
            const PropWare::ErrorCode err = this->safe_get_char(c);
            if (err) {
                this->m_error = err;
                return -1;
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

        /**
         * @brief       Read a character from the file without incrementing the pointer
         *
         * @param[out]  c   Character from file will be stored into `c`
         *
         * @return      0 upon success, error code otherwise
         */
        PropWare::ErrorCode safe_peek (char &c) {
            const PropWare::ErrorCode err = this->safe_get_char(c);
            this->m_ptr--;
            return err;
        }

        /**
         * @brief   Read a character from the file without incrementing the pointer
         *
         * @post    If an error occurs, you can retrieve the error code via `FileReader::get_error()`
         *
         * @return  Character upon success, -1 otherwise
         */
        char peek () {
            const char c = get_char();
            this->m_ptr--;
            return c;
        }

        /**
         * @brief   Get the latest error that occurred during an unsafe method call
         *
         * Methods such as `FileReader::get_char()` would normally throw an exception if an error occurred in a C++
         * program, but due to size constraints, exceptions are not used by PropWare. Therefore, any error that is
         * occurs in a function that does not return `ErrorCode` simply saves the error to the internal state and
         * returns a known value.
         *
         * @return  Most recent error code
         */
        PropWare::ErrorCode get_error () const {
            return this->m_error;
        }

    protected:

        FileReader (Filesystem &fs, const char name[], BlockStorage::Buffer &buffer, const Printer &logger = pwOut)
                : File(fs, name, buffer, logger) {
        }
};

}
