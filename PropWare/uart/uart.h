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
#include <PropWare/printcapable.h>

namespace PropWare {

/**
 * @brief    Interface for all UART devices
 */
class UART : public virtual PrintCapable {
    public:
        typedef enum {
            /** No parity */  NO_PARITY,
            /** Even parity */EVEN_PARITY,
            /** Odd parity */ ODD_PARITY
        } Parity;

        /** Number of allocated error codes for UART */
#define UART_ERRORS_LIMIT            16
        /** First UART error code */
#define UART_ERRORS_BASE             64

        /**
         * Error codes - Proceeded by SD, SPI, and HD44780
         */
        typedef enum {
            /** No errors; Successful completion of the function */                NO_ERROR      = 0,
            /** First error code for PropWare::UART */                             BEG_ERROR = UART_ERRORS_BASE,
            /** The requested baud rate is too high */                             BAUD_TOO_HIGH = BEG_ERROR,
            /** A parity error has occurred during read */                         PARITY_ERROR,
            /** The requested data width is not between 1 and 16 (inclusive) */    INVALID_DATA_WIDTH,
            /** The requested stop bit width is not between 1 and 14 (inclusive) */INVALID_STOP_BIT_WIDTH,
            /** Null pointer was passed as an argument */                          NULL_POINTER,
            /** Last error code used by PropWare::UART */                          END_ERROR     = UART::NULL_POINTER
        } ErrorCode;

    public:
        static const uint8_t      DEFAULT_DATA_WIDTH     = 8;
        static const UART::Parity DEFAULT_PARITY         = NO_PARITY;
        static const uint8_t      DEFAULT_STOP_BIT_WIDTH = 1;

        static const int MAX_BAUD = 4413793;

        static const int *DEFAULT_BAUD;
        static const int *PARALLAX_STANDARD_TX;
        static const int *PARALLAX_STANDARD_RX;

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
        virtual void set_baud_rate (const int32_t baudRate) = 0;

        /**
         * @brief   Retrieve the current baud rate
         *
         * @return  Returns an approximation  of the current baud rate; Value is
         *          not exact due to integer math
         */
        virtual int32_t get_baud_rate () const = 0;

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
};

}
