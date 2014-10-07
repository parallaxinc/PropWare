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
#include <ctype.h>
#include <PropWare/PropWare.h>
#include <PropWare/printcapable.h>
#include <PropWare/pin.h>

#ifndef S_ISNAN
#define S_ISNAN(x) (x != x)
#endif  /* !defined(S_ISNAN) */
#ifndef S_ISINF
#define S_ISINF(x) (x != 0.0 && x + x == x)
#endif  /* !defined(S_ISINF) */

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
        virtual void put_char (const char c) const {
            this->printCapable->put_char(c);
        }

        /**
         * @see PropWare::PrintCapable::puts
         */
        virtual void puts (const char string[]) const {
            this->printCapable->puts(string);
        }

        /**
         * @brief       Print a signed integer in base 10
         *
         * @param[in]   x   Integer to be printed
         */
        virtual void put_int (int32_t x, uint16_t width,
                      const char fillChar,
                      const bool bypassLock = false) const {
            if (0 > x)
                this->printCapable->put_char('-');

            this->put_uint((uint32_t) abs(x), width, fillChar, bypassLock);
        }

        /**
         * @brief       Print an unsigned integer in base 10
         *
         * @param[in]   x   Integer to be printed
         */
        virtual void put_uint (uint32_t x, uint16_t width,
                               const char fillChar,
                               const bool bypassLock = false) const {
            const uint8_t radix = 10;
            char          buf[sizeof(x) * 8];
            uint8_t       j, i  = 0;

            // Create a character array in reverse order, starting with the
            // tens digit and working toward the largest digit
            do {
                buf[i] = x % radix + '0';
                x /= radix;
                ++i;
            } while (x);

            if (width) {
                width -= i;
                while (width--)
                    this->put_char(fillChar);
            }

            // Reverse the character array
            for (j = 0; j < i; ++j)
                this->printCapable->put_char((uint16_t) buf[i - j - 1]);
        }

        /**
         * @brief       Print an integer in base 16 (hexadecimal) with capital
         *              letters
         *
         * @param[in]   x   Integer to be printed
         */
        virtual void put_hex (uint32_t x, uint16_t width,
                      const char fillChar,
                      const bool bypassLock = false) const {
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

            if (width) {
                width -= i;
                while (width--)
                    this->put_char(fillChar);
            }

            // Reverse the character array
            for (j = 0; j < i; ++j)
                this->printCapable->put_char((uint16_t) buf[i - j - 1]);
        }

#ifdef ENABLE_PROPWARE_PRINT_FLOAT
        /**
         * @brief       Print a floating point number with a given width and
         *              precision
         *
         * @param[in]   f           Number to print
         * @param[in]   width       Number of integer digits to print
         * @param[in]   precision   Number of digits to the right of the decimal
         *                          point to print
         */
        virtual void put_float (double f, uint16_t width, uint16_t precision,
                        const char fillChar,
                        const bool bypassLock = false) const {
            ////////////////////////////////////////////////////////////////////
            // Code taken straight from Parallax's floatToString! Thank you!!!
            ////////////////////////////////////////////////////////////////////

            char buffer[32];
            char *s = buffer;
            union convert {
                float v;
                int   w;
            } fval;

            int m = 0;
            int k = 0;

            int j    = 0;
            int sign = 0;
            int g    = 0;

            int   reps  = 0;
            float scale;
            int   ctr   = 0;

            int offset;
            int p;
            int q;
            int n;

            if (S_ISNAN(f)) {
                this->puts("nan");
            }
            if (S_ISINF(f)) {
                if (((int) f) & 0x80000000)
                    this->puts("-inf");
                else
                    this->puts("inf");
            }

            /* clamp the digits. */
            int clamp = 6; /* a buffer must be at least clamp + 4 digits */
            precision = (precision > clamp) ? clamp : precision;

            if (f < 0.0) {
                sign = 1;
                f    = -f;
            }

            if (sign) {
                s[j++] = '-';
            }

            /* Find reasonable starting value for scale.
            // Using 2^10x has similar values to 10^3x. */
            fval.v = f;
            g = fval.w;

            g >>= 23;
            g &= 0xFF;
            g -= 127;

            reps     = (g / 10);
            scale    = 1.0;
            for (ctr = 0; ctr <= reps; ctr++) {
                scale *= 1000.0;
            }

            /* If integer is zero, 0 */
            if (f < 1.0) {
                s[j++] = '0';
            }
            else {
                char c;
                for (; scale >= 1.0; scale /= 10.0) {
                    if (f >= scale) {
                        break;
                    }
                }
                for (; scale >= 1.0; scale /= 10.0) {
                    c = (char) (f / scale);
                    f -= ((float) c * scale);
                    c += 48;
                    s[j++] = c;
                }
            }

            /* If width > current size, move right, then pad with spaces */
            offset = width - j - precision - 1;
            if (precision == 0)
                offset++;

            p = j + offset;
            q = j;
            n = p;

            if (offset > 0) {
                for (; j >= 0;) {
                    s[n--] = s[j--];
                }
                for (; n >= 0;) {
                    s[n--] = fillChar;
                }
                j = p;
            }
            else {
                j = q;
            }

            /* Append with fractional */
            if (precision > 0)
                s[j++] = '.';

            k = j;
            k += precision;
            for (; j <= k;) {
                f *= 10.0;
                s[j++] = (char) f + '0';
                f -= ((int) f);
            }

            m = j - 1;
            j--;
            if (s[j] >= '5') {
                j--;
                for (; (j >= 0); j--) {
                    if ((s[j] < '0') || (s[j] > '9')) continue;
                    if (s[j] < '9') {
                        s[j]++;
                        break;
                    }
                    else {
                        s[j] = '0';
                    }
                }
            }

            s[m] = 0;

            this->puts(s);
        }
#endif

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
         *                - \%f - Floating point number (must have CMake option
         *                  `PROPWARE_PRINT_FLOAT` enabled)
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
         *                        `Printer::printf("Hello, %s!", "David");`<br>
         *                      and "Hello, David!" would be sent out the serial
         *                      port. Multiple arguments can be used as well,
         *                      such as:<br>
         *                        `Printer::printf("%i + %i = %i", 2, 3, 2 +
         *                        3);`<br>
         *                      Which would print:<br>
         *                        `2 + 3 = 5`
         */
        void printf (const char fmt[], ...) const {
            va_list    list;
            va_start(list, fmt);
            this->_printf(fmt, false, list);
            va_end(list);
        }

        virtual void _printf(const char fmt[], const bool bypassLock,
                             const va_list list) const {
            const char *s = fmt;
            char       c, fillChar;
            uint16_t    width;

            while (*s) {
                c = *s;

                if ('%' == c) {
                    c = *(++s);

                    (c == '0') ? fillChar = '0' : fillChar = ' ';

                    width = 0;
                    while (c && isdigit(c)) {
                        width = 10 * width + (c - '0');
                        c     = *(++s);
                    }

                    uint8_t precision = 6;
                    if (c == '.') {
                        precision = 0;
                        c         = *(++s);
                        while (c && isdigit(c)) {
                            precision = 10 * precision + (c - '0');
                            c         = *(++s);
                        }
                    }

                    switch (c) {
                        case 'i':
                        case 'd':
                            this->put_int(va_arg(list, int32_t), width,
                                          fillChar, bypassLock);
                            break;
                        case 'u':
                            this->put_uint(va_arg(list, uint32_t), width,
                                           fillChar, bypassLock);
                            break;
                        case 's':
                            this->printCapable->puts(va_arg(list, char *));
                            break;
                        case 'c':
                            this->printCapable->put_char(va_arg(list, int));
                            break;
                        case 'X':
                            this->put_hex(va_arg(list, uint32_t), width,
                                          fillChar, bypassLock);
                            break;
#ifdef ENABLE_PROPWARE_PRINT_FLOAT
                        case 'f':
                            this->put_float(va_arg(list, double), width,
                                            precision, fillChar, bypassLock);
                            break;
#endif
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
        }

    protected:
        const PrintCapable *printCapable;
};

class SynchronousPrinter : public virtual Printer {
    public:
        SynchronousPrinter (PrintCapable const *printCapable)
                : Printer(printCapable) {
            this->m_lock = locknew();
        }

        ~SynchronousPrinter () {
            lockret(this->m_lock);
        }

        virtual void put_char (const char c) const {
            while (lockset(this->m_lock));
            Printer::put_char(c);
            lockclr(this->m_lock);
        }

        virtual void puts (const char string[]) const {
            while (lockset(this->m_lock));
            Printer::puts(string);
            lockclr(this->m_lock);
        }

        virtual void put_int (int32_t x, uint16_t width, const char fillChar,
                              const bool bypassLock = false) const {
            if (bypassLock)
                Printer::put_int(x, width, fillChar, true);
            else {
                while (lockset(this->m_lock));
                Printer::put_int(x, width, fillChar, true);
                lockclr(this->m_lock);
            }
        }

        virtual void put_uint (uint32_t x, uint16_t width, const char fillChar,
                               const bool bypassLock = false) const {
            if (bypassLock)
                Printer::put_uint(x, width, fillChar, true);
            else {
                while (lockset(this->m_lock));
                Printer::put_uint(x, width, fillChar, true);
                lockclr(this->m_lock);
            }
        }

        virtual void put_hex (uint32_t x, uint16_t width, const char fillChar,
                              const bool bypassLock = false) const {
            if (bypassLock)
                Printer::put_hex(x, width, fillChar, true);
            else {
                while (lockset(this->m_lock));
                Printer::put_hex(x, width, fillChar, true);
                lockclr(this->m_lock);
            }
        }

#ifdef ENABLE_PROPWARE_PRINT_FLOAT

        virtual void put_float (double f, uint16_t width, uint16_t precision,
                                const char fillChar,
                                const bool bypassLock = false) const {
            if (bypassLock)
                Printer::put_float(f, width, precision, fillChar, true);
            else {
                while (lockset(this->m_lock));
                Printer::put_float(f, width, precision, fillChar, true);
                lockclr(this->m_lock);
            }
        }
#endif

        void _printf (const char fmt[], const bool bypassLock,
                      const va_list list) const {
            while (lockset(this->m_lock));
            Printer::_printf(fmt, true, list);
            waitcnt(400 + CNT);
            lockclr(this->m_lock);
        }

    protected:
        volatile int m_lock;
};

}

extern const PropWare::Printer pwOut;
