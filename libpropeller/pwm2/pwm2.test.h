#include "unity.h"
#include "libpropeller/pwm2/pwm2.h"
#include "libpropeller/pulsewidthreader/pulse_width_reader.h"
#include "libpropeller/board/board_unit_tester.h"
#include "c++-alloc.h"

PulseWidthReader * pwr;

PWM2 * sut;

const int kFrequency = 1000;

const int outputPin = Board::kPinTie1a;
const int inputPinMask = 1 << Board::kPinTie1b;

class UnityTests {
public:

    static int MicrosecondsToClockCycles(const int microseconds) {
        return (CLKFREQ * microseconds) / 1000000;
    }

    static int ClockCyclesToMicroseconds(const int clockCycles) {
        return clockCycles / (CLKFREQ / 1000000);
    }

    static void setUp(void) {
        sut = new PWM2();
        sut->Start();
        sut->SetPinX(outputPin);
        sut->SetFrequency(kFrequency);

        pwr = new PulseWidthReader();
        pwr->Start(inputPinMask);

        waitcnt(CLKFREQ / 10 + CNT);

    }

    static void tearDown(void) {
        sut->Stop();
        delete sut;
        sut = NULL;

        pwr->Stop();
        delete pwr;
        pwr = NULL;

    }

    static void helper_CheckWidths(const int highMicroseconds, const int lowMicroseconds) {
        TEST_ASSERT_INT_WITHIN(3, highMicroseconds,
                ClockCyclesToMicroseconds(pwr->getHighTime(0)));

        TEST_ASSERT_INT_WITHIN(3, lowMicroseconds,
                ClockCyclesToMicroseconds(pwr->getLowTime(0)));
    }

    static void helper_TestDuty(const int percent) {



        const int periodUs = 1000000 / kFrequency;
        sut->SetDutyX(percent);
        waitcnt(CLKFREQ / 10 + CNT);

        //while(true){}

        helper_CheckWidths((periodUs * percent) / 100, (periodUs * (100 - percent)) / 100);
    }

    static void test_80PercentDuty(void) {
        helper_TestDuty(80);
    }

    static void test_20PercentDuty(void) {
        helper_TestDuty(20);

    }

    static void test_50PercentDuty(void) {
        helper_TestDuty(50);
    }

};

