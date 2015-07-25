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

#include <PropWare/printer/printer.h>

namespace PropWare {

/**
 * @brief   Print formatted text to a serial terminal, an LCD, or any other device from any cog at any time with no
 *          worries about contention.
 *
 * @warning SynchronousPrinter is only software - it can not magically introduce a pull-up resistor on the TX line as
 *          is needed for synchronous printing by various Propeller boards, including the Quickstart.
 */
class SynchronousPrinter {
    public:
        /**
         * @brief   Creates a synchronous instance of a Printer that can be used from multiple cogs simultaneously.
         *
         * @param   *printer    Address of an instance of a PropWare::Printer device that can be shared across
         * multiple cogs
         */
        SynchronousPrinter (const Printer *printer)
                : m_printer(printer),
                  m_lock(locknew()),
                  m_borrowed(false) {
            lockclr(this->m_lock);
        }

        /**
         * @brief   Ensure that, when a `SynchronousPrinter` is no longer being used, the lock is returned
         */
        ~SynchronousPrinter () {
            lockclr(this->m_lock);
            lockret(this->m_lock);
        }

        /**
         * @brief   Determine if an instance of a `SynchronousPrinter` successfully retrieved a lock
         * @return  True when a lock has been retrieved successfully, false otherwise
         */
        bool has_lock () const {
            return -1 != this->m_lock;
        }

        /**
         * @brief   Retrieve a new lock
         *
         * If this instance already has a lock, the call will block until the lock has been cleared. The lock will
         * then be returned and a new lock will be retrieved.
         *
         * @return  True if the instance was able to successfully retrieve a new lock
         */
        bool refreshLock () {
            if (this->has_lock()) {
                // Wait for any other cogs using the lock to return
                while (lockset(this->m_lock));
                lockclr(this->m_lock);
                lockret(this->m_lock);
            }

            this->m_lock = locknew();
            return this->has_lock();
        }

        /**
         * @brief   Retrieve the printer and acquire the lock. Useful when a class that only supports Printer and not
         *          SynchronousPrinter needs to print
         *
         * @return  Instance of the printer. The invoked instance of SynchronousPrinter will remain locked until
         *          SynchronousPrinter::return_printer() is called
         */
        const Printer *borrow_printer () {
            while (lockset(this->m_lock));
            this->m_borrowed = true;
            return this->m_printer;
        }

        /**
         * @brief   After calling SynchronousPrinter::borrow_printer, this method
         */
        bool return_printer (const Printer *printer) {
            if (printer == this->m_printer) {
                lockclr(this->m_lock);
                this->m_borrowed = false;
                return true;
            } else
                return false;
        }

        /**
         * @see PropWare::Printer::print
         */
        template<typename T>
        void print (const T var) const {
            while (lockset(this->m_lock));
            this->m_printer->print(var);
            lockclr(this->m_lock);
        }

        void println (const char string[]) const {
            while (lockset(this->m_lock));
            this->m_printer->println(string);
            lockclr(this->m_lock);
        }

        /**
         * @see PropWare::Printer::printf(const char fmt[])
         */
        void printf (const char fmt[]) const {
            while (lockset(this->m_lock));
            this->m_printer->puts(fmt);
            lockclr(this->m_lock);
        }

        /**
         * @see PropWare::Printer::printf(const char fmt[], const T first, Targs... remaining)
         */
        template<typename T, typename... Targs>
        void printf (const char fmt[], const T first, const Targs... remaining) const {
            while (lockset(this->m_lock));
            this->m_printer->printf(fmt, first, remaining...);
            lockclr(this->m_lock);
        }

    protected:
        const Printer *m_printer;
        int           m_lock;
        bool          m_borrowed;
};

}

extern const PropWare::SynchronousPrinter pwSyncOut;
