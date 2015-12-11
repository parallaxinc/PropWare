/**
 * @file    PropWare/motor/stepper.h
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

#pragma once

#include <PropWare/pin.h>

namespace PropWare {

/**
 * @brief   4-phase stepper motor driver
 *
 * This driver always runs in full-torque, full-step mode. An excellent animated GIF of
 * how 4-phase stepper motors work can be found here: http://www.piclist.com/images/www/hobby_elec/gif/step_motor1_2.gif
 *
 * The code for this driver was taken directly from Parallax Javelin code and tweaked to work with PropWare's classes:
 * https://www.parallax.com/sites/default/files/downloads/27964-Stepper-Motor-Documentation.pdf
 */
class Stepper {
    public:
        /**
         * @brief   Default delay in microseconds used between each step
         */
        static const unsigned int DEFAULT_DELAY = 10000;

    private:
        typedef enum {
            ZERO,
            ONE,
            TWO,
            THREE
        }                         Step;

    public:
        /**
         * @brief       Constructor
         *
         * @param[in]   phase1Mask  Pin mask connected to phase 1
         * @param[in]   phase2Mask  Pin mask connected to phase 2
         * @param[in]   phase3Mask  Pin mask connected to phase 3
         * @param[in]   phase4Mask  Pin mask connected to phase 4
         * @param[in]   start       What step should the motor start with
         */
        Stepper (const Pin::Mask phase1Mask, const Pin::Mask phase2Mask, const Pin::Mask phase3Mask,
                 const Pin::Mask phase4Mask, const Step start = ZERO)
                : m_currentStep(start) {
            this->m_phase1.set_mask(phase1Mask);
            this->m_phase2.set_mask(phase2Mask);
            this->m_phase3.set_mask(phase3Mask);
            this->m_phase4.set_mask(phase4Mask);

            this->m_phase1.clear();
            this->m_phase2.clear();
            this->m_phase3.clear();
            this->m_phase4.clear();

            this->m_phase1.set_dir_out();
            this->m_phase2.set_dir_out();
            this->m_phase3.set_dir_out();
            this->m_phase4.set_dir_out();

            this->set_full_step();
        }

        /**
         * @brief       Move the motor forward by a specified number of steps
         *
         * @param[in]   steps       Number of steps to move forward
         * @param[in]   usDelay     Delay in microseconds between steps (must be greater than 17)
         */
        void step_forward (unsigned int steps, const unsigned int usDelay = DEFAULT_DELAY) {
            while (steps--) {
                this->m_currentStep = (Step) ((this->m_currentStep + 1) % 4);
                this->set_full_step();
                waitcnt(usDelay * MICROSECOND);
            }
        }

        /**
         * @brief       Move the motor backward by a specified number of steps
         *
         * @param[in]   steps       Number of steps to move backward
         * @param[in]   usDelay     Delay in microseconds between steps (must be greater than 17)
         */
        void step_reverse (unsigned int steps, const unsigned int usDelay = DEFAULT_DELAY) {
            while (steps--) {
                this->m_currentStep = (Step) ((this->m_currentStep + 3) % 4);
                this->set_full_step();
                waitcnt(usDelay * MILLISECOND);
            }
        }

    private:
        /**
         * @brief   Move the motor to the step specified
         */
        void set_full_step () {
            switch (this->m_currentStep) {
                case ZERO:
                    this->m_phase1.set();
                    this->m_phase2.set();
                    this->m_phase3.clear();
                    this->m_phase4.clear();
                    break;
                case ONE:
                    this->m_phase1.clear();
                    this->m_phase2.set();
                    this->m_phase3.set();
                    this->m_phase4.clear();
                    break;
                case TWO:
                    this->m_phase1.clear();
                    this->m_phase2.clear();
                    this->m_phase3.set();
                    this->m_phase4.set();
                    break;
                case THREE:
                    this->m_phase1.set();
                    this->m_phase2.clear();
                    this->m_phase3.clear();
                    this->m_phase4.set();
                    break;
            }
        }

    private:
        Step m_currentStep;
        Pin  m_phase1;
        Pin  m_phase2;
        Pin  m_phase3;
        Pin  m_phase4;
};

}
