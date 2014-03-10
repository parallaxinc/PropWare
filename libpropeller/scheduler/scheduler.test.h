#include "unity.h"
#include "scheduler.h"


unsigned volatile int unit_CNT;
unsigned volatile int unit_CLKFREQ;


/**
 * Test should work with any positive value of default_hz. Feel free to change
 * it from [1, 99999999]
 */
int default_hz = 99999999;

class UnityTests {
public:

    static void setUp(void) {
        unit_CNT = 0;
        unit_CLKFREQ = 80000000;
    }

    static void tearDown(void) {

    }


    // -----------------------------------------------------------------------------

    static void test_SchedulerReadPeriodIncremented(void) {
        int default_hz = 100;
        Scheduler scheduler(default_hz * 10);

        unit_CNT += 1;

        TEST_ASSERT_FALSE(scheduler.Run());

        //At 6000 cycles and 100hz it should loop unit_CNT around.
        //2^32 / (80000000/100) = 5368
        for (int i = 0; i < 6000; i++) {
            unit_CNT += unit_CLKFREQ / default_hz / 2;
            TEST_ASSERT_FALSE(scheduler.Run());
            unit_CNT += unit_CLKFREQ / default_hz / 2;
            TEST_ASSERT_TRUE(scheduler.Run());
        }

    }


    // -----------------------------------------------------------------------------
    // Test the various relative unit_CNT and nextReadTime conditions
    // -----------------------------------------------------------------------------

    static void test_SchedulerLowBoth(void) {
        unit_CNT = 0x0;
        Scheduler scheduler(default_hz);
        TEST_ASSERT_FALSE(scheduler.Run());
    }

    static void test_SchedulerLowBothOpposite(void) {
        unit_CNT = 0x10;
        Scheduler scheduler(default_hz);
        unit_CNT = 0x7FFFFFFF;

        TEST_ASSERT_TRUE(scheduler.Run());
    }

    static void test_SchedulerHighBoth(void) {
        unit_CNT = 0xF0000000;
        Scheduler scheduler(default_hz);
        TEST_ASSERT_FALSE(scheduler.Run());
    }

    static void test_SchedulerHighBothOpposite(void) {
        unit_CNT = 0x80000000;
        Scheduler scheduler(default_hz);
        unit_CNT = 0xFFFFFFFF;
        TEST_ASSERT_TRUE(scheduler.Run());
    }

    static void test_SchedulerRollover(void) {
        unit_CNT = 0xFFFFFFFF;
        Scheduler scheduler(default_hz);
        TEST_ASSERT_FALSE(scheduler.Run());
    }

    static void test_SchedulerRolloverOpposite(void) {
        unit_CNT = 0xFFFFFFFF;
        Scheduler scheduler(default_hz);
        unit_CNT -= 1;
        TEST_ASSERT_TRUE(scheduler.Run());
    }

    static void test_SchedulerMedian(void) {
        //Test across the 0x80000000 boundary.
        unit_CNT = 0x80000000 - Scheduler::GetTicksPerPeriod(default_hz);
        Scheduler scheduler(default_hz);
        TEST_ASSERT_FALSE(scheduler.Run());
    }

    static void test_SchedulerMedianOpposite(void) {
        unit_CNT = 0x80000000;
        Scheduler scheduler(default_hz);
        unit_CNT = 0x7FFFFFFF;

        TEST_ASSERT_TRUE(scheduler.Run());
    }

    // -----------------------------------------------------------------------------

    static void test_SchedulerNotCalledForMultiplePeriods(void) {
        unit_CNT = 0x0;
        Scheduler scheduler(default_hz);
        TEST_ASSERT_FALSE(scheduler.Run());
        unit_CNT += (scheduler.GetTicksPerPeriod(default_hz) * 4) + 1;
        TEST_ASSERT_TRUE(scheduler.Run());
        TEST_ASSERT_TRUE(scheduler.Run());
        TEST_ASSERT_TRUE(scheduler.Run());
        TEST_ASSERT_TRUE(scheduler.Run());
        TEST_ASSERT_FALSE(scheduler.Run());
    }

};



















