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
#include <PropWare/printcapable.h>
#include <PropWare/utility.h>

namespace PropWare {

#ifndef S_ISNAN
#define S_ISNAN(x) (x != x)
#endif  /* !defined(S_ISNAN) */

#ifndef S_ISINF
#define S_ISINF(x) (x != 0.0 && x + x == x)
#endif  /* !defined(S_ISINF) */

#define isdigit(x) ('0' <= x && x <= '9')

/**
 * @brief   Container class that has formatting methods for human-readable output. This class can be constructed and
 *          used for easy and efficient output via any communication protocol.
 *
 * <b>Printing to Terminal</b>
 * <p>
 * To print to the standard terminal, simply use the existing object, `pwOut`:
 *
 * @code
 * pwOut.printf("Hello, world!\n");
 * @endcode
 *
 * <b>Creating Custom `Printers`</b>
 * <p>
 * To create your own `Printer`, you will first need an instance of
 * any object that implements the `PrintCapable` interface. Your code
 * might look something like this:
 *
 * @code
 * PropWare::HD44780       myLCD;
 * const PropWare::Printer lcdPrinter(&myLCD);
 *
 * lcd.start(FIRST_DATA_PIN, RS, RW, EN, BITMODE, DIMENSIONS);
 * lcdPrinter.printf("Hello, LCD!\n");
 * @endcode
 *
 * Adding `const` in front of the `Printer` declaration allows the
 * compiler to make some extra optimizations and is encouraged when
 * possible.
 */
class Printer {
    public:
        static const char DEFAULT_FILL_CHAR = ' ';

        struct Format {
            uint16_t width;
            uint16_t precision;
            uint8_t  radix;
            char     fillChar;

            Format () : width(0),
                        precision(6),
                        radix(10),
                        fillChar(DEFAULT_FILL_CHAR) {
            }
        };

        static const Format DEFAULT_FORMAT;

    public:
        /**
         * @brief   Construct a Printer instance that will use the given
         *          `*printCapable` instance for sending each character
         *
         * @param   *printCapable   The address of any initialized communication
         *                          object such as a PropWare::UART
         */
        Printer (PrintCapable *printCapable, const bool cooked = true)
                : m_printCapable(printCapable),
                  m_cooked(cooked) {
        }

        /**
         * @see PropWare::PrintCapable::put_char
         */
        void put_char (const char c) const {
            if (this->m_cooked && '\n' == c)
                this->m_printCapable->put_char('\r');
            this->m_printCapable->put_char(c);
        }

        /**
         * @see PropWare::PrintCapable::puts
         */
        void puts (const char string[]) const {
            if (this->m_cooked)
                for (const char *s = string; *s; ++s)
                    this->put_char(*s);
            else
                this->m_printCapable->puts(string);
        }

        /**
         * @brief       Print a signed integer in base 10
         *
         * @param[in]   x           Integer to be printed
         * @param[in]   width       Minimum number of characters to print
         * @param[in]   fillChar    Character to print to the left of the number
         *                          if the number's width is less than `width`
         * @param[in]   bypassLock  For internal use only. Leave as default
         *                          value.
         */
        void put_int (int32_t x, uint16_t width = 0, const char fillChar = DEFAULT_FILL_CHAR) const {
            if (0 > x)
                this->put_char('-');

            this->put_uint((uint32_t) abs(x), width, fillChar);
        }

        /**
         * @brief       Print an unsigned integer in base 10
         *
         * @param[in]   x           Integer to be printed
         * @param[in]   width       Minimum number of characters to print
         * @param[in]   fillChar    Character to print to the left of the number
         *                          if the number's width is less than `width`
         */
        void put_uint (uint32_t x, uint16_t width = 0, const char fillChar = DEFAULT_FILL_CHAR) const {
            const uint8_t radix = 10;
            char          buf[sizeof(x) * 8];
            uint8_t       i = 0;

            // Create a character array in reverse order, starting with the
            // tens digit and working toward the largest digit
            do {
                buf[i] = x % radix + '0';
                x /= radix;
                ++i;
            } while (x);

            if (width && width > i) {
                width -= i;
                while (width--)
                    this->put_char(fillChar);
            }

            // Reverse the character array
            for (uint8_t j = 0; j < i; ++j)
                this->put_char(buf[i - j - 1]);
        }

        /**
         * @brief       Print an integer in base 16 (hexadecimal) with capital
         *              letters
         *
         * @param[in]   x           Integer to be printed
         * @param[in]   width       Minimum number of characters to print
         * @param[in]   fillChar    Character to print to the left of the number
         *                          if the number's width is less than `width`
         * @param[in]   bypassLock  For internal use only. Leave as default
         *                          value.
         */
        void put_hex (uint32_t x, uint16_t width = 0, const char fillChar = DEFAULT_FILL_CHAR) const {
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

            if (width && width > i) {
                width -= i;
                while (width--)
                    this->put_char(fillChar);
            }

            // Reverse the character array
            for (j = 0; j < i; ++j)
                this->put_char(buf[i - j - 1]);
        }

        /**
         * @brief       Print a floating point number with a given width and
         *              precision
         *
         * @param[in]   f           Number to print
         * @param[in]   width       Number of integer digits to print (includes
         *                          decimal point)
         * @param[in]   precision   Number of digits to print to the right of
         *                          the decimal point
         * @param[in]   fillChar    Character to print to the left of the number
         *                          if the number's width is less than `width`
         */
        void put_float (double f, uint16_t width = 0, uint16_t precision = 6,
                        const char fillChar = DEFAULT_FILL_CHAR) const {
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
            else
                j = q;

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
                for (; (j >= 0); j--)
                    if (!(s[j] < '0' || s[j] > '9')) {
                        if (s[j] < '9') {
                            s[j]++;
                            break;
                        } else
                            s[j] = '0';
                    }
            }

            s[m] = 0;

            this->puts(s);
        }

        /**
         * @see PropWare::Printer::printf(const char *fmt, const T first,
         * Targs... remaining)
         */
        void printf(const char *fmt) const {
            this->puts(fmt);
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
         *                - \%f - Floating point number (must have CMake option
         *                  `PROPWARE_PRINT_FLOAT` enabled)
         *                - \%\% - Literal percent sign ('\%')
         *
         *              A single space will be printed in place of unsupported
         *              formats
         *
         * @param[in]   *fmt    Format string such as `Hello, %%s!` which can be
         *                      used to print anyone's name in place of `%%s`
         * @param[in]   ...     Variable number of arguments passed here.
         *                      Continuing with the `Hello, %%s!` example, a
         *                      single argument could be passed such as:<br>
         *                        `pwOut.printf("Hello, %s!", "David");`<br>
         *                      and "Hello, David!" would be sent out the serial
         *                      port. Multiple arguments can be used as well,
         *                      such as:<br>
         *                        `pwOut.printf("%i + %i = %i", 2, 3, 2 +
         *                        3);`<br>
         *                      Which would print:<br>
         *                        `2 + 3 = 5`
         */
        template<typename T, typename... Targs>
        void printf (const char fmt[], const T first, const Targs... remaining) const {
            const char *s = fmt;
            char       c;
            Format     format;

            while (*s) {
                c = *s;

                if ('%' == c) {
                    c = *(++s);
                    if ('%' == c)
                        this->put_char(c);
                    else {
                        if (c == '0')
                            format.fillChar = '0';
                        else
                            format.fillChar = DEFAULT_FILL_CHAR;

                        format.width = 0;
                        while (c && isdigit(c)) {
                            format.width = 10 * format.width + (c - '0');
                            c = *(++s);
                        }

                        if (c == '.') {
                            format.precision = 0;
                            c = *(++s);
                            while (c && isdigit(c)) {
                                format.precision = 10 * format.precision
                                        + (c - '0');
                                c = *(++s);
                            }
                        }

                        ++s;

                        switch (c) {
                            case 'i':
                            case 'd':
                                this->print((int32_t) first, format);
                                break;
                            case 'X':
                                format.radix = 16;
                                // No "break;" after 'X' - let it flow into 'u'
                            case 'u':
                                this->print((uint32_t) first, format);
                                break;
                            case 'f':
                            case 's':
                            case 'c':
                                this->print(first, format);
                                break;
                            default:
                                this->put_char(DEFAULT_FILL_CHAR);
                                break;
                        }
                        if (0 == sizeof...(remaining))
                            this->puts(s);
                        else {
                            this->printf(s, remaining...);
                        }
                        return;
                    }
                } else
                    this->put_char(*s);

                ++s;
            }
        }

        /**
         * @brief       Print a single character
         *
         * @param[in]   c       Character to be printed
         * @param       format  Unused
         */
        void print (const char c, const Format format = DEFAULT_FORMAT) const {
            this->put_char(c);
        }

        /**
         * @brief       Print a null-terminated string
         *
         * @param[in]   string[]    String to be printed
         * @param       format      Unused
         */
        void print (const char string[], const Format format = DEFAULT_FORMAT) const {
            this->puts(string);
        }

        /**
         * @brief       Print a null-terminated string followed by a newline ('\n')
         *
         * @param[in]   string[]    String to be printed
         */
        void println (const char string[]) const {
            this->puts(string);
            this->put_char('\n');
        }

        /**
         * @brief   Print a newline ('\n')
         */
        void println () const {
            this->put_char('\n');
        }

        /**
         * @brief       Print a boolean as either "true" or "false"
         *
         * @param[in]   b       Boolean to be printed
         * @param       format
         */
        void print (const bool b, const Format format = DEFAULT_FORMAT) const {
            this->puts(Utility::to_string(b));
        }

        /**
         * @brief       Print a single character
         *
         * @param[in]   x           Unsigned value to be printed
         * @param       format
         */
        void print (const uint32_t x, const Format format = DEFAULT_FORMAT) const {
            switch (format.radix) {
                case 16:
                    this->put_hex(x, format.width, format.fillChar);
                    break;
                default:
                    this->put_uint(x, format.width, format.fillChar);
            }
        }

        /**
         * @brief       Print a single character
         *
         * @param[in]   x           Unsigned value to be printed
         * @param       format
         */
        void print (const int32_t x, const Format format = DEFAULT_FORMAT) const {
            this->put_int(x, format.width, format.fillChar);
        }

        /**
         * @brief       Print a single character
         *
         * @param[in]   x           Unsigned value to be printed
         * @param       format
         */
        void print (const double f, const Format format = DEFAULT_FORMAT) const {
            this->put_float(f, format.width, format.precision, format.fillChar);
        }

        template<typename T>
        const Printer& operator<< (const T arg) const {
            this->print(arg);
            return *this;
        }

    protected:
        PrintCapable *m_printCapable;
        bool m_cooked;
};

}

extern const PropWare::Printer pwOut;
