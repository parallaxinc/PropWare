/**
 * @file    basic_test.cpp
 *
 * @author  David Zemon
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

#include "../PropWareTests.h"

TEST(CheckAssert) {
    ASSERT(true);

    return true;
}

TEST(CheckFail) {
    FAIL("This test was supposed to fail :)");
}

TEST(CheckAssertTrue) {
    ASSERT_TRUE(true);

    return true;
}

TEST(CheckAssertEq) {
    int x = 3;
    int y = 4;
    int expected = 7;
    int actual = x + y;
    ASSERT_EQ(expected, actual);

    return true;
}

TEST(CheckAssertEq_ExpectFailure) {
    int x = 3;
    int y = 42; // Oops! That'd be a typo wouldn't it? :)
    int expected = 7;
    int actual = x + y;
    ASSERT_EQ(expected, actual);

    return true;
}

TEST(CheckAssertNeq) {
    int x = 3;
    int y = 42; // Oops! That'd be a typo wouldn't it? :)
    int expected = 7;
    int actual = x + y;
    ASSERT_NEQ(expected, actual);

    return true;
}

TEST(CheckAssertNeq_ExpectFailure) {
    int x = 3;
    int y = 4;
    int expected = 7;
    int actual = x + y;
    ASSERT_NEQ(expected, actual);

    return true;
}

int main () {
    RUN_TEST(CheckAssert);
    EXPECT_FAIL(CheckFail);
    RUN_TEST(CheckAssertTrue);
    RUN_TEST(CheckAssertEq);
    EXPECT_FAIL(CheckAssertEq_ExpectFailure);
    RUN_TEST(CheckAssertNeq);
    EXPECT_FAIL(CheckAssertNeq_ExpectFailure);

    return 0;
}
