/**
 * @file        PropWare/gpio/dualpwm.h
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
#include <PropWare/gpio/pin.h>

namespace PropWare {

/**
 * @brief   Dual-pin PWM driver. Requires a cog to run and will use both counter modules in that cog.
 *
 * The duty cycle should be a value between 0 and 4096, inclusive. The code has been successfully tested up to 80 kHz
 * with the CMM memory model. The maximum frequency should not vary significantly with memory model as the core loop is
 * typically cached via fcache.
 */
class DualPWM: public Runnable {
    public:
        /**
         * Maximum value for the duty cycle. Setting the duty cycle to this value will result in a permanently high
         * logic value on the associated GPIO pin. Valid values for the duty cycle are between zero and this value.
         */
        static const unsigned int MAX_DUTY = 4096;

    public:
        /**
         * @brief       Construct a DualPWM instance
         *
         * @tparam[in]  N           Number of 32-bit words in the stack. This parameter will be auto-determined by
         *                          the compiler
         * @param[in]   frequency       Frequency of the wave, in Hertz
         * @param[in]   mask1       Pin mask of the first pin
         * @param[in]   mask2       Pin mask of the second pin
         * @param[in]   *duty1      Address of the duty cycle variable for the first pin
         * @param[in]   *duty2      Address of the duty cycle variable for the second pin
         * @param[in]   stack[]     Address of the stack. Should be 48 32-bit words. (Example: `uint32_t stack[48];`)
         */
        template<size_t N>
        DualPWM (const unsigned int frequency, const Pin::Mask mask1, const Pin::Mask mask2,
                 volatile unsigned int *duty1, volatile unsigned int *duty2, const uint32_t (&stack)[N])
            : Runnable(stack),
              m_period(CLKFREQ / frequency),
              m_mask1(mask1),
              m_mask2(mask2),
              m_duty1(duty1),
              m_duty2(duty2) {
        }

        /**
         * @brief       Construct a DualPWM instance
         *
         * @param[in]   frequency       Frequency of the wave, in Hertz
         * @param[in]   mask1           Pin mask of the first pin
         * @param[in]   mask2           Pin mask of the second pin
         * @param[in]   *duty1          Address of the duty cycle variable for the first pin
         * @param[in]   *duty2          Address of the duty cycle variable for the second pin
         * @param[in]   *stack          Address of the stack. Should have allocated 48 32-bit words.
         * @param[in]   stackLength     Number of 32-bit words in the stack
         */
        DualPWM (const unsigned int frequency, const Pin::Mask mask1, const Pin::Mask mask2,
                 volatile unsigned int *duty1, volatile unsigned int *duty2, const uint32_t *stack,
                 const size_t stackLength)
            : Runnable(stack, stackLength),
              m_period(CLKFREQ / frequency),
              m_mask1(mask1),
              m_mask2(mask2),
              m_duty1(duty1),
              m_duty2(duty2) {
        }

        void run () {
            CTRA = 0b00100 << 26 | static_cast<unsigned int>(Pin::from_mask(this->m_mask1));
            CTRB = 0b00100 << 26 | static_cast<unsigned int>(Pin::from_mask(this->m_mask2));
            FRQA = 1;
            FRQB = 1;
            DIRA = this->m_mask1 | this->m_mask2;

            const register unsigned int period = this->m_period;

            auto timer = CNT + period;
            while (1) {
                const auto leftPhase  = *this->m_duty1 * period / MAX_DUTY;
                const auto rightPhase = *this->m_duty2 * period / MAX_DUTY;
                __asm__ volatile("neg PHSA, %0" : : "r" (leftPhase));
                __asm__ volatile("neg PHSB, %0" : : "r" (rightPhase));
                timer = waitcnt2(timer, period);
            }
        }

    private:
        const unsigned int    m_period;
        const Pin::Mask       m_mask1;
        const Pin::Mask       m_mask2;
        volatile unsigned int *m_duty1;
        volatile unsigned int *m_duty2;
};

}
