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
#include <PropWare/scancapable.h>

namespace PropWare {

class AbstractDuplexUART: public virtual DuplexUART,
                          public virtual ScanCapable,
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
            ErrorCode err;
            check_errors(AbstractSimplexUART::set_data_width(dataWidth));

            this->set_msb_mask();
            this->set_receivable_bits();

            return NO_ERROR;
        }

        /**
         * @see PropWare::UART::set_parity
         */
        virtual void set_parity (const UART::Parity parity) {
            AbstractSimplexUART::set_parity(parity);
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
                                        this->m_bitCycles,
                                        this->m_rx.get_mask(), this->m_msbMask);

            if (this->m_parity && 0 != this->checkParity(rxVal))
                return (uint32_t) -1;

            return rxVal & wideDataMask;
        }

        /**
         * @see PropWare::UART::receive_array
         */
        HUBTEXT ErrorCode receive_array (char buffer[], int32_t *length, const uint32_t delim = '\n') const {
            if (NULL == length)
                return NULL_POINTER;
            else if (0 == *length)
                *length = INT32_MAX;
            int32_t wordCnt = 0;

            // Check if the total receivable bits can fit within a byte
            if (8 >= this->m_receivableBits) {
                // Set RX as input
                __asm__ volatile ("andn dira, %0" : : "r" (this->m_rx.get_mask()));

                *length = this->shift_in_byte_array((uint32_t) buffer,
                                                    *length,
                                                    delim,
                                                    this->m_receivableBits,
                                                    this->m_bitCycles,
                                                    this->m_rx.get_mask(),
                                                    this->m_msbMask);

                if (NO_PARITY != this->m_parity)
                    for (int32_t i = 0; i < *length; --i)
                        if (0 != this->checkParity((uint32_t) buffer[i]))
                            return UART::PARITY_ERROR;
            }
            // If total receivable bits does not fit within a byte, shift in
            // one word at a time (this offers no speed improvement - it is
            // only here for user convenience)
            else {
                uint32_t temp;

                do {
                    if (-1 == (temp = this->receive()))
                        return UART::PARITY_ERROR;

                    *buffer = temp;
                    ++buffer;
                    ++wordCnt;
                } while (temp != delim && wordCnt < *length);

                *length = wordCnt;
            }

            return NO_ERROR;
        }

        /**
         * @see PropWare::ScanCapable::get_char
         */
        char get_char () {
            return this->receive();
        }

        PropWare::ErrorCode fgets (char string[], int32_t *bufferSize) const {
            const int32_t originalBufferSize = *bufferSize;

            PropWare::ErrorCode err;
            check_errors(this->receive_array(string, bufferSize));

            // Replace delimiter with null-terminator IFF we found one
            if (*bufferSize != originalBufferSize || '\n' == string[originalBufferSize])
                string[*bufferSize - 1] = '\0';
            return NO_ERROR;
        }

    protected:
        /**
         * @see PropWare::SimplexUART::AbstractSimplexUART()
         */
        AbstractDuplexUART () {
            this->set_data_width(UART::DEFAULT_DATA_WIDTH);
            this->set_parity(UART::DEFAULT_PARITY);
            this->set_stop_bit_width(UART::DEFAULT_STOP_BIT_WIDTH);
            this->set_baud_rate(*UART::DEFAULT_BAUD);
            this->set_tx_mask(
                    (Port::Mask const) (1 << *UART::PARALLAX_STANDARD_TX));
            this->set_rx_mask(
                    (Port::Mask const) (1 << *UART::PARALLAX_STANDARD_RX));
        }

        /**
         * @brief       Initialize a UART module with both pin masks
         *
         * @param[in]   tx  Pin mask for TX (transmit) pin
         * @param[in]   rx  Pin mask for RX (receive) pin
         */
        AbstractDuplexUART (const Port::Mask tx, const Port::Mask rx) {
            this->set_data_width(UART::DEFAULT_DATA_WIDTH);
            this->set_parity(UART::DEFAULT_PARITY);
            this->set_stop_bit_width(UART::DEFAULT_STOP_BIT_WIDTH);
            this->set_baud_rate(*UART::DEFAULT_BAUD);

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
        uint32_t shift_in_data (uint_fast8_t bits, const uint32_t bitCycles, const register uint32_t rxMask,
                                const uint32_t msbMask) const {
            volatile uint32_t data = 0;
            volatile uint32_t waitCycles = bitCycles;

#ifndef DOXYGEN_IGNORE
            __asm__ volatile (
                    "        fcache #(ShiftInDataEnd - ShiftInDataStart)                    \n\t"
                    "        .compress off                                                  \n\t"

                    "ShiftInDataStart:                                                      \n\t"
                    "       shr %[_waitCycles], #1                                          \n\t"
                    "       add %[_waitCycles], %[_bitCycles]                               \n\t"
                    "       waitpne %[_rxMask], %[_rxMask]                                  \n\t"
                    "       add %[_waitCycles], CNT                                         \n\t"

                    // Receive a word
                    "loop%=:                                                                \n\t"
                    "       waitcnt %[_waitCycles], %[_bitCycles]                           \n\t"
                    "       shr %[_data],# 1                                                \n\t"
                    "       test %[_rxMask],ina wz                                          \n\t"
                    "       muxnz %[_data], %[_msbMask]                                     \n\t"
                    "       djnz %[_bits], #__LMM_FCACHE_START+(loop%= - ShiftInDataStart)  \n\t"

                    // Wait for a stop bit
                    "       waitpeq %[_rxMask], %[_rxMask]                                  \n\t"

                    "       jmp __LMM_RET                                                   \n\t"
                    "ShiftInDataEnd:                                                        \n\t"
                    "       .compress default                                               \n\t"
                    :// Outputs
                    [_data] "+r"(data),
                    [_waitCycles] "+r" (waitCycles),
                    [_bits] "+r" (bits)
                    :// Inputs
                    [_rxMask] "r"(rxMask),
                    [_msbMask] "r"(msbMask),
                    [_bitCycles] "r" (bitCycles));
#endif

            return data;
        }

        /**
         * @brief       Shift in an array of data (FCache function)
         *
         * @param[in]   bufferAddr
         * @param[in]   maxLength
         * @param[in]   delim
         * @param[in]   bits
         * @param[in]   bitCycles
         * @param[in]   rxMask
         * @param[in]   msbMask
         */
#ifndef DOXYGEN_IGNORE
        __attribute__ ((fcache))
#endif
        uint32_t shift_in_byte_array (register uint32_t bufferAddr, int32_t maxLength, const register char delim,
                                      const register uint32_t bits, const register uint32_t bitCycles,
                                      const register uint32_t rxMask, const register uint32_t msbMask) const {
#ifndef DOXYGEN_IGNORE
            volatile register uint32_t data           = 0;
            volatile register int32_t  wordCnt        = 0;
            volatile register uint32_t bitIdx         = bits;
            volatile register uint32_t waitCycles;
            volatile register uint32_t initWaitCycles = (bitCycles >> 1) + bitCycles;

            /**
             * FIXME: Despite careful use of do{}while(--var); loops, this is still not be compiled correctly.
             *        This method probably needs to be written entirely in assembly
             */

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
                for (; --bitIdx;) {
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
                };

                __asm__ volatile (
                        // Write the word back to the buffer in HUB memory
                        "wrbyte %[_data], %[_bufAdr]\n\t"

                        // Wait for the stop bits
                        "waitpeq %[_rxMask], %[_rxMask]\n\t"

                        // Clear the data register
                        "mov %[_data], #0\n\t"

                        // Increment the buffer address and total word count
                        "add %[_wordCnt], #1\n\t"

                        :// Outputs
                        [_bufAdr] "+r" (bufferAddr),
                        [_wordCnt] "+r" (wordCnt),
                        [_data] "+r" (data)
                        : [_rxMask] "r" (rxMask));
            } while (*((char *)bufferAddr++) != delim && wordCnt < maxLength);

            return (uint32_t) wordCnt;
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
