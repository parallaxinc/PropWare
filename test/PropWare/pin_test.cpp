/**
 * @file    pin_test.cpp
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

#include <PropWare/gpio/pin.h>
#include "PropWareTests.h"

static PropWare::Pin             *testable;
static PropWare::Pin             *helper;
static const uint8_t             TEST_PIN_NUM  = 12;
static const PropWare::Pin::Mask TEST_MASK     = PropWare::Pin::Mask::P12;
static const PropWare::Pin::Mask CHECK_MASK    = PropWare::Pin::Mask::P13;

void setUp (const PropWare::Pin::Dir dir = PropWare::Pin::Dir::OUT) {
    testable = new PropWare::Pin(TEST_MASK, dir);

    if (PropWare::Pin::Dir::OUT == dir)
        helper = new PropWare::Pin(CHECK_MASK, PropWare::Pin::Dir::IN);
    else
        helper = new PropWare::Pin(CHECK_MASK, PropWare::Pin::Dir::OUT);
}

TEARDOWN {
    delete testable;
}

TEST(Constructor_ShouldSetNullPin) {
    testable = new PropWare::Pin();

    ASSERT_EQ(PropWare::Pin::Mask::NULL_PIN, testable->get_mask());

    tearDown();
}

TEST(Constructor_ShouldSetMask) {
    testable = new PropWare::Pin(TEST_MASK);

    ASSERT_EQ(TEST_MASK, testable->get_mask());

    tearDown();
}

TEST(Constructor_ShouldSetMaskAndDir) {
    testable = new PropWare::Pin(TEST_MASK, PropWare::Pin::Dir::OUT);

    ASSERT_EQ(TEST_MASK, testable->get_mask());
    ASSERT_EQ(PropWare::Pin::Dir::OUT, testable->get_dir());

    tearDown();
}

TEST(SetMask) {
    testable = new PropWare::Pin();

    testable->set_mask(TEST_MASK);
    ASSERT_EQ(TEST_MASK, testable->get_mask());

    tearDown();
}

TEST(SetPinNum) {
    testable = new PropWare::Pin();

    testable->set_pin_number(TEST_PIN_NUM);
    ASSERT_EQ(TEST_MASK, testable->get_mask());
    ASSERT_EQ(TEST_PIN_NUM, testable->get_pin_number());

    tearDown();
}

TEST(SetDir) {
    testable = new PropWare::Pin(TEST_MASK);

    testable->set_dir(PropWare::Pin::Dir::OUT);
    ASSERT_EQ(PropWare::Pin::Dir::OUT, testable->get_dir());

    tearDown();
}

TEST(SetDirOut) {
    testable = new PropWare::Pin(TEST_MASK);

    testable->set_dir_out();
    ASSERT_EQ(PropWare::Pin::Dir::OUT, testable->get_dir());

    tearDown();
}

TEST(SetDirIn) {
    testable = new PropWare::Pin(TEST_MASK);

    testable->set_dir_in();
    ASSERT_EQ(PropWare::Pin::Dir::IN, testable->get_dir());

    tearDown();
}

TEST(Set) {
    setUp();

    testable->set();
    ASSERT_EQ(TEST_MASK, OUTA & TEST_MASK);

    tearDown();
}

TEST(High) {
    setUp();

    testable->high();
    ASSERT_EQ(TEST_MASK, OUTA & TEST_MASK);

    tearDown();
}

TEST(On) {
    setUp();

    testable->on();
    ASSERT_EQ(TEST_MASK, OUTA & TEST_MASK);

    tearDown();
}

TEST(Clear) {
    setUp();

    testable->clear();
    ASSERT_EQ(0, OUTA & TEST_MASK);

    tearDown();
}

TEST(Low) {
    setUp();

    testable->low();
    ASSERT_EQ(0, OUTA & TEST_MASK);

    tearDown();
}

TEST(Off) {
    setUp();

    testable->off();
    ASSERT_EQ(0, OUTA & TEST_MASK);

    tearDown();
}

TEST(Toggle) {
    setUp();

    testable->low();
    ASSERT_EQ(0, OUTA & TEST_MASK);
    testable->toggle();
    ASSERT_EQ(TEST_MASK, OUTA & TEST_MASK);
    testable->toggle();
    ASSERT_EQ(0, OUTA & TEST_MASK);

    tearDown();
}

TEST(Write) {
    setUp();

    testable->write(true);
    ASSERT_EQ(TEST_MASK, OUTA & TEST_MASK);
    testable->write(false);
    ASSERT_EQ(0, OUTA & TEST_MASK);
    testable->write(42); // Ensure no problems when an arbitrary value is passed
    ASSERT_EQ(TEST_MASK, OUTA & TEST_MASK);

    tearDown();
}

TEST(Read) {
    setUp(PropWare::Pin::Dir::IN);

    helper->set();
    ASSERT_TRUE(testable->read());
    helper->clear();
    ASSERT_FALSE(testable->read());

    tearDown();
}

int main () {
    START(PinTest);

    RUN_TEST(Constructor_ShouldSetNullPin);
    RUN_TEST(Constructor_ShouldSetMask);
    RUN_TEST(Constructor_ShouldSetMaskAndDir);
    RUN_TEST(SetMask);
    RUN_TEST(SetPinNum);
    RUN_TEST(SetDir);
    RUN_TEST(SetDirOut);
    RUN_TEST(SetDirIn);
    RUN_TEST(Set);
    RUN_TEST(High);
    RUN_TEST(On);
    RUN_TEST(Clear);
    RUN_TEST(Low);
    RUN_TEST(Off);
    RUN_TEST(Toggle);
    RUN_TEST(Write);
    RUN_TEST(Read);

    // TODO: Test wait_until_* and is_switch_* methods as well

    COMPLETE();
}
