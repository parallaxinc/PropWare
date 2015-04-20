/**
 * @file        duplexuart.h
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
 * @brief    Interface for all duplex UART devices
 */
class DuplexUART: public virtual UART {
    public:
        /**
         * @brief       Set the pin mask for RX pin
         *
         * @param[in]   rx  Pin mask for the receive (RX) pin
         */
        virtual void set_rx_mask (const Port::Mask rx) = 0;

        /**
         * @brief   Retrieve the currently configured receive (RX) pin mask
         *
         * @return  Pin mask of the receive (RX) pin
         */
        virtual Port::Mask get_rx_mask () const = 0;

        /**
         * @brief   Receive one word of data; Will block until word is received
         *
         * Cog execution will be blocked by this call and there is no timeout;
         * Execution will not resume until the RX pin is driven low
         *
         * @pre     RX pin mask must be set
         *
         * @return  Data word will be returned unless parity is value is
         *          incorrect; An invalid parity bit will result in -1 being
         *          returned
         */
        virtual uint32_t receive () const = 0;

        /**
         * @brief       Receive an array of data words
         *
         * Cog execution will be blocked by this call and there is no timeout; Execution will not resume until all data
         * words have been received
         *
         * Current configuration must be data width + parity width <= 8 bits. If receivable bits is greater than 8, an
         * error will be thrown without any regard for the data being sent to the Propeller
         *
         * @pre         RX pin mask must be set
         *
         * @param[out]  buffer[]    Address to begin storing data words
         * @param[in]   *length     Pointer to max number of words to receive; If value is 0, INT32_MAX will be used.
         *                          Null-pointer will result in PropWare::UART::NULL_POINTER error code
         * @param[in]   delim       Delimiting character used to determine when reading should stop
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        virtual ErrorCode receive_array (char buffer[], int32_t *length, const uint32_t delim = '\n') const = 0;
};

}
