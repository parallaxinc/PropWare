/**
 * @file        uart.h
 *
 * @project     PropWare
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

#ifndef PROPWARE_UART_H_
#define PROPWARE_UART_H_

#include <sys/thread.h>
#include <PropWare/PropWare.h>
#include <PropWare/pin.h>
#include <PropWare/port.h>

namespace PropWare {

/**
 * Abstract base class for all UART devices
 *
 * TODO: Finish documenting this class
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
            /** The requested data width is not between 1 and 16 (inclusive) */
            INVALID_DATA_WIDTH,
            /** Valid stop bit width can not be 0 */
            INVALID_STOP_BIT_WIDTH,
            /** Last error code used by PropWare::UART */
            END_ERROR = PropWare::UART::INVALID_STOP_BIT_WIDTH
        } ErrorCode;

    public:
        static const uint8_t DEFAULT_DATA_WIDTH = 8;
        static const PropWare::UART::Parity DEFAULT_PARITY = NO_PARITY;
        static const uint8_t DEFAULT_STOP_BIT_WIDTH = 1;
        static const uint32_t DEFAULT_BAUD = 115200;

        static const uint32_t MAX_BAUD = -1; // TODO: What is the maximum baud?

    public:
        /**
         * @brief       Set the number of bits for each word of data
         *
         * @param[in]   dataWidth   Typical values are between 5 and 9, but any
         *                          value between 1 and 16 is valid
         *
         * @return      Generally 0; PropWare::UART::INVALID_DATA_WIDTH will be
         *              returned if dataWidth is not between 1 and 16
         */
        PropWare::ErrorCode set_data_width (const uint8_t dataWidth) {
            if (1 > dataWidth || dataWidth > 16)
                return PropWare::UART::INVALID_DATA_WIDTH;

            this->m_dataWidth = dataWidth;

            this->m_dataMask = 0;
            for (uint8_t i = 0; i < this->m_dataWidth; ++i)
                this->m_dataMask |= 1 << i;

            this->set_parity_mask();
            this->set_total_bits();

            return PropWare::UART::NO_ERROR;
        }

        /**
         * @brief   Retrieve the currently configured data width
         *
         * @return  Returns a numbers between 1 and 16, inclusive
         */
        uint8_t get_set_data_width () const {
            return this->m_dataWidth;
        }

        /**
         * @brief       Set the parity configuration
         *
         * @param[in]   No parity, even or odd parity can be selected
         */
        void set_parity (const PropWare::UART::Parity parity) {
            this->m_parity = parity;
            this->set_parity_mask();
            this->set_stop_bit_mask();
            this->set_total_bits();
        }

        /**
         * @brief   Retrieve the current parity configuration
         *
         * @return  Current parity configuration
         */
        PropWare::UART::Parity get_parity () const {
            return this->m_parity;
        }

        /**
         * @brief       Set the number of stop bits used
         *
         * @param[in]   Typically either 1 or 2, but can be any number between 1
         *              and 14
         *
         * @return      Returns 0 upon success; Failure can occur when an
         *              invalid value is passed into stopBitWidth
         */
        PropWare::ErrorCode set_stop_bit_width (const uint8_t stopBitWidth) {
            // Error checking
            if (0 == stopBitWidth || stopBitWidth > 14)
                return PropWare::UART::INVALID_STOP_BIT_WIDTH;

            this->m_stopBitWidth = stopBitWidth;

            this->set_stop_bit_mask();

            this->set_total_bits();

            return NO_ERROR;
        }

        /**
         * @brief   Retrieve the current number of stop bits in use
         *
         * @return  Returns a number between 1 and 14 representing the number of
         *          stop bits
         */
        uint8_t get_stop_bit_width () const {
            return this->m_stopBitWidth;
        }

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
        PropWare::ErrorCode set_baud_rate (const uint32_t baudRate) {
            if (PropWare::UART::MAX_BAUD < baudRate)
                return PropWare::UART::BAUD_TOO_HIGH;

            this->m_bitCycles = CLKFREQ / baudRate;
            return NO_ERROR;
        }

        /**
         * @brief   Retrieve the current buad rate
         *
         * @return  Returns an approximation  of the current baud rate; Value is
         *          not exact due to integer math
         */
        uint32_t get_baud_rate () const {
            return CLKFREQ / this->m_bitCycles;
        }

        /**
         * @note    The core loop is taken directly from PropGCC's putchar()
         *          function in tinyio; A big thanks to the PropGCC team for the
         *          simple and elegant algorithm!
         */
        HUBTEXT void send (uint16_t originalData) {
            uint32_t wideData = originalData;

            // Add parity bit
            this->m_parityMask = 1 << this->m_dataWidth;
            if (PropWare::UART::EVEN_PARITY == this->m_parity) {
                __asm__ volatile("test %[_data], %[_dataMask] wc \n\t"
                        "muxc %[_data], %[_parityMask]"
                        : [_data] "+r" (wideData)
                        : [_dataMask] "r" (this->m_dataMask),
                          [_parityMask] "r" (this->m_parityMask));
            } else if (PropWare::UART::ODD_PARITY == this->m_parity) {
                __asm__ volatile("test %[_data], %[_dataMask] wc \n\t"
                        "muxnc %[_data], %[_parityMask]"
                        : [_data] "+r" (wideData)
                        : [_dataMask] "r" (this->m_dataMask),
                          [_parityMask] "r" (this->m_parityMask));
            }

            // Add stop bits
            wideData |= this->m_stopBitMask;

            // Add start bit
            wideData <<= 1;

            uint32_t waitCycles = CNT + this->m_bitCycles;
            for (uint8_t i = 0; i < this->m_totalBits; i++) {
                waitCycles = waitcnt2(waitCycles, this->m_bitCycles);

                // if (value & 1) OUTA |= this->m_tx else OUTA &= ~this->m_tx; value = value >> 1;
                __asm__ volatile("shr %[_data],#1 wc \n\t"
                        "muxc outa, %[_mask]"
                        : [_data] "+r" (wideData)
                        : [_mask] "r" (this->m_tx.get_mask()));
            }
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
            this->set_baud_rate(UART::DEFAULT_BAUD);
        }

        /**
         * @brief   Create a stop bit mask and adjust it shift it based on the
         *          current value of parity
         */
        void set_stop_bit_mask () {
            // Create the mask to the far right
            this->m_stopBitMask = 1;
            for (uint8_t i = 0; i < this->m_stopBitWidth - 1; ++i)
                this->m_stopBitMask |= this->m_stopBitMask << 1;

            // Shift the mask into position (taking into account the current
            // parity settings)
            this->m_stopBitMask <<= this->m_dataWidth;
            if (PropWare::UART::NO_PARITY != this->m_parity)
                this->m_stopBitMask <<= 1;
        }

        /**
         * @brief   Create the parity mask; Takes into account the width of the
         *          data
         */
        void set_parity_mask () {
            this->m_parityMask = 1 << this->m_dataWidth;
        }

        /**
         * @brief       Determine the total number of bits shifted out or in
         *
         * @detailed    Takes into account the start bit, the width of the data,
         *              if there is a parity bit and the number of stop bits
         */
        void set_total_bits () {
            // Total bits = start + data + parity + stop bits
            this->m_totalBits = 1 + this->m_dataWidth + this->m_stopBitWidth;
            if (PropWare::UART::NO_PARITY != this->m_parity)
                ++this->m_totalBits;
        }

    protected:
        PropWare::Pin m_tx;
        uint8_t m_dataWidth;
        uint16_t m_dataMask;
        PropWare::UART::Parity m_parity;
        uint16_t m_parityMask;
        uint8_t m_stopBitWidth;
        uint32_t m_stopBitMask;  // Does not take into account parity bit!
        uint32_t m_bitCycles;
        uint8_t m_totalBits;
};

/**
 * An easy-to-use class for simplex (transmit only) UART communication
 *
 * TODO: Finish documenting this class
 */
class SimplexUART: public PropWare::UART {
    public:
        /**
         * @brief       Construct a UART instance capable of simplex serial
         *              communications
         *
         * @param[in]   Bit mask used for the TX (transmit) pin
         */
        SimplexUART (const PropWare::Port::Mask tx) :
                PropWare::UART() {
            this->m_tx.set_mask(tx);
            this->m_tx.set_dir(PropWare::Port::OUT);
            this->m_tx.set();
        }
};

}

#endif /* PROPWARE_UART_H_ */
