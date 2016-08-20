/**
 * @file        PropWare/serial/uart/uart.h
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

extern "C" {
extern int _cfg_baudrate;
extern int _cfg_txpin;
extern int _cfg_rxpin;
}

namespace PropWare {

#ifdef __PROPELLER_COG__
#define virtual
#endif

/**
 * @brief   Abstract base class for all unbuffered UART devices
 *
 * Configurable with the following options:
 * <ul><li>Data width: 1-16 bits</li>
 * <li>Parity: No parity, odd parity, even parity</li>
 * <li>Stop bits: Any number of stop bits between 1 and 14
 * </li></ul>
 *
 * @note    Total number of bits within start, data, parity, and stop cannot
 *          exceed 32. For instance, a configuration of 16 data bits, even or
 *          odd parity, and 2 stop bits would be 1 + 16 + 1 + 2 = 20 (this is
 *          allowed). A configuration of 16 data bits, no parity, and 16 stop
 *          bits would be 1 + 16 + 0 + 16 = 33 (not allowed).
 *
 * @note    No independent cog is needed for execution and therefore all
 *          communication methods are blocking (cog execution will not return
 *          from the method until the relevant data has been received/sent)
 *
 * Speed tests:
@htmlonly
<ul>
    <li>All tests performed with XTAL @ 80 MHz</li>
    <li>Max burst speed:
        <ul>
            <li>Send: <b>4,444,444 baud</b></li>
            <li>Receive: <b>2,750,000 baud</b></li>
        </ul>
    </li>
    <li>Max transmit throughput (average bitrate of puts/send_array w/ 8N1 config): <b>2,680,144 bps</b></li>
    <li>Transmit delay between words for single- and multi-byte routines
        <ul>
            <li>CMM:
                 <ul>
                     <li>send: <b>63.0 us</b></li>
                     <li>puts/send_array: <b>1.0 us</b></li>
                 </ul>
            </li>
            <li>LMM:
                 <ul>
                     <li>send: <b>15.6 us</b></li>
                     <li>puts/send_array: <b>1.0 us</b></li>
                 </ul>
            </li>
        </ul>
     </li>
 </ul>
@endhtmlonly
 */
class UART {
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

    public:
        virtual ErrorCode set_data_width (const uint8_t dataWidth) {
            if (1 > dataWidth || dataWidth > 16)
                return UART::INVALID_DATA_WIDTH;

            this->m_dataWidth = dataWidth;

            this->m_dataMask = 0;
            for (uint8_t i = 0; i < this->m_dataWidth; ++i)
                this->m_dataMask |= 1 << i;

            this->set_parity_mask();
            this->set_total_bits();

            return UART::NO_ERROR;
        }

        uint8_t get_data_width () const {
            return this->m_dataWidth;
        }

        virtual void set_parity (const UART::Parity parity) {
            this->m_parity = parity;
            this->set_parity_mask();
            this->set_stop_bit_mask();
            this->set_total_bits();
        }

        UART::Parity get_parity () const {
            return this->m_parity;
        }

        ErrorCode set_stop_bit_width (const uint8_t stopBitWidth) {
            // Error checking
            if (0 == stopBitWidth || stopBitWidth > 14)
                return UART::INVALID_STOP_BIT_WIDTH;

            this->m_stopBitWidth = stopBitWidth;

            this->set_stop_bit_mask();

            this->set_total_bits();

            return NO_ERROR;
        }

        uint8_t get_stop_bit_width () const {
            return this->m_stopBitWidth;
        }

        void set_baud_rate (const int32_t baudRate) {
            this->m_bitCycles = CLKFREQ / baudRate;
        }

        int32_t get_baud_rate () const {
            return CLKFREQ / this->m_bitCycles;
        }

    protected:
        /**
         * @brief   Set default values for all configuration parameters; TX mask
         *          must still be set before it can be used
         */
        UART () {
            this->set_data_width(UART::DEFAULT_DATA_WIDTH);
            this->set_parity(UART::DEFAULT_PARITY);
            this->set_stop_bit_width(UART::DEFAULT_STOP_BIT_WIDTH);
            this->set_baud_rate(_cfg_baudrate);
        }

        /**
         * @brief   Create a stop bit mask and adjust it shift it based on the
         *          current value of parity
         */
        void set_stop_bit_mask () {
            // Create the mask to the far right
            this->m_stopBitMask = 1;
            for (uint8_t i      = 0; i < this->m_stopBitWidth - 1; ++i)
                this->m_stopBitMask |= this->m_stopBitMask << 1;

            // Shift the mask into position (taking into account the current
            // parity settings)
            this->m_stopBitMask <<= this->m_dataWidth;
            if (UART::NO_PARITY != this->m_parity)
                this->m_stopBitMask <<= 1;
        }

        /**
         * @brief   Create the parity mask; Takes into account the width of the
         *          data
         */
        void set_parity_mask () {
            this->m_parityMask = (uint16_t) (1 << this->m_dataWidth);
        }

        /**
         * @brief       Determine the total number of bits shifted out or in
         *
         * Takes into account the start bit, the width of the data, if there is
         * a parity bit and the number of stop bits
         */
        void set_total_bits () {
            // Total bits = start + data + parity + stop bits
            this->m_totalBits = (uint8_t) (1 + this->m_dataWidth
                    + this->m_stopBitWidth);
            if (UART::NO_PARITY != this->m_parity)
                ++this->m_totalBits;
        }

    protected:
        uint8_t      m_dataWidth;
        uint16_t     m_dataMask;
        UART::Parity m_parity;
        uint16_t     m_parityMask;
        uint8_t      m_stopBitWidth;
        uint32_t     m_stopBitMask;
        uint32_t     m_bitCycles;
        uint8_t      m_totalBits;
};

#ifdef __PROPELLER_COG__
#undef virtual
#endif

}
