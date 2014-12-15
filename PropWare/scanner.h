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

#include <PropWare/PropWare.h>
#include <PropWare/scancapable.h>
#include <PropWare/printer/printer.h>
#include <PropWare/uart/halfduplexuart.h>

namespace PropWare {

/**
* @brief    Interface for all classes capable of scanning
*/
class Scanner {
    public:
        Scanner (const ScanCapable *scanCapable,
                 const Printer *printer) : m_scanCapable(scanCapable),
                                           m_printer(printer) {
        }

        /**
         * @see PropWare::ScanCapable::get_char
         */
        char get_char () const {
            char c = this->m_scanCapable->get_char();
            this->m_printer->put_char(c);
            return c;
        }

        /**
         * @see PropWare::ScanCapable::fgets
         */
        ErrorCode gets (char string[], int32_t length) const {
            char *buf = string;
            while (--length > 0) {
                char ch = this->m_scanCapable->get_char();

                if (ch == 8 || ch == 127) {
                    if (buf > string) {
                        this->m_printer->puts("\010 \010");
                        ++length;
                        --buf;
                    }
                    length += 1;
                    continue;
                }

                this->m_printer->put_char(ch);
                if (ch == '\r')
                    this->m_printer->put_char('\n');

                if (ch == '\r' || ch == '\n')
                    break;

                *(buf++) = ch;
            }
            *buf = 0;

            return 0;
        }

    private:
        const ScanCapable *m_scanCapable;
        const Printer     *m_printer;
};

}

extern const PropWare::Scanner pwIn;
