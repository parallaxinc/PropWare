// Copyright 2013 SRLM and Red9
#include <propeller.h>
#include "unity.h"
#include "libpropeller/i2c/i2c.h"

const int kSDAPin = 1;
const int kSCLPin = 0;


//static const unsigned char EEPROM = 0b10100000;
static const unsigned char Accl = 0b00110010;
static const unsigned char GYRO = 0b11010110;
static const unsigned char Magn = 0b00111100;
static const unsigned char BARO = 0b11101110;
static const unsigned char Fuel = 0b01101100;

I2C sut;

int registerAddress = 4;
int randomAddress = 0;

class UnityTests {
public:

    static int RandomAddress() {
        registerAddress = registerAddress * 1103515245 + 12345;
        registerAddress = (registerAddress & 0xFFFF0000) & 0x8FFF; //Note: keep this & 0x8FFF so that it's within 32kB address range
        return registerAddress;
    }

    static void setUp(void) {
        randomAddress = RandomAddress();
        sut.Init(kSCLPin, kSDAPin);
    }

    static void tearDown(void) {

    }

    // -----------------------------------------------------------------------------
    // "Normal I2C"
    // -----------------------------------------------------------------------------

    static void test_L3GD20Ping(void) {
        TEST_ASSERT_TRUE(sut.Ping(GYRO));
    }

    static void test_PingNonExistentDevice(void) {
        TEST_ASSERT_FALSE(sut.Ping(0x38));
    }

    static void test_L3GD20ReadWhoAmIRegister(void) {
        TEST_ASSERT_EQUAL_INT(0b11010100, sut.Get(GYRO, 0b00001111));
    }

    static void test_L3GD20WriteCtrlReg1(void) {
        //Test twice so that we *know* that it actually wrote (instead of from a previous test)
        TEST_ASSERT_TRUE(sut.Put(GYRO, 0x20, 0b01111111));
        TEST_ASSERT_EQUAL_HEX8(0b01111111, sut.Get(GYRO, 0x20));

        TEST_ASSERT_TRUE(sut.Put(GYRO, 0x20, 0xFF));
        TEST_ASSERT_EQUAL_HEX8(0xFF, sut.Get(GYRO, 0x20));
    }

    static void test_L3GD20WriteMultipleBytes(void) {
        //Note that the address (SUB) is bitwise OR'd with 0x80 (set the MSb high) in
        //  order to indicate to the slave device to auto increment the address.
        unsigned char data_address = 0x32 | 0x80; //Interupt threshold registers
        char indata1[] = {0x0F, 0xFA, 0x0E, 0x80, 0x01, 0x22};
        char indata2[] = {0x0E, 0xF9, 0x0D, 0x7F, 0x00, 0x21};
        int data_size = 6;
        char outdata[data_size];

        //First Write
        TEST_ASSERT_TRUE(sut.Put(GYRO, data_address, indata1, data_size));
        sut.Get(GYRO, data_address, outdata, data_size);
        TEST_ASSERT_EQUAL_MEMORY(indata1, outdata, data_size);

        //Second Write
        TEST_ASSERT_TRUE(sut.Put(GYRO, data_address, indata2, data_size));
        sut.Get(GYRO, data_address, outdata, data_size);
        TEST_ASSERT_EQUAL_MEMORY(indata2, outdata, data_size);

    }

    static void test_L3GD20WriteMultipleBytesButOnlyOne(void) {
        //Note that the address (SUB) is bitwise OR'd with 0x80 (set the MSb high) in
        //  order to indicate to the slave device to auto increment the address.
        //Test using the multiple byte reads for just one byte
        unsigned char data_address = 0x32 | 0x80;
        char indata1[] = {0x0F};
        char indata2[] = {0x0E};
        int data_size = 1;
        char outdata[data_size];


        //First Write
        TEST_ASSERT_TRUE(sut.Put(GYRO, data_address, indata1, data_size));
        sut.Get(GYRO, data_address, outdata, data_size);
        TEST_ASSERT_EQUAL_MEMORY(indata1, outdata, data_size);

        //Second Write
        TEST_ASSERT_TRUE(sut.Put(GYRO, data_address, indata2, data_size));
        sut.Get(GYRO, data_address, outdata, data_size);
        TEST_ASSERT_EQUAL_MEMORY(indata2, outdata, data_size);
    }

    // -----------------------------------------------------------------------------
    // MS5611
    // -----------------------------------------------------------------------------

    static void test_MS5611PutSingleByteGetMultipleBytes(void) {
        TEST_IGNORE_MESSAGE("Must confirm function with Logic16");
        //Command read memory address 011
        TEST_ASSERT_TRUE(sut.Put(BARO, 0b10100110));

        int data_size = 2;
        char indata1[data_size];
        TEST_ASSERT_TRUE(sut.Get(BARO, indata1, data_size));
    }




};
