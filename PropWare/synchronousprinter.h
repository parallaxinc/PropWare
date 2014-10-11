/**
 * @file        synchronousprinter.h
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

#include <PropWare/printer.h>

namespace PropWare {

/**
 * @brief   IMPORTANT! SynchronousPrinter is not yet working! DO NOT attempt to
 *          use SynchronousPrinter until this note disappears
 */
class SynchronousPrinter: public Printer {
    public:
        /**
         * @brief   Creates a synchronous instance of a Printer that can be used
         *          from multiple cogs simultaneously.
         *
         * @param   *printCapable   Address of an instance of a
         *                          PropWare::PrintCapable device that can be
         *                          shared across multiple cogs
         */
        SynchronousPrinter (PrintCapable const *printCapable)
                : Printer(printCapable) {
            this->m_lock = locknew();
            lockclr(this->m_lock);
        }

        /**
         * @brief   Ensure that, when a `SynchronousPrinter` is no longer being
         *          used, the lock is returned
         */
        ~SynchronousPrinter () {
            lockclr(this->m_lock);
            lockret(this->m_lock);
        }

        /**
         * @brief   Determine if an instance of a `SynchronousPrinter`
         *          successfully retrieved a lock
         * @return  True when a lock has been retrieved successfully, false
         *          otherwise
         */
        bool hasLock () const {
            return -1 != this->m_lock;
        }

        /**
         * @brief   Retrieve a new lock
         *
         * If this instance already has a lock, the call will block until the
         * lock has been cleared. The lock will then be returned and a new lock
         * will be retrieved.
         *
         * @return  True if the instance was able to successfully retrieve a new
         *          lock
         */
        bool refreshLock () {
            if (this->hasLock()) {
                // Wait for any other cogs using the lock to return
                while(lockset(this->m_lock));
                lockclr(this->m_lock);
                lockret(this->m_lock);
            }

            this->m_lock = locknew();
            return this->hasLock();
        }

        /**
         * @see PropWare::Printer::put_char
         */
        void put_char (const char c) const {
            while (lockset(this->m_lock));
            Printer::put_char(c);
            lockclr(this->m_lock);
        }

        /**
         * @see PropWare::Printer::puts
         */
        void puts (const char string[]) const {
            while (lockset(this->m_lock));
            this->printCapable->puts(string);
            lockclr(this->m_lock);
        }

        /**
         * @see PropWare::Printer::put_int
         */
        void put_int (int32_t x, uint16_t width = 0, const char fillChar = ' ',
                      const bool bypassLock = false) const {
            if (bypassLock)
                Printer::put_int(x, width, fillChar, true);
            else {
                while (lockset(this->m_lock));
                Printer::put_int(x, width, fillChar, true);
                lockclr(this->m_lock);
            }
        }

        /**
         * @see PropWare::Printer::put_uint
         */
        void put_uint (uint32_t x, uint16_t width = 0,
                       const char fillChar = ' ',
                       const bool bypassLock = false) const {
            if (bypassLock)
                Printer::put_uint(x, width, fillChar, true);
            else {
                while (lockset(this->m_lock));
                Printer::put_uint(x, width, fillChar, true);
                lockclr(this->m_lock);
            }
        }

        /**
         * @see PropWare::Printer::put_hex
         */
        void put_hex (uint32_t x, uint16_t width = 0, const char fillChar = ' ',
                      const bool bypassLock = false) const {
            if (bypassLock)
                Printer::put_hex(x, width, fillChar, true);
            else {
                while (lockset(this->m_lock));
                Printer::put_hex(x, width, fillChar, true);
                lockclr(this->m_lock);
            }
        }

#ifdef ENABLE_PROPWARE_PRINT_FLOAT
        /**
         * @see PropWare::Printer::put_float
         */
        void put_float (double f, uint16_t width, uint16_t precision,
                        const char fillChar,
                        const bool bypassLock = false) const {
            if (bypassLock)
                Printer::put_float(f, width, precision, fillChar, true);
            else {
                while (lockset(this->m_lock));
                Printer::put_float(f, width, precision, fillChar, true);
                lockclr(this->m_lock);
            }
        }
#endif
};

}

extern const PropWare::SynchronousPrinter pwSyncOut;
