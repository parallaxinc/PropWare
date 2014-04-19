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

namespace PropWare {

class UART {
    public:
        /** Used as index for an array of PropWare objects */
        static const uint8_t PROPWARE_OBJECT_NUMBER = 4;

    public:
        typedef enum {
            /** 5 Bits */B5,
            /** 6 Bits */B6,
            /** 7 Bits */B7,
            /** 8 Bits */B8,
            /** 9 Bits */B9
        } DataWidth;

        typedef enum {
            /** No parity */NO_PARITY,
            /** Even parity */EVEN,
            /** Odd parity */ODD
        } Parity;

        typedef enum {
            NO_ERROR = 0
        } ErrorCode;

#ifndef PROPWARE_NO_SAFE_UART
    private:
#else
    public:
#endif
        UART () {
            this->m_mailbox = -1;
            this->m_cog = -1;
        }

    public:
        static UART* getInstance () {
            // TODO: Add a lock to make this thread-safe
            static UART instance;
            return &instance;
        }

        static atomic_t s;

        PropWare::ErrorCode start (const PropWare::Port::Mask tx,
                const PropWare::Port::Mask rx, const int32_t baud,
                const UART::DataWidth dataWidth, const UART::Parity parity,
                const uint8_t stopBits);

        PropWare::ErrorCode stop () {
            if (!this->is_running())
                return UART::NO_ERROR;

            cogstop(this->m_cog);
            this->m_cog = -1;
            this->m_mailbox = -1;

            return UART::NO_ERROR;
        }

        bool is_running () {
            return -1 != this->m_cog;
        }

        PropWare::ErrorCode wait () {
            const uint32_t timeoutCnt = UART::WR_TIMEOUT_VAL + CNT;

            // Wait for GAS cog to read in value and write -1
            while ((uint32_t) -1 != this->m_mailbox)
                if (abs(timeoutCnt - CNT) < UART::TIMEOUT_WIGGLE_ROOM)
                    return UART::TIMEOUT;

            return UART::NO_ERROR;
        }

        PropWare::ErrorCode wait_specific (const uint32_t value) {
            const uint32_t timeoutCnt = UART::WR_TIMEOUT_VAL + CNT;

            // Wait for GAS cog to read in value and write -1
            while (value == this->m_mailbox)
                if (abs(timeoutCnt - CNT) < UART::TIMEOUT_WIGGLE_ROOM)
                    // Always use return instead of spi_error() for private
                    // f0unctions
                    return UART::TIMEOUT;

            return UART::NO_ERROR;
        }

        PropWare::ErrorCode set_bit_width (const uint8_t dataWidth);

        PropWare::ErrorCode set_parity (const PropWare::UART::Parity parity);

        PropWare::ErrorCode set_stop_bit_width (const uint8_t stopBitWidth);

        PropWare::ErrorCode set_baud_rate (const uint32_t baudRate);

        PropWare::ErrorCode send (const uint16_t data);

    protected:
        atomic_t m_mailbox;
        uint8_t m_cog;
};

}

#endif /* PROPWARE_UART_H_ */
