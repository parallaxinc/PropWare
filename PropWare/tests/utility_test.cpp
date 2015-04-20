/**
 * @file    utility_test.cpp
 *
 * @author  David Zemon
 *
 * Hardware:
 *      Connect pins P12 and P13 together
 *
 * @copyright
 * The MIT License (MIT)<br>
 * <br>Copyright (c) 2013 David Zemon<br>
 * <br>Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:<br>
 * <br>The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.<br>
 * <br>THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <math.h>
#include "PropWareTests.h"

using namespace PropWare;

TEARDOWN {
}

TEST(CountBits) {
    ASSERT_EQ_MSG(0, Utility::count_bits(0));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_0));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_1));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_2));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_3));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_4));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_5));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_6));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_7));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_8));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_9));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_10));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_11));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_12));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_13));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_14));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_15));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_16));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_17));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_18));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_19));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_20));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_21));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_22));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_23));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_24));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_25));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_26));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_27));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_28));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_29));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_30));
    ASSERT_EQ_MSG(1, Utility::count_bits(BIT_31));

    ASSERT_EQ_MSG(4, Utility::count_bits(NIBBLE_0));
    ASSERT_EQ_MSG(4, Utility::count_bits(NIBBLE_1));
    ASSERT_EQ_MSG(4, Utility::count_bits(NIBBLE_2));
    ASSERT_EQ_MSG(4, Utility::count_bits(NIBBLE_3));
    ASSERT_EQ_MSG(4, Utility::count_bits(NIBBLE_4));
    ASSERT_EQ_MSG(4, Utility::count_bits(NIBBLE_5));
    ASSERT_EQ_MSG(4, Utility::count_bits(NIBBLE_6));
    ASSERT_EQ_MSG(4, Utility::count_bits(NIBBLE_7));

    ASSERT_EQ_MSG(8, Utility::count_bits(BYTE_0));
    ASSERT_EQ_MSG(8, Utility::count_bits(BYTE_1));
    ASSERT_EQ_MSG(8, Utility::count_bits(BYTE_2));
    ASSERT_EQ_MSG(8, Utility::count_bits(BYTE_3));

    ASSERT_EQ_MSG(16, Utility::count_bits(WORD_0));
    ASSERT_EQ_MSG(16, Utility::count_bits(WORD_1));

    ASSERT_EQ_MSG(5, Utility::count_bits(0x1234))

    tearDown();
}

TEST(ToLower) {
    char actual[] = "HelWorl da12 dad%";
    char expected[] = "helworl da12 dad%";

    Utility::to_lower(actual);
    ASSERT_EQ_MSG(0, strcmp(expected, actual));

    tearDown();
}

TEST(ToLower_DoesNotFailOnEmpty) {
    char actual[] = "";

    Utility::to_lower(actual);
    ASSERT_EQ_MSG(0, strlen(actual));

    tearDown();
}

TEST(ToUpper) {
    char actual[] = "HelWorl da12 dad%";
    char expected[] = "HELWORL DA12 DAD%";

    Utility::to_upper(actual);
    ASSERT_EQ_MSG(0, strcmp(expected, actual));

    tearDown();
}

TEST(ToUpper_DoesNotFailOnEmpty) {
    char actual[] = "";

    Utility::to_upper(actual);
    ASSERT_EQ_MSG(0, strlen(actual));

    tearDown();
}

TEST(ToString) {
    ASSERT_EQ_MSG(0, strcmp("true", Utility::to_string(true)));
    ASSERT_EQ_MSG(0, strcmp("false", Utility::to_string(false)));

    tearDown();
}

TEST(RomLog) {
    ASSERT_EQ_MSG(0, (Utility::rom_log(BIT_0) >> 16));
    ASSERT_EQ_MSG(1, (Utility::rom_log(BIT_1) >> 16));
    ASSERT_EQ_MSG(2, (Utility::rom_log(BIT_2) >> 16));
    ASSERT_EQ_MSG(3, (Utility::rom_log(BIT_3) >> 16));
    ASSERT_EQ_MSG(4, (Utility::rom_log(BIT_4) >> 16));
    ASSERT_EQ_MSG(5, (Utility::rom_log(BIT_5) >> 16));
    ASSERT_EQ_MSG(6, (Utility::rom_log(BIT_6) >> 16));
    ASSERT_EQ_MSG(7, (Utility::rom_log(BIT_7) >> 16));
    ASSERT_EQ_MSG(8, (Utility::rom_log(BIT_8) >> 16));
    ASSERT_EQ_MSG(9, (Utility::rom_log(BIT_9) >> 16));

    // Check that our fixed-point log table is close to the math library
    const double expected    = log2(2356);
    const double actual      = (double) Utility::rom_log(2356) / BIT_16;
    const double difference  = fabs(actual - expected) / expected;
    const bool   withinRange = 0.0001 > difference;
    ASSERT_TRUE(withinRange);

    tearDown();
}

TEST(MeasureTimeInterval) {
    const unsigned int expected = 1000;

    unsigned int start = CNT;
    waitcnt(expected * MILLISECOND + CNT);
    unsigned int actual = Utility::measure_time_interval(start) / 1000;
    ASSERT_EQ_MSG(expected, actual);

    tearDown();
}

int main () {
    START(UtilityTest);

    RUN_TEST(CountBits);
    RUN_TEST(ToLower);
    RUN_TEST(ToLower_DoesNotFailOnEmpty);
    RUN_TEST(ToUpper);
    RUN_TEST(ToUpper_DoesNotFailOnEmpty);
    RUN_TEST(ToString);
    RUN_TEST(RomLog);
    RUN_TEST(MeasureTimeInterval);

    COMPLETE();
}
