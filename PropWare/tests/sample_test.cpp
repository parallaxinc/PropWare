/**
 * @file    sample_test.cpp
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

#include "PropWareTests.h"

TEARDOWN {
}

TEST(CheckEmpty) {
    return true;
}

TEST(CheckFail) {
    FAIL("This test was supposed to fail :)");
}

TEST(CheckAssert) {
    ASSERT(true);

    return true;
}

TEST(CheckAssert_ExpectFailure) {
    ASSERT(false);

    return true;
}

TEST(CheckAssertTrue) {
    ASSERT_TRUE(true);

    return true;
}

TEST(CheckAssertTrue_ExpectFailure) {
    ASSERT_TRUE(false);

    return true;
}

TEST(CheckAssertFalse) {
    ASSERT_FALSE(false);

    return true;
}

TEST(CheckAssertFalse_ExpectFailure) {
    ASSERT_FALSE(true);

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

TEST(CheckAssertEqMsg) {
    int x = 3;
    int y = 4;
    int expected = 7;
    int actual = x + y;
    ASSERT_EQ_MSG(expected, actual);

    return true;
}

TEST(CheckAssertEqMsg_ExpectFailure) {
    int x = 3;
    int y = 42; // Oops! That'd be a typo wouldn't it? :)
    int expected = 7;
    int actual = x + y;
    ASSERT_EQ_MSG(expected, actual);

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

TEST(CheckAssertNeqMsg) {
    int x = 3;
    int y = 42; // Oops! That'd be a typo wouldn't it? :)
    int expected = 7;
    int actual = x + y;
    ASSERT_NEQ_MSG(expected, actual);

    return true;
}

TEST(CheckAssertNeqMsg_ExpectFailure) {
    int x = 3;
    int y = 4;
    int expected = 7;
    int actual = x + y;
    ASSERT_NEQ_MSG(expected, actual);

    return true;
}

TEST(CheckAssertNull) {
    ASSERT_NULL(NULL);

    int *p = NULL;
    ASSERT_NULL(p);

    return true;
}

TEST(CheckAssertNull_1_ExpectFailure) {
    ASSERT_NULL(1);

    return true;
}

TEST(CheckAssertNull_Neg1_ExpectFailure) {
    ASSERT_NULL(-1);

    return true;
}

TEST(CheckAssertNotNull) {
    ASSERT_NOT_NULL(1);

    int y = 4;
    int *p = &y;
    ASSERT_NOT_NULL(p);

    return true;
}

TEST(CheckAssertNotNull_ExpectFailure) {
    ASSERT_NOT_NULL(NULL);

    return true;
}

TEST(PrintUserMessage) {
    MESSAGE("Hello, this is a simple message.");
    MESSAGE("My name is %s!", "David");

    return true;
}

TEST(MsgIfFail) {
    MSG_IF_FAIL(1, ASSERT(true), "FAIL!!! You should not see this message!");

    tearDown();
}

TEST(MsgIfFail_ExpectFailure) {
    MSG_IF_FAIL(2, ASSERT(false),
                "Sample message upon failing this assertion (expected).");

    tearDown();
}

TEST(MsgIfFail_WithArgs_ExpectFailure) {
    MSG_IF_FAIL(2, ASSERT(false),
                "Sample message upon failing this assertion %s.", "(expected)");

    tearDown();
}

int main () {
    START(SampleTest);

    RUN_TEST(CheckEmpty);
    EXPECT_FAIL(CheckFail);
    RUN_TEST(CheckAssert);
    EXPECT_FAIL(CheckAssert_ExpectFailure);
    RUN_TEST(CheckAssertTrue);
    EXPECT_FAIL(CheckAssertTrue_ExpectFailure);
    RUN_TEST(CheckAssertFalse);
    EXPECT_FAIL(CheckAssertFalse_ExpectFailure);
    RUN_TEST(CheckAssertEq);
    EXPECT_FAIL(CheckAssertEq_ExpectFailure);
    RUN_TEST(CheckAssertEqMsg);
    EXPECT_FAIL(CheckAssertEqMsg_ExpectFailure);
    RUN_TEST(CheckAssertNeq);
    EXPECT_FAIL(CheckAssertNeq_ExpectFailure);
    RUN_TEST(CheckAssertNeqMsg);
    EXPECT_FAIL(CheckAssertNeqMsg_ExpectFailure);
    RUN_TEST(CheckAssertNull);
    EXPECT_FAIL(CheckAssertNull_1_ExpectFailure);
    EXPECT_FAIL(CheckAssertNull_Neg1_ExpectFailure);
    RUN_TEST(CheckAssertNotNull);
    EXPECT_FAIL(CheckAssertNotNull_ExpectFailure);
    RUN_TEST(PrintUserMessage);
    RUN_TEST(MsgIfFail);
    EXPECT_FAIL(MsgIfFail_ExpectFailure);
    EXPECT_FAIL(MsgIfFail_WithArgs_ExpectFailure);

    COMPLETE();
}
