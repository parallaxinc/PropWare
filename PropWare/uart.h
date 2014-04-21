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
#include <tinyio.h> // TODO: DEBUG LINE; delete
#include <simpletext.h>
#include <PropWare/PropWare.h>
#include <PropWare/pin.h>
#include <PropWare/port.h>

namespace PropWare {

class UART {
    public:
        /** Used as index for an array of PropWare objects */
        static const uint8_t PROPWARE_OBJECT_NUMBER = 4;

    public:
        typedef enum {
            /** No parity */NO_PARITY,
            /** Even parity */EVEN,
            /** Odd parity */ODD
        } Parity;

        /** Number of allocated error codes for UART */
#define UART_ERRORS_LIMIT            16
        /** First UART error code */
#define UART_ERRORS_BASE             64

        /**
         * Error codes - Proceeded by SD, SPI
         */
        typedef enum {
            NO_ERROR = 0,
            BEG_ERROR = UART_ERRORS_BASE,
            BAUD_TOO_HIGH = BEG_ERROR,
            INVALID_STOP_BIT_WIDTH,
            END_ERROR = PropWare::UART::INVALID_STOP_BIT_WIDTH
        } ErrorCode;

    public:
        static const uint8_t DEFAULT_DATA_WIDTH = 8;
        static const PropWare::UART::Parity DEFAULT_PARITY = NO_PARITY;
        static const uint8_t DEFAULT_STOP_BIT_WIDTH = 1;
        static const uint32_t DEFAULT_BAUD = 115200;

        static const uint32_t MAX_BAUD = -1;

    public:
        void set_data_width (const uint8_t dataWidth) {
            this->m_dataWidth = dataWidth;

            this->m_dataMask = 0;
            for (uint8_t i = 0; i < this->m_dataWidth; ++i)
                this->m_dataMask |= 1 << i;

            this->set_total_bits();
            this->set_parity_mask();
        }

        uint8_t get_set_data_width () const {
            return this->m_dataWidth;
        }

        void set_parity (const PropWare::UART::Parity parity) {
            this->m_parity = parity;
            this->set_total_bits();
            this->set_parity_mask();
        }

        PropWare::UART::Parity get_parity () const {
            return this->m_parity;
        }

        PropWare::ErrorCode set_stop_bit_width (const uint8_t stopBitWidth) {
            // Error checking
            if (0 == stopBitWidth)
                return PropWare::UART::INVALID_STOP_BIT_WIDTH;

            this->m_stopBitWidth = stopBitWidth;

            this->set_stop_bit_mask();

            this->set_total_bits();

            return NO_ERROR;
        }

        uint8_t get_stop_bit_width () const {
            return this->m_stopBitWidth;
        }

        PropWare::ErrorCode set_baud_rate (const uint32_t baudRate) {
            if (PropWare::UART::MAX_BAUD < baudRate)
                return PropWare::UART::BAUD_TOO_HIGH;

            this->m_bitCycles = CLKFREQ / baudRate;
            return NO_ERROR;
        }

        uint32_t get_baud_rate () const {
            return CLKFREQ / this->m_bitCycles;
        }

        HUBTEXT void send (uint16_t data) {
            printf("Original data: 0x%04x, %u\n", data, data);
            putBin(data);
            printf("\n");

            // Add parity bit
            this->m_parityMask = 1 << this->m_dataWidth;
            if (PropWare::UART::EVEN == this->m_parity) {
                __asm__ volatile("test %[_data], %[_dataMask] wc \n\t"
                        "muxc %[_data], %[_parityMask]"
                        : [_data] "+r" (data)
                        : [_dataMask] "r" (this->m_dataMask),
                          [_parityMask] "r" (this->m_parityMask));
            } else if (PropWare::UART::ODD == this->m_parity) {
                __asm__ volatile("test %[_data], %[_dataMask] wc \n\t"
                        "muxnc %[_data], %[_parityMask]"
                        : [_data] "+r" (data)
                        : [_dataMask] "r" (this->m_dataMask),
                          [_parityMask] "r" (this->m_parityMask));
            }

            printf("Parity mask: 0x%04x\n", this->m_parityMask);
            printf("Data after parity: 0x%04x\n", data);

            // Add stop bits
            data |= this->m_stopBitMask;

            printf("Data after stop bits: 0x%04x\n", data);

            // Add start bit
            data <<= 1;

            printf("Final data: 0x%04x\n", data);
            printf("                    ");
            putBin(data);
            printf("\n");

            uint32_t waitCycles = CNT + this->m_bitCycles;
            for (uint8_t i = 0; i < this->m_totalBits; i++) {
                waitCycles = waitcnt2(waitCycles, this->m_bitCycles);

                // if (value & 1) OUTA |= this->m_tx else OUTA &= ~this->m_tx; value = value >> 1;
                __asm__ volatile("shr %[_data],#1 wc \n\t"
                        "muxc outa, %[_mask]"
                        : [_data] "+r" (data)
                        : [_mask] "r" (this->m_tx.get_mask()));
//                printf("Data after sending: ");
//                putBin(data);
//                printf("\n");
            }
        }

    protected:
        UART () {
            this->m_dataWidth = UART::DEFAULT_DATA_WIDTH;
            this->m_parity = UART::DEFAULT_PARITY;
            this->set_stop_bit_width(UART::DEFAULT_STOP_BIT_WIDTH);
            this->set_baud_rate(UART::DEFAULT_BAUD);
        }

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

        void set_total_bits () {
            // Total bits = start + data + parity + stop bits
            this->m_totalBits = 1 + this->m_dataWidth + this->m_stopBitWidth;
            if (PropWare::UART::NO_PARITY != this->m_parity)
                ++this->m_totalBits;
        }

        void set_parity_mask () {
            this->m_parityMask = 1 << this->m_dataWidth;
        }

    public: // TODO: DEBUG LINE; change to 'protected'
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

class SimplexUART: public PropWare::UART {
    public:
        SimplexUART (const PropWare::Port::Mask tx) :
                PropWare::UART() {
            this->m_tx.set_mask(tx);
            this->m_tx.set_dir(PropWare::Port::OUT);
            this->m_tx.set();
        }
};

}

#endif /* PROPWARE_UART_H_ */
