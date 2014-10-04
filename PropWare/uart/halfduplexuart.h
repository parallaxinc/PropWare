/**
 * @file        halfduplexuart.h
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

#include <PropWare/uart/abstractduplexuart.h>

namespace PropWare {

/**
 * @brief   Simple wrapper for full duplex that provides half duplex capability
 *
 * It is important to note that, just like PropWare::FullDuplexUART, receiving
 * data is an indefinitely blocking call
 */
class HalfDuplexUART: public AbstractDuplexUART {
    public:
        /**
         * @see PropWare::SimplexUART::SimplexUART()
         */
        HalfDuplexUART () :
                AbstractDuplexUART() {
        }

        /**
         * @see PropWare::FullDuplexUART::FullDuplexUART()
         */
        HalfDuplexUART (const Port::Mask pinMask) :
                AbstractDuplexUART(pinMask, pinMask) {
        }

        /**
         * @see PropWare::UART::send()
         */
        HUBTEXT virtual void send (uint16_t originalData) const {
            // Set pin as output
            __asm__ volatile ("or outa, %0" : : "r" (this->m_tx.get_mask()));
            __asm__ volatile ("or dira, %0" : : "r" (this->m_tx.get_mask()));

            this->AbstractSimplexUART::send(originalData);

            // Set pin as input
            __asm__ volatile ("andn dira, %0" : : "r" (this->m_rx.get_mask()));
        }

        /**
         * @see PropWare::UART::send_array()
         */
        HUBTEXT virtual void send_array (const char array[],
                uint32_t words) const {
            // Set pin as output
            __asm__ volatile ("or outa, %0" : : "r" (this->m_tx.get_mask()));
            __asm__ volatile ("or dira, %0" : : "r" (this->m_tx.get_mask()));

            this->AbstractSimplexUART::send_array(array, words);

            // Set pin as input
            __asm__ volatile ("andn dira, %0" : : "r" (this->m_rx.get_mask()));
        }

        /**
         * @see PropWare::FullDuplexUART::receive()
         */
        HUBTEXT virtual uint32_t receive () const {
            // Set RX as input
            __asm__ volatile ("andn dira, %0" : : "r" (this->m_rx.get_mask()));

            // Receive data
            return this->AbstractDuplexUART::receive();
        }

        /**
         * @see PropWare::FullDuplexUART::receive_array()
         */
        HUBTEXT virtual ErrorCode receive_array (char *buffer,
                uint32_t words) const {
            // Set RX as input
            __asm__ volatile ("andn dira, %0" : : "r" (this->m_rx.get_mask()));

            // Receive data
            return this->AbstractDuplexUART::receive_array(buffer, words);
        }
};

}
