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

#include <PropWare/gpio/pin.h>

namespace PropWare {

/**
 * @brief   4-pin bipolar and 5-pin unipolar stepper motor driver
 */
class Stepper {
    public:
        enum class Step {
                HALF,
                ONE,
                ONE_AND_A_HALF,
                TWO,
                TWO_AND_A_HALF,
                THREE,
                THREE_AND_A_HALF,
                FOUR
        };

        /**
         * @brief   Default delay in microseconds used between each step
         */
        static const unsigned int DEFAULT_DELAY      = 10000;
        static const Step         DEFAULT_START_STEP = Step::HALF;

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
                 const Pin::Mask phase4Mask, const Step start = DEFAULT_START_STEP)
            : m_currentStep(start),
              m_phase1(phase1Mask, Pin::Dir::OUT),
              m_phase2(phase2Mask, Pin::Dir::OUT),
              m_phase3(phase3Mask, Pin::Dir::OUT),
              m_phase4(phase4Mask, Pin::Dir::OUT) {
            this->rotate();
        }

        /**
         * @brief           Move the motor directly to the specified step
         *
         * @param step[in]  Step to move to
         */
        void step_to (const Stepper::Step step) {
            this->m_currentStep = step;
            this->rotate();
        }

        /**
         * @brief       Get the current step
         * @return      Current step where the motor is holding
         */
        Step get_current_step () const {
            return this->m_currentStep;
        }

        /**
         * @brief                   Step the motor to the nearest half-step to achieve a full-power hold
         *
         * @param direction[in]     True for forward, false for backward
         * @param usDelay           Delay in microseconds after the step is taken (must be greater than 17 for lmm)
         *
         * @return                  True if the motor requirement movement to achieve full-power hold, false if the
         *                          motor was already on a full-power hold
         */
        bool full_power_hold (const bool direction, const unsigned int usDelay = DEFAULT_DELAY) {
            const bool movementNeeded = static_cast<bool>(static_cast<uint32_t>(this->m_currentStep) % 2);
            if (movementNeeded) {
                if (direction)
                    this->half_forward(1, usDelay);
                else
                    this->half_reverse(1, usDelay);
            }
            return movementNeeded;
        }

        /**
         * @brief                   Step the motor to the nearest full-step to achieve a half-power hold
         *
         * @param direction[in]     True for forward, false for backward
         * @param usDelay           Delay in microseconds after the step is taken (must be greater than 17 for lmm)
         *
         * @return                  True if the motor requirement movement to achieve half-power hold, false if the
         *                          motor was already on a half-power hold
         */
        bool half_power_hold (const bool direction, const unsigned int usDelay = DEFAULT_DELAY) {
            const bool movementNeeded = !static_cast<bool>(static_cast<uint32_t>(this->m_currentStep) % 2);
            if (movementNeeded) {
                if (direction)
                    this->half_forward(1, usDelay);
                else
                    this->half_reverse(1, usDelay);
            }
            return movementNeeded;
        }

        /**
         * @brief       Move the motor forward by a specified number of steps
         *
         * @param[in]   steps       Number of steps to move forward
         * @param[in]   usDelay     Delay in microseconds between steps (must be greater than 17 for lmm)
         */
        void step_forward (unsigned int steps, const unsigned int usDelay = DEFAULT_DELAY) {
            while (steps--) {
                const unsigned int stepNumber = static_cast<unsigned int>(this->m_currentStep) + 2;
                this->m_currentStep = static_cast<Step>(stepNumber % 8);
                this->rotate();
                waitcnt(usDelay * MICROSECOND + CNT);
            }
        }

        /**
         * @brief       Move the motor forward the a specified number of half-steps
         *
         * @param[in]   halfSteps   Number of half-steps to move forward
         * @param[in]   usDelay     Delay in microseconds between half-steps (must be greater than 17 for lmm)
         */
        void half_forward (unsigned int halfSteps, const unsigned int usDelay = DEFAULT_DELAY) {
            while (halfSteps--) {
                const unsigned int stepNumber = static_cast<unsigned int>(this->m_currentStep) + 1;
                this->m_currentStep = static_cast<Step>(stepNumber % 8);
                this->rotate();
                waitcnt(usDelay * MICROSECOND + CNT);
            }
        }

        /**
         * @brief       Move the motor backward by a specified number of steps
         *
         * @param[in]   steps       Number of steps to move backward
         * @param[in]   usDelay     Delay in microseconds between steps (must be greater than 17 for lmm)
         */
        void step_reverse (unsigned int steps, const unsigned int usDelay = DEFAULT_DELAY) {
            while (steps--) {
                const unsigned int stepNumber = static_cast<unsigned int>(this->m_currentStep) + 6;
                this->m_currentStep = static_cast<Step>(stepNumber % 8);
                this->rotate();
                waitcnt(usDelay * MICROSECOND + CNT);
            }
        }

        /**
         * @brief       Move the motor backward by a specified number of half-steps
         *
         * @param[in]   halfSteps   Number of half-steps to move backward
         * @param[in]   usDelay     Delay in microseconds between steps (must be greater than 17 for lmm)
         */
        void half_reverse (unsigned int halfSteps, const unsigned int usDelay = DEFAULT_DELAY) {
            while (halfSteps--) {
                const unsigned int stepNumber = static_cast<unsigned int>(this->m_currentStep) + 7;
                this->m_currentStep = static_cast<Step>(stepNumber % 8);
                this->rotate();
                waitcnt(usDelay * MICROSECOND + CNT);
            }
        }

    private:
        /**
         * @brief   Rotate the motor to the appropriate position for the current step
         */
        virtual void rotate () {
            switch (this->m_currentStep) {
                case Step::HALF:
                    this->m_phase1.set();
                    this->m_phase2.clear();
                    this->m_phase3.clear();
                    this->m_phase4.set();
                    break;
                case Step::ONE:
                    this->m_phase1.set();
                    this->m_phase2.clear();
                    this->m_phase3.clear();
                    this->m_phase4.clear();
                    break;
                case Step::ONE_AND_A_HALF:
                    this->m_phase1.set();
                    this->m_phase2.set();
                    this->m_phase3.clear();
                    this->m_phase4.clear();
                    break;
                case Step::TWO:
                    this->m_phase1.clear();
                    this->m_phase2.set();
                    this->m_phase3.clear();
                    this->m_phase4.clear();
                    break;
                case Step::TWO_AND_A_HALF:
                    this->m_phase1.clear();
                    this->m_phase2.set();
                    this->m_phase3.set();
                    this->m_phase4.clear();
                    break;
                case Step::THREE:
                    this->m_phase1.clear();
                    this->m_phase2.clear();
                    this->m_phase3.set();
                    this->m_phase4.clear();
                    break;
                case Step::THREE_AND_A_HALF:
                    this->m_phase1.clear();
                    this->m_phase2.clear();
                    this->m_phase3.set();
                    this->m_phase4.set();
                    break;
                case Step::FOUR:
                    this->m_phase1.clear();
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
