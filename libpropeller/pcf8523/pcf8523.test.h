#include "unity.h"
#include "c++-alloc.h"
#include "pcf8523.h"

//TODO(SRLM): Get rid of the hardcoded pins.
const int kPIN_I2C_SCL = 0;
const int kPIN_I2C_SDA = 1;

PCF8523 * sut;
I2C * bus;


int startYear, startMonth, startDay, startHour, startMinute, startSecond;

class UnityTests {
public:

    static void setUp(void) {
        bus = new I2C;
        bus->Init(kPIN_I2C_SCL, kPIN_I2C_SDA);
        sut = new PCF8523();
        sut->Init(bus);
    }

    static void tearDown(void) {
        delete bus;
        delete sut;
    }

    static void test_SaveCurrentClock(void) {
        //Saves the current clock so that we don't have to keep putting the time back in.
        //It's ok if it get's off by a second or two.
        sut->GetClock(startYear, startMonth, startDay, startHour, startMinute, startSecond);
        TEST_IGNORE();
    }

    static void test_GetStatusPass(void) {
        TEST_ASSERT_TRUE(sut->GetStatus());
    }

    static void test_ConvertToBCDUnitsOnly(void) {
        TEST_ASSERT_EQUAL_HEX8(0b00000010, sut->ConvertToBCD(2));
    }

    static void test_ConvertToBCDTensOnly(void) {
        TEST_ASSERT_EQUAL_HEX8(0b01010000, sut->ConvertToBCD(50));
    }

    static void test_ConvertToBCDBothUnitsAndTens(void) {
        TEST_ASSERT_EQUAL_HEX8(0b01001001, sut->ConvertToBCD(49));
    }

    static void test_ConvertFromBCDUnitsOnly(void) {
        TEST_ASSERT_EQUAL_INT(2, sut->ConvertFromBCD(0b00000010));
    }

    static void test_ConvertFromBCDTensOnly(void) {
        TEST_ASSERT_EQUAL_INT(50, sut->ConvertFromBCD(0b01010000));
    }

    static void test_ConvertFromBCDUnitsAndTens(void) {
        TEST_ASSERT_EQUAL_INT(49, sut->ConvertFromBCD(0b01001001));
    }

    static void test_SetGetClock(void) {

        int clock[] = {12, 1, 24, 13, 59, 12, 4};
        int result[7];
        TEST_ASSERT_TRUE(sut->SetClock(clock[0], clock[1], clock[2], clock[3], clock[4], clock[5], clock[6]));
        TEST_ASSERT_TRUE(sut->GetClock(result[0], result[1], result[2], result[3], result[4], result[5], result[6]));

        TEST_ASSERT_EQUAL_INT_ARRAY(clock, result, 7);

    }

    static void test_SetGetClockNoWeekday(void) {

        int clock[] = {12, 1, 24, 13, 59, 12};
        int result[7];
        TEST_ASSERT_TRUE(sut->SetClock(clock[0], clock[1], clock[2], clock[3], clock[4], clock[5]));
        TEST_ASSERT_TRUE(sut->GetClock(result[0], result[1], result[2], result[3], result[4], result[5]));

        TEST_ASSERT_EQUAL_INT_ARRAY(clock, result, 6);


    }

    static void test_ClockCountsSeconds(void) {
        int clock[] = {12, 1, 24, 13, 59, 12};
        int result[7];
        TEST_ASSERT_TRUE(sut->SetClock(clock[0], clock[1], clock[2], clock[3], clock[4], clock[5]));

        const int secondsDelay = 2;
        waitcnt(CLKFREQ * secondsDelay + CNT);
        clock[5] += secondsDelay;
        TEST_ASSERT_TRUE(sut->GetClock(result[0], result[1], result[2], result[3], result[4], result[5]));
        TEST_ASSERT_EQUAL_INT_ARRAY(clock, result, 6);


    }

    static void test_RestoreCurrentClock(void) {
        sut->SetClock(startYear, startMonth, startDay, startHour, startMinute, startSecond);
        TEST_IGNORE();
    }

    static void test_GetStatusIsFalseForNoBus(void) {
        PCF8523 dummy;
        TEST_ASSERT_FALSE(dummy.GetStatus());
    }
};


