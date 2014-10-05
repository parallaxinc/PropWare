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

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <PropWare/PropWare.h>
#include <PropWare/printcapable.h>

namespace PropWare {

/**
* @brief    Interface for all classes capable of printing
*/
class Printer {
        public:
        Printer (const PrintCapable *printCapable)
                : printCapable(printCapable) {
        }

        /**
         * @see PropWare::PrintCapable::put_char
         */
        void put_char (const char c) const {
            this->printCapable->put_char(c);
        }

        /**
         * @see PropWare::PrintCapable::puts
         */
        void puts (const char string[]) const {
            this->printCapable->puts(string);
        }

        /**
         * @brief       Print a signed integer in base 10
         *
         * @param[in]   x   Integer to be printed
         */
        void put_int (int32_t x) const {
            if (0 > x)
                this->printCapable->put_char('-');

            this->put_uint((uint32_t) abs(x));
        }

        /**
         * @brief       Print an unsigned integer in base 10
         *
         * @param[in]   x   Integer to be printed
         */
        void put_uint (uint32_t x) const {
            const uint8_t radix = 10;
            char          buf[sizeof(x)*8];
            uint8_t       j, i    = 0;

            if (0 == x)
                this->printCapable->put_char('0');
            else {
                // Create a character array in reverse order, starting with the
                // tens digit and working toward the largest digit
                while (x) {
                    buf[i] = x % radix + '0';
                    x /= radix;
                    ++i;
                }

                // Reverse the character array
                for (j = 0; j < i; ++j)
                    this->printCapable->put_char((uint16_t) buf[i - j - 1]);
            }
        }

        /**
         * @brief       Print an integer in base 16 (hexadecimal) with capital
         *              letters
         *
         * @param[in]   x   Integer to be printed
         */
        void put_hex (uint32_t x) const {
            char    buf[sizeof(x)*2];
            uint8_t temp, j, i = 0;

            while (x) {
                temp = x & NIBBLE_0;
                if (temp < 10)
                    buf[i] = temp + '0';
                else {
                    temp -= 10;
                    buf[i] = temp + 'A';
                }
                ++i;
                x >>= 4;
            }

            // Reverse the character array
            for (j = 0; j < i; ++j)
                this->printCapable->put_char((uint16_t) buf[i - j - 1]);
        }

        /**
         * @brief       Similar in functionality to the C-standard, this method
         *              supports formatted printing using the following formats:
         *
         *                - \%i - Signed integer (32-bit max)
         *                - \%d - Signed integer (32-bit max)
         *                - \%u - Unsigned integer (32-bit max)
         *                - \%s - String
         *                - \%c - Single character
         *                - \%X - Hexadecimal with capital letters
         *                - \%\% - Literal percent sign ('\%')
         *
         *              A single space will be printed in place of unsupported
         *              formats
         *
         * @param[in]   fmt     Format string such as `Hello, %%s!` which can be
         *                      used to print anyone's name in place of `%%s`
         * @param[in]   ...     Variable number of arguments passed here.
         *                      Continuing with the `Hello, %%s!` example, a
         *                      single argument could be passed such as:<br>
         *                        `UART::printf("Hello, %s!", "David");`<br>
         *                      and "Hello, David!" would be sent out the serial
         *                      port. Multiple arguments can be used as well,
         *                      such as:<br>
         *                        `UART::printf("%i + %i = %i", 2, 3, 2 +
         *                        3);`<br>
         *                      Which would print:<br>
         *                        `2 + 3 = 5`
         */
        void printf (const char fmt[], ...) const {
            const char *s = fmt;
            va_list    list;
            va_start(list, fmt);
            while (*s) {
                if ('%' == *s) {
                    ++s;
                    switch (*s) {
                        case 'i':
                        case 'd':
                            this->put_int(va_arg(list, int32_t));
                            break;
                        case 'u':
                            this->put_uint(va_arg(list, uint32_t));
                            break;
                        case 's':
                            this->printCapable->puts(va_arg(list, char *));
                            break;
                        case 'c':
                            this->printCapable->put_char(va_arg(list, int));
                            break;
                        case 'X':
                            this->put_hex(va_arg(list, uint32_t));
                            break;
                        case '%':
                            this->printCapable->put_char('%');
                            break;
                        default:
                            va_arg(list, int);  // Increment va_arg pointer
                            this->printCapable->put_char(' ');
                            break;
                    }
                } else
                    this->printCapable->put_char(*s);
                ++s;
            }
            va_end(list);
        }

    protected:
        const PrintCapable *printCapable;
};

}
