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
 * @brief   IMPORTANT! SynchronousPrinter is not yet working! DO NOT attempt to
 *          use SynchronousPrinter until this note disappears
 */
class SynchronousPrinter {
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
                : m_printCapable(printCapable) {
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

    protected:
        const PrintCapable *m_printCapable;
        int m_lock;
};

}

extern const PropWare::SynchronousPrinter pwSyncOut;
