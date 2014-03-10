#include <propeller.h>
#include "unity.h"
#include "libpropeller/lsm303dlhc/lsm303dlhc.h"
#include "libpropeller/c++allocate/c++allocate.h"

const int kSDAPin = 1;
const int kSCLPin = 2;


I2C * bus;
LSM303DLHC acclMagn;

class UnityTests {
public:

    static void setUp(void) {
        bus = new I2C();
        bus->Init(kSCLPin, kSDAPin);
        acclMagn.Init(bus);
    }

    static void tearDown(void) {
        delete bus;
    }

    // -----------------------------------------------------------------------------

    static void test_Init(void) {
        TEST_ASSERT_EQUAL_INT(0b00111000, bus->GetPutStack());
        TEST_ASSERT_EQUAL_INT(0b10010111, bus->GetPutStack());
        TEST_ASSERT_EQUAL_INT(0b00000000, bus->GetPutStack());
        TEST_ASSERT_EQUAL_INT(0b00100000, bus->GetPutStack());
        TEST_ASSERT_EQUAL_INT(0b10011100, bus->GetPutStack());
        TEST_ASSERT_EQUAL_INT(-1, bus->GetPutStack());
    }

    static void test_ReadAcclPositiveNumbers(void) {
        //All 16 bits are returned from the LSM303DLHC
        char indata[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
        bus->SetXYZ(indata, 6);
        int x, y, z;
        TEST_ASSERT_TRUE(acclMagn.ReadAccl(x, y, z));

        TEST_ASSERT_EQUAL_HEX32(0x0201, x);
        TEST_ASSERT_EQUAL_HEX32(0x0403, y);
        TEST_ASSERT_EQUAL_HEX32(0x0605, z);
    }

    static void test_ReadAcclNegativeNumbers(void) {
        //All 16 bits are returned from the LSM303DLHC
        char indata[] = {0x01, 0xF2, 0x03, 0xF4, 0x05, 0xF6};
        bus->SetXYZ(indata, 6);
        int x, y, z;
        TEST_ASSERT_TRUE(acclMagn.ReadAccl(x, y, z));

        TEST_ASSERT_EQUAL_HEX32(0xFFFFFF201, x);
        TEST_ASSERT_EQUAL_HEX32(0xFFFFFF403, y);
        TEST_ASSERT_EQUAL_HEX32(0xFFFFFF605, z);
    }

    static void test_ReadMagnPositiveNumbers(void) {
        //Notice how the lower 4 bits are removed: LSM303DLHC returns 12 bit numbers!
        char indata[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
        bus->SetXYZ(indata, 6);
        int x, y, z;
        TEST_ASSERT_TRUE(acclMagn.ReadMagn(x, y, z));

        TEST_ASSERT_EQUAL_HEX32(0x0102, x);
        TEST_ASSERT_EQUAL_HEX32(0x0304, z);
        TEST_ASSERT_EQUAL_HEX32(0x0506, y);
    }

    static void test_ReadMagnNegativeNumbers(void) {
        //Notice how the lower 4 bits are removed: LSM303DLHC returns 12 bit numbers!
        char indata[] = {0xF1, 0xA2, 0xF3, 0xA4, 0xF5, 0xA6};
        bus->SetXYZ(indata, 6);
        int x, y, z;
        TEST_ASSERT_TRUE(acclMagn.ReadMagn(x, y, z));

        TEST_ASSERT_EQUAL_HEX32(0xFFFFFF1A2, x);
        TEST_ASSERT_EQUAL_HEX32(0xFFFFFF3A4, z);
        TEST_ASSERT_EQUAL_HEX32(0xFFFFFF5A6, y);
    }

    //TODO(SRLM): Add tests for the ReadAccl/ReadMagn bus->Get == bus->kNak case
    //Add tests for status being bus->kNak

};
