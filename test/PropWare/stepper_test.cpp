/**
 * @file    stepper_test.cpp
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

#include "PropWareTests.h"
#include <PropWare/motor/stepper.h>
#include <vector>

using PropWare::Stepper;
using PropWare::Pin;

static uint32_t INITIAL_DIRA;
static uint32_t INITIAL_OUTA;

class MockStepper: public Stepper {
    public:
        MockStepper (const Step start = DEFAULT_START_STEP,
                     const Pin::Mask phase1Mask = Pin::NULL_PIN,
                     const Pin::Mask phase2Mask = Pin::NULL_PIN,
                     const Pin::Mask phase3Mask = Pin::NULL_PIN,
                     const Pin::Mask phase4Mask = Pin::NULL_PIN)
            : Stepper(phase1Mask, phase2Mask, phase3Mask, phase4Mask, start) {
        }

        ~MockStepper () {
            this->m_stepsTaken.clear();
        }

        virtual void rotate () {
            this->m_stepsTaken.push_back(this->m_currentStep);
        }

        std::vector<Stepper::Step> m_stepsTaken;
};

Stepper::Step operator+ (const Stepper::Step step, const unsigned int increment) {
    return static_cast<Stepper::Step>((static_cast<uint32_t>(step) + increment) % 8);
}

Stepper::Step operator+ (const unsigned int increment, const Stepper::Step step) {
    return step + increment;
}

Stepper::Step operator- (const Stepper::Step step, const unsigned int decrement) {
    return static_cast<Stepper::Step>((static_cast<uint32_t>(step) + (8 - decrement)) % 8);
}

Stepper::Step operator- (const unsigned int decrement, const Stepper::Step step) {
    return step - decrement;
}

class StepperTest {
    public:
        StepperTest () {
            DIRA = INITIAL_DIRA;
            OUTA = INITIAL_OUTA;
        }

    public:
        MockStepper testable;
};

TEST_F(StepperTest, Constructor_ShouldSetPinsAndStep) {
    const auto      startStep = Stepper::Step::ONE;
    const Pin::Mask pin0      = Pin::P0;
    const Pin::Mask pin1      = Pin::P1;
    const Pin::Mask pin2      = Pin::P2;
    const Pin::Mask pin3      = Pin::P3;

    MockStepper localTestable(startStep, pin0, pin1, pin2, pin3);

    ASSERT_TRUE(DIRA & pin0);
    ASSERT_TRUE(DIRA & pin1);
    ASSERT_TRUE(DIRA & pin2);
    ASSERT_TRUE(DIRA & pin3);

    ASSERT_TRUE(OUTA & pin0);
    ASSERT_FALSE(OUTA & pin1);
    ASSERT_FALSE(OUTA & pin2);
    ASSERT_FALSE(OUTA & pin3);

    ASSERT_EQ(startStep, localTestable.m_currentStep)
}

TEST_F(StepperTest, SetStep) {
    testable.step_to(Stepper::Step::ONE_AND_A_HALF);

    ASSERT_EQ(Stepper::Step::ONE_AND_A_HALF, testable.m_currentStep);
    ASSERT_EQ_MSG(1, testable.m_stepsTaken.size());
    ASSERT_EQ(Stepper::Step::ONE_AND_A_HALF, testable.m_stepsTaken[0]);
}

TEST_F(StepperTest, GetStep) {
    testable.m_currentStep = Stepper::Step::THREE_AND_A_HALF;

    ASSERT_EQ(Stepper::Step::THREE_AND_A_HALF, testable.get_current_step());
    ASSERT_EQ_MSG(0, testable.m_stepsTaken.size());
}

TEST_F(StepperTest, FullPowerHold_noMove) {
    MockStepper localTestable(Stepper::Step::HALF);

    ASSERT_FALSE(localTestable.full_power_hold(true));
    ASSERT_EQ(Stepper::Step::HALF, localTestable.m_currentStep);
    ASSERT_EQ_MSG(0, localTestable.m_stepsTaken.size());
}

TEST_F(StepperTest, FullPowerHold_moveForward) {
    MockStepper localTestable(Stepper::Step::FOUR);

    ASSERT_TRUE(localTestable.full_power_hold(true));
    ASSERT_EQ(Stepper::Step::HALF, localTestable.m_currentStep);
    ASSERT_EQ_MSG(1, localTestable.m_stepsTaken.size());
    ASSERT_EQ(Stepper::Step::HALF, localTestable.m_stepsTaken[0]);
}

TEST_F(StepperTest, FullPowerHold_moveBackward) {
    MockStepper localTestable(Stepper::Step::ONE);

    ASSERT_TRUE(localTestable.full_power_hold(false));
    ASSERT_EQ(Stepper::Step::HALF, localTestable.m_currentStep);
    ASSERT_EQ_MSG(1, localTestable.m_stepsTaken.size());
    ASSERT_EQ(Stepper::Step::HALF, localTestable.m_stepsTaken[0]);
}

TEST_F(StepperTest, HalfPowerHold_noMove) {
    MockStepper localTestable(Stepper::Step::ONE);

    ASSERT_FALSE(localTestable.half_power_hold(true));
    ASSERT_EQ(Stepper::Step::ONE, localTestable.m_currentStep);
    ASSERT_EQ_MSG(0, localTestable.m_stepsTaken.size());
}

TEST_F(StepperTest, HalfPowerHold_moveForward) {
    MockStepper localTestable(Stepper::Step::HALF);

    ASSERT_TRUE(localTestable.half_power_hold(true));
    ASSERT_EQ(Stepper::Step::ONE, localTestable.m_currentStep);
    ASSERT_EQ_MSG(1, localTestable.m_stepsTaken.size());
    ASSERT_EQ(Stepper::Step::ONE, localTestable.m_stepsTaken[0]);
}

TEST_F(StepperTest, HalfPowerHold_moveBackward) {
    MockStepper localTestable(Stepper::Step::ONE_AND_A_HALF);

    ASSERT_TRUE(localTestable.half_power_hold(false));
    ASSERT_EQ(Stepper::Step::ONE, localTestable.m_currentStep);
    ASSERT_EQ_MSG(1, localTestable.m_stepsTaken.size());
    ASSERT_EQ(Stepper::Step::ONE, localTestable.m_stepsTaken[0]);
}

TEST_F(StepperTest, StepForward_ZeroShouldBeAllowed) {
    testable.step_forward(0);

    ASSERT_EQ(0, testable.m_stepsTaken.size());
}

TEST_F(StepperTest, StepForward_OneStep) {
    testable.step_forward(1);

    ASSERT_EQ(1, testable.m_stepsTaken.size());
    ASSERT_EQ(Stepper::DEFAULT_START_STEP + 2, testable.m_stepsTaken[0]);
    ASSERT_EQ(Stepper::DEFAULT_START_STEP + 2, testable.m_currentStep);
}

TEST_F(StepperTest, StepForward_FourSteps) {
    testable.step_forward(4);

    ASSERT_EQ(4, testable.m_stepsTaken.size());
    ASSERT_EQ(Stepper::DEFAULT_START_STEP + 2, testable.m_stepsTaken[0]);
    ASSERT_EQ(Stepper::DEFAULT_START_STEP + 4, testable.m_stepsTaken[1]);
    ASSERT_EQ(Stepper::DEFAULT_START_STEP + 6, testable.m_stepsTaken[2]);
    ASSERT_EQ(Stepper::DEFAULT_START_STEP, testable.m_stepsTaken[3]);
    ASSERT_EQ(Stepper::DEFAULT_START_STEP, testable.m_currentStep);
}

TEST_F(StepperTest, StepReverse_ZeroShouldBeAllowed) {
    testable.step_reverse(0);

    ASSERT_EQ(0, testable.m_stepsTaken.size());
}

TEST_F(StepperTest, StepReverse_OneStep) {
    testable.step_reverse(1);

    ASSERT_EQ(1, testable.m_stepsTaken.size());
    ASSERT_EQ(Stepper::DEFAULT_START_STEP - 2, testable.m_stepsTaken[0]);
    ASSERT_EQ(Stepper::DEFAULT_START_STEP - 2, testable.m_currentStep);
}

TEST_F(StepperTest, StepReverse_FourSteps) {
    testable.step_reverse(4);

    ASSERT_EQ(4, testable.m_stepsTaken.size());
    ASSERT_EQ(Stepper::DEFAULT_START_STEP - 2, testable.m_stepsTaken[0]);
    ASSERT_EQ(Stepper::DEFAULT_START_STEP - 4, testable.m_stepsTaken[1]);
    ASSERT_EQ(Stepper::DEFAULT_START_STEP - 6, testable.m_stepsTaken[2]);
    ASSERT_EQ(Stepper::DEFAULT_START_STEP, testable.m_stepsTaken[3]);
    ASSERT_EQ(Stepper::DEFAULT_START_STEP, testable.m_currentStep);
}

TEST_F(StepperTest, HalfForward_ZeroShouldBeAllowed) {
    testable.half_forward(0);

    ASSERT_EQ(0, testable.m_stepsTaken.size());
}

TEST_F(StepperTest, HalfForward_OneStep) {
    testable.half_forward(1);

    ASSERT_EQ(1, testable.m_stepsTaken.size());
    ASSERT_EQ(Stepper::DEFAULT_START_STEP + 1, testable.m_stepsTaken[0]);
    ASSERT_EQ(Stepper::DEFAULT_START_STEP + 1, testable.m_currentStep);
}

TEST_F(StepperTest, HalfForward_FourSteps) {
    testable.half_forward(4);

    ASSERT_EQ(4, testable.m_stepsTaken.size());
    ASSERT_EQ(Stepper::DEFAULT_START_STEP + 1, testable.m_stepsTaken[0]);
    ASSERT_EQ(Stepper::DEFAULT_START_STEP + 2, testable.m_stepsTaken[1]);
    ASSERT_EQ(Stepper::DEFAULT_START_STEP + 3, testable.m_stepsTaken[2]);
    ASSERT_EQ(Stepper::DEFAULT_START_STEP + 4, testable.m_stepsTaken[3]);
    ASSERT_EQ(Stepper::DEFAULT_START_STEP + 4, testable.m_currentStep);
}

TEST_F(StepperTest, HalfReverse_ZeroShouldBeAllowed) {
    testable.half_reverse(0);

    ASSERT_EQ(0, testable.m_stepsTaken.size());
}

TEST_F(StepperTest, HalfReverse_OneStep) {
    testable.half_reverse(1);

    ASSERT_EQ(1, testable.m_stepsTaken.size());
    ASSERT_EQ(Stepper::DEFAULT_START_STEP - 1, testable.m_stepsTaken[0]);
    ASSERT_EQ(Stepper::DEFAULT_START_STEP - 1, testable.m_currentStep);
}

TEST_F(StepperTest, HalfReverse_FourSteps) {
    testable.half_reverse(4);

    ASSERT_EQ(4, testable.m_stepsTaken.size());
    ASSERT_EQ(Stepper::DEFAULT_START_STEP - 1, testable.m_stepsTaken[0]);
    ASSERT_EQ(Stepper::DEFAULT_START_STEP - 2, testable.m_stepsTaken[1]);
    ASSERT_EQ(Stepper::DEFAULT_START_STEP - 3, testable.m_stepsTaken[2]);
    ASSERT_EQ(Stepper::DEFAULT_START_STEP - 4, testable.m_stepsTaken[3]);
    ASSERT_EQ(Stepper::DEFAULT_START_STEP - 4, testable.m_currentStep);
}

int main () {
    START(PinTest);

    INITIAL_DIRA = DIRA;
    INITIAL_OUTA = OUTA;

    RUN_TEST_F(StepperTest, Constructor_ShouldSetPinsAndStep);
    RUN_TEST_F(StepperTest, SetStep);
    RUN_TEST_F(StepperTest, GetStep);
    RUN_TEST_F(StepperTest, FullPowerHold_noMove);
    RUN_TEST_F(StepperTest, FullPowerHold_moveForward);
    RUN_TEST_F(StepperTest, FullPowerHold_moveBackward);
    RUN_TEST_F(StepperTest, HalfPowerHold_noMove);
    RUN_TEST_F(StepperTest, HalfPowerHold_moveForward);
    RUN_TEST_F(StepperTest, HalfPowerHold_moveBackward);
    RUN_TEST_F(StepperTest, StepForward_ZeroShouldBeAllowed);
    RUN_TEST_F(StepperTest, StepForward_OneStep);
    RUN_TEST_F(StepperTest, StepForward_FourSteps);
    RUN_TEST_F(StepperTest, StepReverse_ZeroShouldBeAllowed);
    RUN_TEST_F(StepperTest, StepReverse_OneStep);
    RUN_TEST_F(StepperTest, StepReverse_FourSteps);
    RUN_TEST_F(StepperTest, HalfForward_ZeroShouldBeAllowed);
    RUN_TEST_F(StepperTest, HalfForward_OneStep);
    RUN_TEST_F(StepperTest, HalfForward_FourSteps);
    RUN_TEST_F(StepperTest, HalfReverse_ZeroShouldBeAllowed);
    RUN_TEST_F(StepperTest, HalfReverse_OneStep);
    RUN_TEST_F(StepperTest, HalfReverse_FourSteps);

    COMPLETE();
}
