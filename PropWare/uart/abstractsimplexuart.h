/**
 * @file        abstractsimplexuart.h
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

#include <PropWare/uart/uart.h>

namespace PropWare {

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
    <li>Max speed [baud]:
        <ul>
            <li>Send
                <ul>
                    <li>CMM: 1,428,550</li>
                    <li>LMM: 1,428,550</li>
                </ul>
            </li>
            <li>Receive
                <ul>
                    <li>CMM: 740,720</li>
                    <li>LMM: 740,720</li>
                </ul>
            </li>
        </ul>
    </li>
    <li>Max transmit speed [average bitrate of PropWare::UART::puts() w/ 8N1
    config]:
        <ul>
            <li>CMM: 339,227</li>
            <li>LMM: 673,230</li>
        </ul>
    </li>
    <li>TODO: Determine maximum baudrate that receive_array() and receive() can
    read data in 8N1 configuration with minimum stop-bits between each word</li>
    <li>PropWare::UART::send() vs PropWare::UART::puts() minimum delay between
    each character
         <ul>
             <li>CMM:
                 <ul>
                     <li>send: 40.6 us</li>
                     <li>puts: 17.3 us</li>
                 </ul>
             </li>
             <li>LMM:
                 <ul>
                     <li>send: 11.3 us</li>
                     <li>puts: 5.75 us</li>
                 </ul>
             </li>
         </ul>
     </li>
 </ul>
@endhtmlonly
 */
class AbstractSimplexUART : public virtual UART {
    public:
        /**
         * @see PropWare::UART::set_tx_mask
         */
        void set_tx_mask (const Port::Mask tx) {
            this->m_tx.set_mask(tx);
            this->m_tx.set();
            this->m_tx.set_dir(Port::OUT);
        }

        /**
         * @see PropWare::UART::get_tx_mask
         */
        Port::Mask get_tx_mask () const {
            return this->m_tx.get_mask();
        }

        /**
         * @see PropWare::UART::set_data_width
         */
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

        /**
         * @see PropWare::UART::get_data_width
         */
        uint8_t get_data_width () const {
            return this->m_dataWidth;
        }

        /**
         * @see PropWare::UART::set_parity
         */
        virtual void set_parity (const UART::Parity parity) {
            this->m_parity = parity;
            this->set_parity_mask();
            this->set_stop_bit_mask();
            this->set_total_bits();
        }

        /**
         * @see PropWare::UART::get_parity
         */
        UART::Parity get_parity () const {
            return this->m_parity;
        }

        /**
         * @see PropWare::UART::set_stop_bit_width
         */
        ErrorCode set_stop_bit_width (const uint8_t stopBitWidth) {
            // Error checking
            if (0 == stopBitWidth || stopBitWidth > 14)
                return UART::INVALID_STOP_BIT_WIDTH;

            this->m_stopBitWidth = stopBitWidth;

            this->set_stop_bit_mask();

            this->set_total_bits();

            return NO_ERROR;
        }

        /**
         * @see PropWare::UART::get_stop_bit_width
         */
        uint8_t get_stop_bit_width () const {
            return this->m_stopBitWidth;
        }

        /**
         * @see PropWare::UART::set_baud_rate
         */
        void set_baud_rate (const int32_t baudRate) {
            this->m_bitCycles = CLKFREQ / baudRate;
        }

        /**
         * @see PropWare::UART::get_baud_rate
         */
        int32_t get_baud_rate () const {
            return CLKFREQ / this->m_bitCycles;
        }

        /**
         * @see PropWare::UART::send
         */
        HUBTEXT virtual void send (uint16_t originalData) const {
            uint32_t wideData = originalData;

            // Set pin as output
            this->m_tx.set();
            this->m_tx.set_dir_out();

            // Add parity bit
            if (UART::EVEN_PARITY == this->m_parity) {
                __asm__ volatile("test %[_data], %[_dataMask] wc \n\t"
                        "muxc %[_data], %[_parityMask]"
                        : [_data] "+r" (wideData)
                        : [_dataMask] "r" (this->m_dataMask),
                        [_parityMask] "r" (this->m_parityMask));
            } else if (UART::ODD_PARITY == this->m_parity) {
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

            this->shift_out_data(wideData, this->m_totalBits, this->m_bitCycles,
                    this->m_tx.get_mask());
        }

        /**
         * @see PropWare::UART::send_array
         */
        HUBTEXT virtual void send_array (const char array[],
                                         uint32_t words) const {
                char *arrayPtr = (char *) array;
                register uint32_t wideData;
                register uint32_t dataMask    = this->m_dataMask;
                register uint32_t parityMask  = this->m_parityMask;
                register uint32_t stopBitMask = this->m_stopBitMask;
                register uint32_t totalBits   = this->m_totalBits;
                register uint32_t bitCycles   = this->m_bitCycles;
                register uint32_t txMask      = this->m_tx.get_mask();

                // Set pin as output
                this->m_tx.set();
                this->m_tx.set_dir_out();

                switch (this->m_parity) {
                    case UART::NO_PARITY:
                        do {
                            // Add stop bits
                            wideData = this->m_stopBitMask | *arrayPtr;

                            // Add start bit
                            wideData <<= 1;

                            this->shift_out_data(wideData, totalBits, bitCycles,
                                    txMask);

                            // Increment the character pointer
                            ++arrayPtr;
                        } while (--words);
                        break;
                    case UART::ODD_PARITY:
                        do {
                            wideData = (uint32_t) *arrayPtr;

                            // Add parity
                            __asm__ volatile(
                                    "test %[_data], %[_dataMask] wc \n\t"
                                    "muxnc %[_data], %[_parityMask] \n\t"
                                    : [_data] "+r" (wideData)
                                    : [_dataMask] "r" (dataMask),
                                    [_parityMask] "r" (parityMask));

                            // Add stop bits
                            wideData |= stopBitMask;

                            // Add start bit
                            wideData <<= 1;

                            this->shift_out_data(wideData, totalBits, bitCycles,
                                    txMask);

                            // Increment the character pointer
                            ++arrayPtr;
                        } while (--words);
                        break;
                    case UART::EVEN_PARITY:
                        do {
                            wideData = (uint32_t) *arrayPtr;

                            // Add parity
                            __asm__ volatile(
                                    "test %[_data], %[_dataMask] wc \n\t"
                                    "muxc %[_data], %[_parityMask]"
                                    : [_data] "+r" (wideData)
                                    : [_dataMask] "r" (this->m_dataMask),
                                    [_parityMask] "r" (this->m_parityMask));

                            // Add stop bits
                            wideData |= this->m_stopBitMask;

                            // Add start bit
                            wideData <<= 1;

                            this->shift_out_data(wideData, totalBits, bitCycles,
                                    txMask);

                            // Increment the character pointer
                            ++arrayPtr;
                        } while (--words);
                        break;
                }
            }

        /**
         * @see PropWare::PrintCapable::put_char
         */
        void put_char (const char c) const {
            this->send((uint16_t) c);
        }

        /**
         * @see PropWare::PrintCapable::puts
         */
        void puts (const char string[]) const {
            const uint32_t length = strlen(string);
            this->send_array(string, length);
        }

    protected:
        /**
         * @brief   Set default values for all configuration parameters; TX mask
         *          must still be set before it can be used
         */
        AbstractSimplexUART () {
            this->set_data_width(UART::DEFAULT_DATA_WIDTH);
            this->set_parity(UART::DEFAULT_PARITY);
            this->set_stop_bit_width(UART::DEFAULT_STOP_BIT_WIDTH);
            this->set_tx_mask(
                    (Port::Mask const) (1 << *UART::PARALLAX_STANDARD_TX));
            this->set_baud_rate(*UART::DEFAULT_BAUD);
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

        /**
         * @brief       Shift out one word of data (FCache function)
         *
         * @param[in]   data        A fully configured, ready-to-go, data word
         * @param[in]   bits        Number of shiftable bits in the data word
         * @param[in]   bitCycles   Delay between each bit; Unit is clock cycles
         * @param[in]   txMask      Pin mask of the TX pin
         */
#ifndef DOXYGEN_IGNORE
        __attribute__ ((fcache))
#endif
        void shift_out_data (register uint32_t data, register uint32_t bits,
                             const register uint32_t bitCycles,
                             const register uint32_t txMask) const {
#ifndef DOXYGEN_IGNORE
            volatile uint32_t waitCycles;

            __asm__ volatile (
                    "mov %[_waitCycles], %[_bitCycles]\n\t"
                    "add %[_waitCycles], CNT \n\t"
                    :  // Outputs
                    [_waitCycles] "+r" (waitCycles)
                    :// Inputs
                    [_bitCycles] "r" (bitCycles));

            do {
                __asm__ volatile(
                        "waitcnt %[_waitCycles], %[_bitCycles]\n\t"
                        "shr %[_data],#1 wc \n\t"
                        "muxc outa, %[_mask]"
                        : [_data] "+r" (data),
                        [_waitCycles] "+r" (waitCycles)
                        : [_mask] "r" (txMask),
                        [_bitCycles] "r" (bitCycles));
            } while (--bits);
#endif
        }

    protected:
        Pin          m_tx;
        uint8_t      m_dataWidth;
        uint16_t     m_dataMask;
        UART::Parity m_parity;
        uint16_t     m_parityMask;
        uint8_t      m_stopBitWidth;
        uint32_t     m_stopBitMask;
        uint32_t     m_bitCycles;
        uint8_t      m_totalBits;
};

}
