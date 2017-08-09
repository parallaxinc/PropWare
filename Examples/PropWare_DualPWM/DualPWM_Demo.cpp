/**
 * @file    DualPWM_Demo.cpp
 *
 * @author  David Zemon
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

#include <PropWare/gpio/dualpwm.h>

using PropWare::DualPWM;
using PropWare::Runnable;
using PropWare::Port;

static const unsigned int         PWM_FREQUENCY = 20000;
static const PropWare::Port::Mask MASK_1        = Port::P0;
static const PropWare::Port::Mask MASK_2        = Port::P1;

/**
 * @example     DualPWM_Demo.cpp
 *
 * Drive two PWM waves on GPIO pins with varying duty cycles. Duty will vary from 0% up to 100% in 12.5% increments.
 *
 * @include PropWare_DualPWM/CMakeLists.txt
 */

int main () {
    static uint32_t       stack[48];
    volatile unsigned int leftDuty;
    volatile unsigned int rightDuty;

    const DualPWM pwmController(PWM_FREQUENCY, MASK_1, MASK_2, &leftDuty, &rightDuty, stack);
    Runnable::invoke(pwmController);

    while (1) {
        for (unsigned int i = 0; i <= DualPWM::MAX_DUTY; i += DualPWM::MAX_DUTY / 8) {
            leftDuty  = i;
            rightDuty = DualPWM::MAX_DUTY - i;
            waitcnt(CNT + MILLISECOND);
        }
    }
}
