/**
 * @file        uart.h
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

#include <sys/thread.h>
#include <stdlib.h>
#include <stdarg.h>
#include <PropWare/PropWare.h>
#include <PropWare/pin.h>
#include <PropWare/port.h>

namespace PropWare {

/**
 * @brief    Interface for all UART devices
 */
class UART {
    public:
        typedef enum {
            /** No parity */NO_PARITY,
            /** Even parity */EVEN_PARITY,
            /** Odd parity */ODD_PARITY
        } Parity;

        /** Number of allocated error codes for UART */
#define UART_ERRORS_LIMIT            16
        /** First UART error code */
#define UART_ERRORS_BASE             64

        /**
         * Error codes - Proceeded by SD, SPI, and HD44780
         */
        typedef enum {
            /** No errors; Successful completion of the function */
            NO_ERROR = 0,
            /** First error code for PropWare::UART */
            BEG_ERROR = UART_ERRORS_BASE,
            /** The requested baud rate is too high */
            BAUD_TOO_HIGH = BEG_ERROR,
            /** A parity error has occurred during read */
            PARITY_ERROR,
            /**
             * The requested data width is not between 1 and 16 (inclusive)
             */
            INVALID_DATA_WIDTH,
            /**
             * The requested stop bit width is not between 1 and 14 (inclusive)
             */
            INVALID_STOP_BIT_WIDTH,
            /** Last error code used by PropWare::UART */
            END_ERROR = UART::INVALID_STOP_BIT_WIDTH
        } ErrorCode;

    public:
        static const uint32_t     DEFAULT_BAUD           = 115200;
        static const uint8_t      DEFAULT_DATA_WIDTH     = 8;
        static const UART::Parity DEFAULT_PARITY         = NO_PARITY;
        static const uint8_t      DEFAULT_STOP_BIT_WIDTH = 1;

        static const Port::Mask PARALLAX_STANDARD_TX = Port::P30;
        static const Port::Mask PARALLAX_STANDARD_RX = Port::P31;

    public:
        /**
         * @brief       Set the pin mask for TX pin
         *
         * @param[in]   tx  Pin mask for the transmit (TX) pin
         */
        virtual void set_tx_mask (const Port::Mask tx) = 0;

        /**
         * @brief   Retrieve the currently configured transmit (TX) pin mask
         *
         * @return  Pin mask of the transmit (TX) pin
         */
        virtual Port::Mask get_tx_mask () const = 0;

        /**
         * @brief       Set the number of bits for each word of data
         *
         * @param[in]   dataWidth   Typical values are between 5 and 9, but any
         *                          value between 1 and 16 is valid
         *
         * @return      Generally 0; PropWare::UART::INVALID_DATA_WIDTH will be
         *              returned if dataWidth is not between 1 and 16
         */
        virtual ErrorCode set_data_width (const uint8_t dataWidth) = 0;

        /**
         * @brief   Retrieve the currently configured data width
         *
         * @return  Returns a numbers between 1 and 16, inclusive
         */
        virtual uint8_t get_data_width () const = 0;

        /**
         * @brief       Set the parity configuration
         *
         * @param[in]   parity  No parity, even or odd parity can be selected
         */
        virtual void set_parity (const UART::Parity parity) = 0;

        /**
         * @brief   Retrieve the current parity configuration
         *
         * @return  Current parity configuration
         */
        virtual UART::Parity get_parity () const = 0;

        /**
         * @brief       Set the number of stop bits used
         *
         * @param[in]   stopBitWidth    Typically either 1 or 2, but can be any
         *                              number between 1 and 14
         *
         * @return      Returns 0 upon success; Failure can occur when an
         *              invalid value is passed into stopBitWidth
         */
        virtual ErrorCode set_stop_bit_width (const uint8_t stopBitWidth) = 0;

        /**
         * @brief   Retrieve the current number of stop bits in use
         *
         * @return  Returns a number between 1 and 14 representing the number of
         *          stop bits
         */
        virtual uint8_t get_stop_bit_width () const = 0;

        /**
         * @brief       Set the baud rate
         *
         * @note        Actual baud rate will be approximate due to integer math
         *
         * @param[in]   baudRate    A value between 1 and
         *                          PropWare::UART::MAX_BAUD representing the
         *                          desired baud rate
         *
         * @return      Returns 0 upon success; PropWare::UART::BAUD_TOO_HIGH
         *              when baudRate is set too high for the Propeller's clock
         *              frequency
         */
        virtual void set_baud_rate (const uint32_t baudRate) = 0;

        /**
         * @brief   Retrieve the current baud rate
         *
         * @return  Returns an approximation  of the current baud rate; Value is
         *          not exact due to integer math
         */
        virtual uint32_t get_baud_rate () const = 0;

        /**
         * @brief       Send a word of data out the serial port
         *
         * @pre         Note to UART developers, not users: this->m_tx must be
         *              already configured as output
         *
         * @note        The core loop is taken directly from PropGCC's putchar()
         *              function in tinyio; A big thanks to the PropGCC team for
         *              the simple and elegant algorithm!
         *
         * @param[in]   originalData    Data word to send out the serial port
         */
        virtual void send (uint16_t originalData) const = 0;

        /**
         * @brief       Send a null-terminated character array
         *
         * @pre         `words` must be greater than 0
         *
         * @param[in]   array[] Array of data words
         * @param[in]   words   Number of words to be sent
         */
        HUBTEXT virtual void send_array (const char array[],
                uint32_t words) const = 0;

        /**
         * @brief       Send a null-terminated character array
         *
         * @pre         `string[]` must be terminated with a null terminator
         *
         * @param[in]   string[]    Array of data words with the final word
         *                          being 0 - the null terminator
         */
        virtual void puts (const char string[]) const = 0;

        /**
         * @brief       Print a signed integer in base 10
         *
         * @param[in]   x   Integer to be printed
         */
        void put_int (int32_t x) const {
            if (0 > x)
                this->send('-');

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
                this->send('0');
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
                    this->send((uint16_t) buf[i - j - 1]);
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
                this->send((uint16_t) buf[i - j - 1]);
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
                        this->puts(va_arg(list, char *));
                        break;
                    case 'c':
                        this->send(va_arg(list, int));
                        break;
                    case 'X':
                        this->put_hex(va_arg(list, uint32_t));
                        break;
                    case '%':
                        this->send('%');
                        break;
                    default:
                        va_arg(list, int);  // Increment va_arg pointer
                        this->send(' ');
                        break;
                }
            } else
                this->send((uint16_t) *s);
            ++s;
        }
        va_end(list);
    }
};

}
