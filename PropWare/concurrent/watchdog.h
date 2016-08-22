/**
 * @file        PropWare/concurrent/watchdog.h
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

#include <PropWare/concurrent/runnable.h>
#include <PropWare/PropWare.h>
#include <PropWare/utility/utility.h>

namespace PropWare {

/**
 * @brief   Simple WatchDog object for resetting the chip if it stops responding
 */
class WatchDog : public Runnable {
    public:
        /**
         * @brief   Constructor
         *
         * @param[in]   stack[]             A small stack for a few variables
         * @param[in]   timeout             Length of time (in clock ticks) before the Propeller should be reset
         * @param[in]   monitorFrequency    Length of time to sleep between each check for the timeout (default
         *                                  value of 128us is recommended)
         */
        template<size_t N>
        WatchDog (const uint32_t (&stack)[N], const unsigned int timeout,
                  const unsigned int monitorFrequency = MICROSECOND << 7)
                : Runnable(stack),
                  m_timeout(timeout),
                  m_sleepTime(monitorFrequency) { }

        /**
         * @brief   Reset the timer
         */
        void reset () {
            this->m_timer = CNT;
        }

        void run () {
            this->m_timer = CNT;

            register unsigned int delay = CNT + this->m_sleepTime;
            while (1) {
                waitcnt(delay);
                delay += this->m_sleepTime;
                const unsigned int timeSinceTimerReset = CNT - this->m_timer;
                if (timeSinceTimerReset > this->m_timeout)
                    Utility::reboot(); // Hard reset
            }
        }

    private:
        const unsigned int    m_timeout;
        const unsigned int    m_sleepTime;
        volatile unsigned int m_timer;
};

}
