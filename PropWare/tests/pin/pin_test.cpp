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

#include <PropWare/pin.h>
#include "../PropWareTests.h"

PropWare::Pin             *testable;
const uint8_t             TEST_PIN_NUM = 12;
const PropWare::Pin::Mask TEST_MASK    = PropWare::Pin::P12;
const PropWare::Pin::Mask CHECK_MASK   = PropWare::Pin::P13;

SETUP {
    testable = new PropWare::Pin(TEST_MASK, PropWare::Pin::OUT);
}

TEARDOWN {
    delete testable;
}

TEST(Constructor_ShouldSetNullPin) {
    testable = new PropWare::Pin();

    ASSERT_EQ(PropWare::Pin::NULL_PIN, testable->get_mask());

    tearDown();
}

TEST(Constructor_ShouldSetMask) {
    testable = new PropWare::Pin(TEST_MASK);

    ASSERT_EQ(TEST_MASK, testable->get_mask());

    tearDown();
}

TEST(Constructor_ShouldSetMaskAndDir) {
    testable = new PropWare::Pin(TEST_MASK, PropWare::Pin::OUT);

    ASSERT_EQ(TEST_MASK, testable->get_mask());
    ASSERT_EQ(PropWare::Pin::OUT, testable->get_dir());

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

    testable->set_pin_num(TEST_PIN_NUM);
    ASSERT_EQ(TEST_MASK, testable->get_mask());

    tearDown();
}

TEST(SetDir) {
    testable = new PropWare::Pin(TEST_MASK);

    testable->set_dir(PropWare::Pin::OUT);
    ASSERT_EQ(PropWare::Pin::OUT, testable->get_dir());

    tearDown();
}

TEST(Set) {
    setUp();

    testable->set();
    ASSERT_TRUE(OUTA & TEST_MASK);

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
    RUN_TEST(Set);

    COMPLETE();
}
