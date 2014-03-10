#include "unity.h"
#include "c++-alloc.h"
#include "libpropeller/i2c/i2c.h"
#include "libpropeller/max17048/max17048.h"

MAX17048 * sut;
I2C * bus;

const int kPIN_I2C_SCL = 0;
const int kPIN_I2C_SDA = 1;

class UnityTests {
public:

    static void setUp(void) {
        bus = new I2C();
        bus->Init(kPIN_I2C_SCL, kPIN_I2C_SDA);
        sut = new MAX17048();
        sut->Init(bus);
    }

    static void tearDown(void) {
        delete bus;
        delete sut;
    }

    static void test_GetStatus(void) {
        TEST_ASSERT_TRUE(sut->GetStatus());
    }

    static void test_GetVersion(void) {
        TEST_ASSERT_EQUAL_HEX32(0x0011, sut->GetVersion());
    }

    static void test_GetVoltage(void) {
        //	TEST_ASSERT_EQUAL_INT_MESSAGE(-1, sut->GetVoltage(),
        TEST_IGNORE_MESSAGE(
                "Note: Must be manually checked. Fully charged should be about 4200.");
    }

    static void test_GetStateOfCharge(void) {
        //	TEST_ASSERT_EQUAL_INT_MESSAGE(-1, sut->GetStateOfCharge(),
        TEST_IGNORE_MESSAGE(
                "Note: Must be manually checked. Fully charged should be about 100.");
    }

    static void test_GetChargeRate(void) {
        //	TEST_ASSERT_EQUAL_INT_MESSAGE(-1, sut->GetChargeRate(),
        TEST_IGNORE_MESSAGE(
                "Note: Must be manually checked. Fully charged should be about 0.");
    }

};




