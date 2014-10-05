/**
 * @file        abstractduplexuart.h
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

#include <PropWare/uart/simplexuart.h>
#include <PropWare/uart/duplexuart.h>

namespace PropWare {

class AbstractDuplexUART: public virtual DuplexUART,
                          public AbstractSimplexUART {
    public:
        /**
         * @see PropWare::UART::set_rx_mask
         */
        void set_rx_mask (const Port::Mask rx) {
            this->m_rx.set_mask(rx);
            this->m_rx.set_dir(Port::IN);
        }

        /**
         * @see PropWare::UART::get_rx_mask
         */
        Port::Mask get_rx_mask () const {
            return this->m_rx.get_mask();
        }

        /**
         * @see PropWare::UART::set_data_width
         */
        virtual ErrorCode set_data_width (const uint8_t dataWidth) {
            ErrorCode err = this->AbstractSimplexUART::set_data_width(dataWidth);
            if (err)
                return err;

            this->set_msb_mask();
            this->set_receivable_bits();

            return NO_ERROR;
        }

        /**
         * @see PropWare::UART::set_parity
         */
        virtual void set_parity (const UART::Parity parity) {
            this->AbstractSimplexUART::set_parity(parity);
            this->set_msb_mask();
            this->set_receivable_bits();
        }

        /**
         * @see PropWare::UART::receive
         */
        HUBTEXT uint32_t receive () const {
            uint32_t rxVal;
            uint32_t wideDataMask = this->m_dataMask;

            // Set RX as input
            __asm__ volatile ("andn dira, %0" : : "r" (this->m_rx.get_mask()));

            rxVal = this->shift_in_data(this->m_receivableBits,
                    this->m_bitCycles, this->m_rx.get_mask(), this->m_msbMask);

            if (this->m_parity && 0 != this->checkParity(rxVal))
                return (uint32_t) -1;

            return rxVal & wideDataMask;
        }

        /**
         * @see PropWare::UART::receive_array
         */
        HUBTEXT ErrorCode receive_array (char *buffer,
                uint32_t words) const {
            uint32_t wideData;

            // Set RX as input
            __asm__ volatile ("andn dira, %0" : : "r" (this->m_rx.get_mask()));

            // Check if the total receivable bits can fit within a byte
            if (8 >= this->m_receivableBits) {
                this->shift_in_array((uint32_t) buffer, words,
                        this->m_receivableBits, this->m_bitCycles,
                        this->m_rx.get_mask(), this->m_msbMask);

                for (uint32_t i = words; i; --i) {
                    wideData = (uint32_t) buffer[i];
                    if (0 != this->checkParity(wideData))
                        return UART::PARITY_ERROR;
                }
            }
            // If total receivable bits does not fit within a byte, shift in
            // one word at a time (this offers no speed improvement - it is
            // only here for user convenience)
            else
                do {
                    *buffer = (char) this->shift_in_data(this->m_receivableBits,
                            this->m_bitCycles, this->m_rx.get_mask(),
                            this->m_msbMask);
                    if (-1 == *buffer)
                        return UART::PARITY_ERROR;
                    ++buffer;
                } while (--words);

            return NO_ERROR;
        }

    protected:
        /**
         * @see PropWare::SimplexUART::AbstractSimplexUART()
         */
        AbstractDuplexUART () :
                AbstractSimplexUART() {
        }

        /**
         * @brief       Initialize a UART module with both pin masks
         *
         * @param[in]   tx  Pin mask for TX (transmit) pin
         * @param[in]   rx  Pin mask for RX (receive) pin
         */
        AbstractDuplexUART (const Port::Mask tx, const Port::Mask rx) :
                AbstractSimplexUART() {
            this->AbstractSimplexUART::set_data_width(this->m_dataWidth);

            // Set rx direction second so that, in the case of half-duplex, the
            // pin floats high
            this->set_tx_mask(tx);
            this->set_rx_mask(rx);
        }

        /**
         * @brief   Set a bit-mask for the data word's MSB (assuming LSB is bit
         *          0 - the start bit is not taken into account)
         */
        void set_msb_mask () {
            if (this->m_parity)
                this->m_msbMask = (Port::Mask) (1 << this->m_dataWidth);
            else
                this->m_msbMask = (Port::Mask) (1 << (this->m_dataWidth - 1));
        }

        /**
         * @brief   Set the number of receivable bits - based on data width and
         *          parity selection
         */
        void set_receivable_bits () {
            if (this->m_parity)
                this->m_receivableBits = (uint8_t) (this->m_dataWidth + 1);
            else
                this->m_receivableBits = this->m_dataWidth;
        }

        /**
         * Shift in one word of data (FCache function)
         */
#ifndef DOXYGEN_IGNORE
        __attribute__ ((fcache))
#endif
        uint32_t shift_in_data (register uint32_t bits,
                const register uint32_t bitCycles,
                const register uint32_t rxMask,
                const register uint32_t msbMask) const {
            volatile register uint32_t data;
            volatile register uint32_t waitCycles;

#ifndef DOXYGEN_IGNORE
            __asm__ volatile (
                    // Initialize the waitCycles variable
                    "mov %[_waitCycles], %[_bitCycles]\n\t"
                    "shr %[_waitCycles], #1\n\t"
                    "add %[_waitCycles], %[_bitCycles]\n\t"

                    // Wait for the start bit
                    "waitpne %[_rxMask], %[_rxMask]\n\t"

                    // Begin the timer
                    "add %[_waitCycles], CNT \n\t"
                    :// Outputs
                    [_waitCycles] "+r" (waitCycles)
                    :// Inputs
                    [_rxMask] "r" (rxMask),
                    [_bitCycles] "r" (bitCycles));

            do {
                __asm__ volatile (
                        // Wait for the next bit
                        "waitcnt %[_waitCycles], %[_bitCycles]\n\t"
                        "shr %[_data],# 1\n\t"
                        "test %[_rxMask],ina wz \n\t"
                        "muxnz %[_data], %[_msbMask]"
                        :// Outputs
                        [_waitCycles] "+r" (waitCycles),
                        [_data] "+r" (data)
                        :// Inputs
                        [_bitCycles] "r" (bitCycles),
                        [_rxMask] "r" (rxMask),
                        [_msbMask] "r" (msbMask));
            } while (--bits);

            __asm__ volatile ("waitpeq %[_rxMask], %[_rxMask]"
                    :  // No outputs
                    : [_rxMask] "r" (rxMask));
#endif
            return data;
        }

        /**
         * @brief       Shift in an array of data (FCache function)
         *
         * @param[in]   bufferAddr
         * @param[in]   words
         * @param[in]   bits
         * @param[in]   bitCycles
         * @param[in]   rxMask
         * @param[in]   msbMask
         */
#ifndef DOXYGEN_IGNORE
        __attribute__ ((fcache))
#endif
        void shift_in_array (register uint32_t bufferAddr,
                register uint32_t words, const register uint32_t bits,
                const register uint32_t bitCycles,
                const register uint32_t rxMask,
                const register uint32_t msbMask) const {
#ifndef DOXYGEN_IGNORE
            volatile register uint32_t data           = 0;
            volatile register uint32_t bitIdx         = bits;
            volatile register uint32_t waitCycles;
            volatile register uint32_t initWaitCycles = (bitCycles >> 1)
                    + bitCycles;

            do {
                /**
                 *  Receive one word
                 */

                // Initialize variables
                __asm__ volatile (
                        // Initialize the index variable
                        "mov %[_bitIdx], %[_bits]\n\t"

                        // Re-initialize the timer
                        "mov %[_waitCycles], %[_initWaitCycles]\n\t"

                        // Wait for the start bit
                        "waitpne %[_rxMask], %[_rxMask]\n\t"

                        // Begin the timer
                        "add %[_waitCycles], CNT \n\t"

                        :// Outputs
                        [_bitIdx] "+r" (bitIdx),
                        [_waitCycles] "+r" (waitCycles)
                        :// Inputs
                        [_rxMask] "r" (rxMask),
                        [_bits] "r" (bits),
                        [_bitCycles] "r" (bitCycles),
                        [_initWaitCycles] "r" (initWaitCycles));

                // Perform receive loop
                do {
                    __asm__ volatile (
                            // Wait for the next bit
                            "waitcnt %[_waitCycles], %[_bitCycles]\n\t"
                            "shr %[_data],# 1\n\t"
                            "test %[_rxMask],ina wz \n\t"
                            "muxnz %[_data], %[_msbMask]\n\t"
                            :// Outputs
                            [_waitCycles] "+r" (waitCycles),
                            [_data] "+r" (data)
                            :// Inputs
                            [_bitCycles] "r" (bitCycles),
                            [_rxMask] "r" (rxMask),
                            [_msbMask] "r" (msbMask));
                } while (--bitIdx);

                __asm__ volatile (
                        // Write the word back to the buffer in HUB memory
                        "wrbyte %[_data], %[_bufAdr]\n\t"

                        // Wait for the stop bits
                        "waitpeq %[_rxMask], %[_rxMask]\n\t"

                        // Clear the data register
                        "mov %[_data], #0\n\t"

                        // Increment the buffer address
                        "add %[_bufAdr], #1"

                        :// Outputs
                        [_bufAdr] "+r" (bufferAddr),
                        [_data] "+r" (data)
                        : [_rxMask] "r" (rxMask));
            } while (--words);
#endif
        }

        /**
         * @brief       Check parity for a received value
         *
         * @param[in]   rxVal   Received value with parity bit exactly as
         *                      received
         *
         * @return      0 for proper parity; -1 for parity error
         */
        HUBTEXT ErrorCode checkParity (uint32_t rxVal) const {
            uint32_t evenParityResult;
            uint32_t wideParityMask = this->m_parityMask;
            uint32_t wideDataMask   = this->m_dataMask;

            evenParityResult = 0;
            __asm__ volatile("test %[_data], %[_dataMask] wc \n\t"
                    "muxc %[_parityResult], %[_parityMask]"
                    : [_parityResult] "+r" (evenParityResult)
                    : [_data] "r" (rxVal),
                    [_dataMask] "r" (wideDataMask),
                    [_parityMask] "r" (wideParityMask));

            if (UART::ODD_PARITY == this->m_parity) {
                if (evenParityResult != (rxVal & this->m_parityMask))
                    return UART::PARITY_ERROR;
            } else if (evenParityResult == (rxVal & this->m_parityMask))
                return UART::PARITY_ERROR;

            return UART::NO_ERROR;
        }

    protected:
        Pin        m_rx;
        Port::Mask m_msbMask;
        uint8_t    m_receivableBits;
};

}
