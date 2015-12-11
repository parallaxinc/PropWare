/**
 * @file        PropWare/uart/fullduplexuart.h
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
#include <PropWare/uart/abstractduplexuart.h>

namespace PropWare {

/**
 * @brief   Full duplex UART communication module
 *
 * Because this class does not use an independent cog for receiving,
 * "Full duplex" may be an exaggeration. Though two separate pins can be
 * used for communication, transmitting and receiving can not happen
 * simultaneously, all receiving calls are indefinitely blocking and there is no
 * receive buffer (data sent to the Propeller will be ignored if execution is
 * not in the receive() method) PropWare::FullDuplexUART::receive() will not
 * return until after the RX pin is low and all data, parity (if applicable) and
 * stop bits have been read.
 */
class FullDuplexUART : public AbstractDuplexUART {
    public:
        /**
         * @see PropWare::AbstractDuplexUART::AbstractDuplexUART()
         */
        FullDuplexUART () :
                AbstractDuplexUART() {
        }

        /**
         * @see PropWare::AbstractDuplexUART::AbstractDuplexUART(
         *              const Port::Mask tx, const Port::Mask rx)
         */
        FullDuplexUART (const Port::Mask tx, const Port::Mask rx) :
                AbstractDuplexUART(tx, rx) {
        }
};

}
