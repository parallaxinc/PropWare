/**
 * @file    PropWare/serial/uart/fullduplexserial.h
 *
 * @author  Chip Gracey
 * @author  Jeff Martin
 * @author  David Zemon
 *
 * @copyright
 * The MIT License (MIT)<br>
 * <br>2006-2009 Parallax, Inc.<br>
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

#include <PropWare/PropWare.h>
#include <PropWare/serial/uart/uartcommondata.h>
#include <PropWare/hmi/output/printcapable.h>
#include <PropWare/hmi/input/scancapable.h>

namespace PropWare {

void *get_full_duplex_serial_driver ();

/**
 * @brief   Converted to C++ using spin2cpp and then modified to become a PrintCapable object in PropWare's arsenal.
 */
class FullDuplexSerial : public PrintCapable,
                         public ScanCapable {
    public:
        typedef enum {
            INVERT_RX            = BIT_0,
            INVERT_TX            = BIT_1,
            OPEN_DRAIN_SOURCE_TX = BIT_2,
            IGNORE_TX_ECHO_ON_RX = BIT_3
        } Mode;

        static const size_t BUFFER_SIZE = 16;

    public:
        /**
         * Construct a full-duplex, buffered UART instance
         *
         * This object requires a dedicated cog to run the driver code. The driver must be started by invoking
         * PropWare::FullDuplexSerial::start() on this object.
         *
         * @param rxPinNumber   Pin number to receive data
         * @param txPinNumber   Pin number to transmit data
         * @param mode          Combination of some, none, or all of the Mode values which can change the behavior of
         *                      the device
         * @param baudrate      Baudrate to run the transmit and recieve routines
         */
        FullDuplexSerial (const int rxPinNumber = _cfg_rxpin, const int txPinNumber = _cfg_txpin,
                          const uint32_t mode = 0, const int baudrate = _cfg_baudrate)
                : m_transmitLock(locknew()),
                  m_stringLock(locknew()),
                  m_cogID(-1),
                  m_receivePinNumber(rxPinNumber),
                  m_transmitPinNumber(txPinNumber),
                  m_mode(mode),
                  m_bitTicks(CLKFREQ / baudrate),
                  m_bufferPointer((uint32_t) this->m_receiveBuffer) {
        }

        /**
         * @brief   Stop the driver cog and return the locks
         */
        ~FullDuplexSerial () {
            if (-1 != this->m_cogID)
                cogstop(this->m_cogID);
            lockret(this->m_transmitLock);
            lockret(this->m_stringLock);
        }

        /**
         * @brief   Start the driver cog
         *
         * @return  Cog ID of the driver cog. -1 for failure
         */
        int start () {
            return this->m_cogID = cognew(get_full_duplex_serial_driver(), (int32_t) (&this->m_receiveHead));
        }

        /**
         * @brief   Empty the receive buffer
         */
        void truncate () {
            char c;
            while (this->get_char_non_blocking(c));
        }

        /**
         * @brief   Find out if a byte is waiting in the receive buffer
         *
         * @return  True if a byte is waiting, false otherwise
         */
        bool receive_ready () const {
            return this->m_receiveHead != this->m_receiveTail;
        }

        /**
         * @brief       Check if byte received (never waits)
         *
         * @param[out]  c   Byte received from the buffer
         *
         * @return      True if `c` is valid, false otherwise
         */
        bool get_char_non_blocking (char &c) {
            if (this->receive_ready()) {
                c = this->m_receiveBuffer[this->m_receiveTail];
                this->m_receiveTail = (this->m_receiveTail + 1) & 0xf;
                return true;
            } else
                return false;
        }

        /**
         * @brief       Wait for a byte to be received and return after a timeout
         *
         * @param[out]  c           Byte received from the buffer
         * @param[in]   timeout     Timeout (in clock ticks) before exiting the function
         *
         * @return      True if `c` is valid, false if no character was available before the timeout
         */
        bool get_char (char &c, const unsigned int timeout) {
            const unsigned int startTime = CNT;
            bool               success;
            while (!(success = this->get_char_non_blocking(c))
                    && ((CNT - startTime) < timeout));
            return success;
        }

        char get_char () {
            char c;
            while (!this->get_char_non_blocking(c));
            return c;
        }

        void put_char (const char c) {
            // Send byte (may wait for room in buffer)
            while (lockset(this->m_transmitLock));
            while (this->m_transmitTail == ((this->m_transmitHead + 1) & 0xf));
            this->m_transmitBuffer[this->m_transmitHead] = c;
            this->m_transmitHead = (this->m_transmitHead + 1) & 0xf;
            lockclr(this->m_transmitLock);
            if (this->m_mode & IGNORE_TX_ECHO_ON_RX)
                this->get_char();
        }

        void puts (const char string[]) {
            const unsigned int length = strlen(string);
            while (lockset(this->m_stringLock));
            for (unsigned int i = 0; i < length; i++)
                this->put_char((string++)[0]);
            lockclr(this->m_stringLock);
        }

    protected:
        const uint8_t m_transmitLock;
        const uint8_t m_stringLock;
        int32_t       m_cogID;
        char          m_receiveBuffer[BUFFER_SIZE];
        char          m_transmitBuffer[BUFFER_SIZE];

        // These variables must appear in this order. The assembly code relies on the exact order
        volatile uint32_t m_receiveHead;
        volatile uint32_t m_receiveTail;
        volatile uint32_t m_transmitHead;
        volatile uint32_t m_transmitTail;
        const int         m_receivePinNumber;
        const int         m_transmitPinNumber;
        const uint32_t    m_mode;
        const uint32_t    m_bitTicks;
        const uint32_t    m_bufferPointer;
};

}
