#include <propeller.h>
#include "unity.h"
#include "full_duplex_serial.h"
#include "c++-alloc.h"
#include "string.h"

#include "board/board_unit_tester.h"

const int rxpin = Board::kPinTie1a;
const int txpin = Board::kPinTie1b;
const int baud = 230400;

const int TIMEOUT = 10;

FullDuplexSerial * sut = NULL;

class UnityTests {
public:

    static void setUp(void) {
        sut = new FullDuplexSerial();
        sut->Start(rxpin, txpin, 0, baud);
    }

    static void tearDown(void) {
        sut->Stop();
        delete sut;
        sut = NULL;
    }

    static void test_BasicRxTx(void) {
        char letter = 'A';
        sut->Put(letter);
        TEST_ASSERT_EQUAL_INT(letter, sut->Get());
    }

    static void test_RxcheckWithNothingInBuffer(void) {
        TEST_ASSERT_EQUAL_INT(-1, sut->Get(TIMEOUT));
    }

    static void test_BasicRxCheckTx(void) {
        char letter = 'C';
        sut->Put('C');
        waitcnt(CLKFREQ / 100 + CNT);
        TEST_ASSERT_EQUAL_INT(letter, sut->Get(TIMEOUT));
    }

    static void test_PutGetMultipleBytes(void) {
        for (char letter = 'a'; letter <= 'z'; letter++) {
            sut->Put(letter);
            waitcnt(CLKFREQ / 100 + CNT);
            TEST_ASSERT_EQUAL_INT(letter, sut->Get());
        }
    }

    static void test_RxflushClearsBuffer(void) {
        char letter1 = 'E';
        char letter2 = 'F';
        sut->Put(letter1);
        waitcnt(CLKFREQ / 100 + CNT);
        sut->GetFlush();
        sut->Put(letter2);
        waitcnt(CLKFREQ / 100 + CNT);

        sut->Get(0);
        TEST_ASSERT_EQUAL_INT(-1, sut->Get(TIMEOUT));
    }

    static void test_PutLongString(void) {
        char string [] = "Hello, World! I'm a Propeller. What are you?";
        int length = strlen(string);
        sut->Put(string);
        waitcnt(CLKFREQ / 100 + CNT);

        for (int i = 0; i < length; i++) {
            TEST_ASSERT_EQUAL_INT(string[i], sut->Get(TIMEOUT));
        }

        TEST_ASSERT_EQUAL_INT(-1, sut->Get(TIMEOUT));

    }

    static void test_PutEmptyString(void) {
        char string [] = "";
        sut->Put(string);
        waitcnt(CLKFREQ / 100 + CNT);
        TEST_ASSERT_EQUAL_INT(-1, sut->Get(TIMEOUT));
    }

};