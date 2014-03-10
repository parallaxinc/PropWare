// Copyright 2013 SRLM and Red9
#include <propeller.h>
#include "unity.h"
#include "c++-alloc.h"
#include "libpropeller/l3gd20/l3gd20.h"
#include "libpropeller/i2c/i2cMOCK.h"


//TODO(SRLM): Get rid of the hardcoded pins.
const int kSDAPin = 1;
const int kSCLPin = 2;


I2C * bus;
L3GD20 gyro;

class UnityTests {
public:

    static void setUp(void) {
        bus = new I2C();
        bus->Init(kSCLPin, kSDAPin);
        gyro.Init(bus, L3GD20::LSB_1);
    }

    static void tearDown(void) {
        delete bus;
        bus = NULL;
    }

    // -----------------------------------------------------------------------------

    static void test_Init(void) {
        TEST_ASSERT_EQUAL_INT(0b00110000, bus->GetPutStack());
        TEST_ASSERT_EQUAL_INT(0b11111111, bus->GetPutStack());
        TEST_ASSERT_EQUAL_INT(-1, bus->GetPutStack());
    }

    static void test_ReadGyroPositiveNumbers(void) {
        char indata[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
        bus->SetXYZ(indata, 6);
        int x, y, z;
        TEST_ASSERT_TRUE(gyro.ReadGyro(x, y, z));

        TEST_ASSERT_EQUAL_HEX32(0x0201, x);
        TEST_ASSERT_EQUAL_HEX32(0x0403, y);
        TEST_ASSERT_EQUAL_HEX32(0x0605, z);
    }

    static void test_ReadGyroNegativeNumbers(void) {
        char indata[] = {0x01, 0xF2, 0x03, 0xF4, 0x05, 0xF6};
        bus->SetXYZ(indata, 6);
        int x, y, z;
        TEST_ASSERT_TRUE(gyro.ReadGyro(x, y, z));

        TEST_ASSERT_EQUAL_HEX32(0xFFFFF201, x);
        TEST_ASSERT_EQUAL_HEX32(0xFFFFF403, y);
        TEST_ASSERT_EQUAL_HEX32(0xFFFFF605, z);
    }

};
