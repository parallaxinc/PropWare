#include "unity.h"
#include "stopwatch.h"

unsigned int unit_CNT;
unsigned int unit_CLKFREQ = 80000000;

class UnityTests{
public:

    static void setUp(void) {

    }

    static void tearDown(void) {

    }

    //TODO(SRLM): Add test for GetStarted();

    // -----------------------------------------------------------------------------

    static void test_short_basic_case(void) {
        const int milliseconds = 10000;
        unit_CNT = 0;
        Stopwatch sw;
        sw.Start();
        unit_CNT = unit_CLKFREQ / 1000 * milliseconds;
        TEST_ASSERT_EQUAL_INT(milliseconds, sw.GetElapsed());
    }

    static void test_long_basic_case(void) {
        const int milliseconds = 50000;
        unit_CNT = 0;
        Stopwatch sw;
        sw.Start();
        unit_CNT = unit_CLKFREQ / 1000 * milliseconds;
        TEST_ASSERT_EQUAL_INT(milliseconds, sw.GetElapsed());
    }

    static void test_short_rollover_case(void) {
        const int milliseconds = 5000;
        unit_CNT = 0xFfffFfff;
        Stopwatch sw;
        sw.Start();
        unit_CNT = unit_CNT + unit_CLKFREQ / 1000 * milliseconds;
        TEST_ASSERT_EQUAL_INT(milliseconds, sw.GetElapsed());
    }

    static void test_long_rollover_case(void) {
        const int milliseconds = 50000;
        unit_CNT = 0xFfffFfff;
        Stopwatch sw;
        sw.Start();
        unit_CNT = unit_CNT + unit_CLKFREQ / 1000 * milliseconds;
        TEST_ASSERT_EQUAL_INT(milliseconds, sw.GetElapsed());
    }

};