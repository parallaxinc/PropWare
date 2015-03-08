/**
 * @file        printer.h
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
class PrintCapable {
    public:
        /**
         * @brief       Print a single character
         *
         * @param[in]   c   Individual char to be printed
         */
        virtual void put_char (const char c) = 0;

        /**
         * @brief       Send a null-terminated character array. Though this method
         *              could be created using put_char, some objects (such as
         *              PropWare::UART), have optimized methods for sending a string
         *              and PrintCapable::puts can utilize them.
         *
         * @pre         `string[]` must be terminated with a null terminator
         *
         * @param[in]   string[]    Array of data words with the final word
         *                          being 0 - the null terminator
         */
        virtual void puts (const char string[]) = 0;
};

}
