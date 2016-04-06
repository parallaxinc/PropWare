/**
 * @file        PropWare/serial/uart/uarttx.h
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

#include <PropWare/hmi/output/printcapable.h>
#include <PropWare/serial/uart/uart.h>
#include <PropWare/gpio/pin.h>

namespace PropWare {

class UARTTX : public UART,
               public PrintCapable {
    public:
        UARTTX ()
                : UART() {
            this->set_tx_mask((Port::Mask const) (1 << *UART::PARALLAX_STANDARD_TX));
        }

        UARTTX (const Pin::Mask tx)
                : UART() {
            this->set_tx_mask(tx);
        }

        virtual void set_tx_mask (const Port::Mask tx) {
            // Reset the old pin
            this->m_pin.set_dir(Port::IN);
            this->m_pin.clear();

            this->m_pin.set_mask(tx);
            this->m_pin.set();
            this->m_pin.set_dir(Port::OUT);
        }

        Port::Mask get_tx_mask () const {
            return this->m_pin.get_mask();
        }

        virtual void send (uint16_t originalData) const {
            uint32_t wideData = originalData;

            // Add parity bit
            if (UART::EVEN_PARITY == this->m_parity) {
                __asm__ volatile("test %[_data], %[_dataMask] wc \n\t"
                        "muxc %[_data], %[_parityMask]"
                : [_data] "+r"(wideData)
                : [_dataMask] "r"(this->m_dataMask),
                [_parityMask] "r"(this->m_parityMask));
            } else if (UART::ODD_PARITY == this->m_parity) {
                __asm__ volatile("test %[_data], %[_dataMask] wc \n\t"
                        "muxnc %[_data], %[_parityMask]"
                : [_data] "+r"(wideData)
                : [_dataMask] "r"(this->m_dataMask),
                [_parityMask] "r"(this->m_parityMask));
            }

            // Add stop bits
            wideData |= this->m_stopBitMask;

            // Add start bit
            wideData <<= 1;

            this->shift_out_data(wideData, this->m_totalBits, this->m_bitCycles, this->m_pin.get_mask());
        }

        virtual void send_array (const char array[], uint32_t words) const {
            char     *arrayPtr = (char *) array;
            uint32_t data      = 0, waitCycles = 0, bits = 0;

#ifndef DOXYGEN_IGNORE
            switch (this->m_parity) {
                case UART::NO_PARITY:
                    __asm__ volatile (
                    FC_START("SendArrayStart%=", "SendArrayEnd%=")
                            // Prepare next word
                            "sendArrayLoop%=:                                                                   \n\t"
                            "        rdbyte %[_data], %[_arrayPtr]                                              \n\t"
                            // Set start & stop bits
                            "        or %[_data], %[_stopBitMask]                                               \n\t"
                            "        shl %[_data], #1                                                           \n\t"

                            // Send one word
                            "        mov %[_bits], %[_totalBits]                                                \n\t"
                            "        mov %[_waitCycles], %[_bitCycles]                                          \n\t"
                            "        add %[_waitCycles], CNT                                                    \n\t"
                            "sendWordLoop%=:                                                                    \n\t"
                            "        waitcnt %[_waitCycles], %[_bitCycles]                                      \n\t"
                            "        shr %[_data],#1 wc                                                         \n\t"
                            "        muxc outa, %[_mask]                                                        \n\t"
                            "        djnz %[_bits], #" FC_ADDR("sendWordLoop%=", "SendArrayStart%=") "          \n\t"

                            // Increment the pointer and loop
                            "        add %[_arrayPtr], #1                                                       \n\t"
                            "        djnz %[_words], #" FC_ADDR("sendArrayLoop%=", "SendArrayStart%=") "        \n\t"
                            FC_END("SendArrayEnd%=")
                    : [_data] "+r"(data),
                    [_waitCycles] "+r"(waitCycles),
                    [_arrayPtr] "+r"(arrayPtr),
                    [_bits] "+r"(bits),
                    [_words] "+r"(words)
                    : [_mask] "r"(this->m_pin.get_mask()),
                    [_bitCycles] "r"(this->m_bitCycles),
                    [_totalBits] "r"(this->m_totalBits),
                    [_stopBitMask] "r"(this->m_stopBitMask));
                    break;
                case UART::ODD_PARITY:
                    __asm__ volatile (
                    FC_START("SendArrayStart%=", "SendArrayEnd%=")
                            // Prepare next word
                            "sendArrayLoop%=:"
                            "        rdbyte %[_data], %[_arrayPtr]                                              \n\t"
                            // Set parity
                            "        test %[_data], %[_dataMask] wc                                             \n\t"
                            "        muxnc %[_data], %[_parityMask]                                             \n\t"
                            // Set start & stop bits
                            "        or %[_data], %[_stopBitMask]                                               \n\t"
                            "        shl %[_data], #1                                                           \n\t"

                            // Send one word
                            "        mov %[_bits], %[_totalBits]                                                \n\t"
                            "        mov %[_waitCycles], %[_bitCycles]                                          \n\t"
                            "        add %[_waitCycles], CNT                                                    \n\t"
                            "sendWordLoop%=:                                                                    \n\t"
                            "        waitcnt %[_waitCycles], %[_bitCycles]                                      \n\t"
                            "        shr %[_data],#1 wc                                                         \n\t"
                            "        muxc outa, %[_mask]                                                        \n\t"
                            "        djnz %[_bits], #" FC_ADDR("sendWordLoop%=", "SendArrayStart%=") "          \n\t"

                            // Increment the pointer and loop
                            "        add %[_arrayPtr], #1                                                       \n\t"
                            "        djnz %[_words], #" FC_ADDR("sendArrayLoop%=", "SendArrayStart%=") "        \n\t"
                            FC_END("SendArrayEnd%=")
                    : [_data] "+r"(data),
                    [_waitCycles] "+r"(waitCycles),
                    [_arrayPtr] "+r"(arrayPtr),
                    [_bits] "+r"(bits),
                    [_words] "+r"(words)
                    : [_mask] "r"(this->m_pin.get_mask()),
                    [_bitCycles] "r"(this->m_bitCycles),
                    [_totalBits] "r"(this->m_totalBits),
                    [_stopBitMask] "r"(this->m_stopBitMask),
                    [_dataMask] "r"(this->m_dataMask),
                    [_parityMask] "r"(this->m_parityMask));
                    break;
                case UART::EVEN_PARITY:
                    __asm__ volatile (
                    FC_START("SendArrayStart%=", "SendArrayEnd%=")
                            // Prepare next word
                            "sendArrayLoop%=:"
                            "        rdbyte %[_data], %[_arrayPtr]                                              \n\t"
                            // Set parity
                            "        test %[_data], %[_dataMask] wc                                             \n\t"
                            "        muxc %[_data], %[_parityMask]                                              \n\t"
                            // Set start & stop bits
                            "        or %[_data], %[_stopBitMask]                                               \n\t"
                            "        shl %[_data], #1                                                           \n\t"

                            // Send one word
                            "        mov %[_bits], %[_totalBits]                                                \n\t"
                            "        mov %[_waitCycles], %[_bitCycles]                                          \n\t"
                            "        add %[_waitCycles], CNT                                                    \n\t"
                            "sendWordLoop%=:                                                                    \n\t"
                            "        waitcnt %[_waitCycles], %[_bitCycles]                                      \n\t"
                            "        shr %[_data],#1 wc                                                         \n\t"
                            "        muxc outa, %[_mask]                                                        \n\t"
                            "        djnz %[_bits], #" FC_ADDR("sendWordLoop%=", "SendArrayStart%=") "          \n\t"

                            // Increment the pointer and loop
                            "        add %[_arrayPtr], #1                                                       \n\t"
                            "        djnz %[_words], #" FC_ADDR("sendArrayLoop%=", "SendArrayStart%=") "        \n\t"
                            FC_END("SendArrayEnd%=")
                    : [_data] "+r"(data),
                    [_waitCycles] "+r"(waitCycles),
                    [_arrayPtr] "+r"(arrayPtr),
                    [_bits] "+r"(bits),
                    [_words] "+r"(words)
                    : [_mask] "r"(this->m_pin.get_mask()),
                    [_bitCycles] "r"(this->m_bitCycles),
                    [_totalBits] "r"(this->m_totalBits),
                    [_stopBitMask] "r"(this->m_stopBitMask),
                    [_dataMask] "r"(this->m_dataMask),
                    [_parityMask] "r"(this->m_parityMask));
                    break;
            }
#endif
        }

        virtual void put_char (const char c) {
            this->send((uint16_t) c);
        }

        virtual void puts (const char string[]) {
            const uint32_t length = strlen(string);
            if (length)
                this->send_array(string, length);
        }

    protected:
        /**
         * @brief       Shift out one word of data (FCache function)
         *
         * @param[in]   data        A fully configured, ready-to-go, data word
         * @param[in]   bits        Number of shiftable bits in the data word
         * @param[in]   bitCycles   Delay between each bit; Unit is clock cycles
         * @param[in]   txMask      Pin mask of the TX pin
         */
        inline void shift_out_data (uint32_t data, uint32_t bits, const uint32_t bitCycles,
                                    const uint32_t txMask) const {
#ifndef DOXYGEN_IGNORE
            volatile uint32_t waitCycles = bitCycles;
            __asm__ volatile (
            FC_START("ShiftOutDataStart%=", "ShiftOutDataEnd%=")
                    "        add %[_waitCycles], CNT                                           \n\t"

                    "loop%=:                                                                   \n\t"
                    "        waitcnt %[_waitCycles], %[_bitCycles]                             \n\t"
                    "        shr %[_data],#1 wc                                                \n\t"
                    "        muxc outa, %[_mask]                                               \n\t"
                    "        djnz %[_bits], #" FC_ADDR("loop%=", "ShiftOutDataStart%=") "      \n\t"
                    FC_END("ShiftOutDataEnd%=")
            : [_data] "+r"(data),
            [_waitCycles] "+r"(waitCycles),
            [_bits] "+r"(bits)
            : [_mask] "r"(txMask),
            [_bitCycles] "r"(bitCycles));
#endif
        }

    protected:
        Pin m_pin;
};

}
