/**
 * @file        scancapable.h
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

namespace PropWare {

/**
 * @brief    Interface for all classes capable of printing
 */
class ScanCapable {
    public:
        static const char STRING_DELIMITER = '\n';

    public:
        /**
         * @brief       Read and return a single character (blocking)
         */
        virtual char get_char () const = 0;

        /**
         * @brief        Read a newline-terminated (`\n`) character array. Though
         *               this method could be created using get_char, some
         *               objects (such as PropWare::UART), have optimized
         *               methods for reading a string and ScanCapable::fgets can
         *               utilize them. (blocking)
         *
         * @pre          `string[]` must have enough space allocated with a null
         * terminator
         *
         * @param[out]   string[]    Buffer to store the string in
         * @param[out]   *length     Maximum number of characters to read. If
         *                           null-pointer, characters will be read until
         *                           newline ('\n').
         */
        virtual ErrorCode fgets (char string[], int32_t *length) const = 0;
};

}
