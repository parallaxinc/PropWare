#include "unity.h"
#include "c++-alloc.h"

#include "libpropeller/mcp3208/mcp3208.h"

#include "libpropeller/board/board_unit_tester.h"

MCP3208 * sut;


const int kChannelDac = 3;
const int kChannel5vOver2 = 4;
const int kChannel3v3Over2 = 5;
const int kChannelGround = 6;
const int kChannel3v3 = 7;


// Should be 4096 (for 3.3v)
// On my board, 3.3v is actually 3.27v
// Should be 4058 (for 3.27v)
const int kValue3v3 = 4058;
// Should be 3103 (for 5v)
// On my board, 5v is actually 4.71v
// Should be 2923 (for 4.71v)
const int kValue5vOver2 = 2923;
const int kValue3v3Over2 = kValue3v3 / 2;
const int kValueGround = 0;

const int delta = 25;

const int kMode = 0xFF;

class UnityTests {
public:

    static void setUp(void) {
        sut = new MCP3208();
        sut->Start(Board::kPinMCP3208Data,
                Board::kPinMCP3208Clock,
                Board::kPinMCP3208Select, kMode,
                Board::kPinDac);
        waitcnt(CLKFREQ / 100 + CNT);
    }

    static void tearDown(void) {
        sut->Stop();
        delete sut;
        sut = NULL;
    }

    static void test_3v3(void) {

        TEST_ASSERT_INT_WITHIN(delta, kValue3v3, sut->In(kChannel3v3));
    }

    static void test_Ground(void) {
        // Should be 0
        TEST_ASSERT_INT_WITHIN(delta, kValueGround, sut->In(kChannelGround));
    }

    static void test_3v3Over2(void) {
        TEST_ASSERT_INT_WITHIN(delta, kValue3v3Over2, sut->In(kChannel3v3Over2));
    }

    static void test_5vOver2(void) {
        TEST_ASSERT_INT_WITHIN(delta, kValue5vOver2, sut->In(kChannel5vOver2));
    }

    static void test_Average(void) {
        TEST_ASSERT_INT_WITHIN(delta, kValue3v3Over2, sut->Average(kChannel3v3Over2, 16));

    }

    static void test_DAClow(void) {
        sut->Out(0);
        waitcnt(CLKFREQ / 100 + CNT);
        TEST_ASSERT_INT_WITHIN(delta, kValueGround, sut->In(kChannelDac));
    }

    static void test_DACHigh(void) {
        sut->Out(0xFFFF);
        waitcnt(CLKFREQ / 10 + CNT);
        // For some reason the 3.3v from the Propeller and the 3.3v from the power
        // rail are significantly different. Hence, the need for the explicit constant.
        TEST_ASSERT_INT_WITHIN(delta, 4096, sut->In(kChannelDac));
    }

    static void test_DACMiddle(void) {
        sut->Out(0xFFFF >> 1);
        waitcnt(CLKFREQ / 100 + CNT);
        TEST_ASSERT_INT_WITHIN(delta, kValue3v3Over2, sut->In(kChannelDac));
    }



};

