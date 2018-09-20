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

TEST(CheckEmpty) {
}

TEST(CheckFail_SimpleMessage) {
    FAIL("This test was supposed to fail :)");
}

TEST(CheckFail_ComplexMessage) {
    FAIL("This test was supposed to fail. Sample: '%s'", "Hello, World!");
}

TEST(CheckAssert) {
    ASSERT(true);
}

TEST(CheckAssert_ExpectFailure) {
    ASSERT(false);
}

TEST(CheckAssertTrue) {
    ASSERT_TRUE(true);
}

TEST(CheckAssertTrue_ExpectFailure) {
    ASSERT_TRUE(false);
}

TEST(CheckAssertFalse) {
    ASSERT_FALSE(false);
}

TEST(CheckAssertFalse_ExpectFailure) {
    ASSERT_FALSE(true);
}

TEST(CheckAssertEq) {
    int x        = 3;
    int y        = 4;
    int expected = 7;
    int actual   = x + y;
    ASSERT_EQ(expected, actual);
}

TEST(CheckAssertEq_ExpectFailure) {
    int x        = 3;
    int y        = 42; // Oops! That'd be a typo wouldn't it? :)
    int expected = 7;
    int actual   = x + y;
    ASSERT_EQ(expected, actual);
}

TEST(CheckAssertEqMsg) {
    int x        = 3;
    int y        = 4;
    int expected = 7;
    int actual   = x + y;
    ASSERT_EQ_MSG(expected, actual);
}

TEST(CheckAssertEqMsg_ExpectFailure) {
    int x        = 3;
    int y        = 42; // Oops! That'd be a typo wouldn't it? :)
    int expected = 7;
    int actual   = x + y;
    ASSERT_EQ_MSG(expected, actual);
}

TEST(CheckAssertNeq) {
    int x        = 3;
    int y        = 42; // Oops! That'd be a typo wouldn't it? :)
    int expected = 7;
    int actual   = x + y;
    ASSERT_NEQ(expected, actual);
}

TEST(CheckAssertNeq_ExpectFailure) {
    int x        = 3;
    int y        = 4;
    int expected = 7;
    int actual   = x + y;
    ASSERT_NEQ(expected, actual);
}

TEST(CheckAssertNeqMsg) {
    int x        = 3;
    int y        = 42; // Oops! That'd be a typo wouldn't it? :)
    int expected = 7;
    int actual   = x + y;
    ASSERT_NEQ_MSG(expected, actual);
}

TEST(CheckAssertNeqMsg_ExpectFailure) {
    int x        = 3;
    int y        = 4;
    int expected = 7;
    int actual   = x + y;
    ASSERT_NEQ_MSG(expected, actual);
}

TEST(CheckAssertNull) {
    ASSERT_NULL(NULL);

    int *p = NULL;
    ASSERT_NULL(p);
}

TEST(CheckAssertNull_1_ExpectFailure) {
    ASSERT_NULL(1);
}

TEST(CheckAssertNull_Neg1_ExpectFailure) {
    ASSERT_NULL(-1);
}

TEST(CheckAssertNotNull) {
    ASSERT_NOT_NULL(1);

    int y  = 4;
    int *p = &y;
    ASSERT_NOT_NULL(p);
}

TEST(CheckAssertNotNull_ExpectFailure) {
    ASSERT_NOT_NULL(NULL);
}

TEST(CheckGt) {
    int x         = 3;
    int y         = 4;
    int leftHand  = 8;
    int rightHand = x + y;
    ASSERT_GT(leftHand, rightHand);
}

TEST(CheckGt_ExpectFailure_WhenEqual) {
    int x         = 3;
    int y         = 4;
    int leftHand  = 7;
    int rightHand = x + y;
    ASSERT_GT(leftHand, rightHand);
}

TEST(CheckGt_ExpectFailure_WhenGreater) {
    int x         = 3;
    int y         = 4;
    int leftHand  = 6;
    int rightHand = x + y;
    ASSERT_GT(leftHand, rightHand);
}

TEST(CheckGtMsg) {
    int x         = 3;
    int y         = 4;
    int leftHand  = 8;
    int rightHand = x + y;
    ASSERT_GT_MSG(leftHand, rightHand);
}

TEST(CheckGtMsg_ExpectFailure_WhenEqual) {
    int x         = 3;
    int y         = 4;
    int leftHand  = 7;
    int rightHand = x + y;
    ASSERT_GT_MSG(leftHand, rightHand);
}

TEST(CheckGtMsg_ExpectFailure_WhenGreater) {
    int x         = 3;
    int y         = 4;
    int leftHand  = 6;
    int rightHand = x + y;
    ASSERT_GT_MSG(leftHand, rightHand);
}

TEST(CheckLt) {
    int x         = 3;
    int y         = 4;
    int leftHand  = 6;
    int rightHand = x + y;
    ASSERT_LT(leftHand, rightHand);
}

TEST(CheckLt_ExpectFailure_WhenEqual) {
    int x         = 3;
    int y         = 4;
    int leftHand  = 7;
    int rightHand = x + y;
    ASSERT_LT(leftHand, rightHand);
}

TEST(CheckLt_ExpectFailure_WhenGreater) {
    int x         = 3;
    int y         = 4;
    int leftHand  = 8;
    int rightHand = x + y;
    ASSERT_LT(leftHand, rightHand);
}

TEST(CheckLtMsg) {
    int x         = 3;
    int y         = 4;
    int leftHand  = 6;
    int rightHand = x + y;
    ASSERT_LT_MSG(leftHand, rightHand);
}

TEST(CheckLtMsg_ExpectFailure_WhenEqual) {
    int x         = 3;
    int y         = 4;
    int leftHand  = 7;
    int rightHand = x + y;
    ASSERT_LT_MSG(leftHand, rightHand);
}

TEST(CheckLtMsg_ExpectFailure_WhenGreater) {
    int x         = 3;
    int y         = 4;
    int leftHand  = 8;
    int rightHand = x + y;
    ASSERT_LT_MSG(leftHand, rightHand);
}

TEST(CheckAtMost_IsLess_ShouldPass) {
    int x       = 3;
    int y       = 4;
    int maximum = 8;
    int actual  = x + y;
    ASSERT_AT_MOST(maximum, actual);
}

TEST(CheckAtMost_IsEqual_ShouldPass) {
    int x       = 3;
    int y       = 4;
    int maximum = 7;
    int actual  = x + y;
    ASSERT_AT_MOST(maximum, actual);
}

TEST(CheckAtMost_IsGreater_ShouldFail) {
    int x       = 3;
    int y       = 4;
    int maximum = 6;
    int actual  = x + y;
    ASSERT_AT_MOST(maximum, actual);
}

TEST(CheckAtMostMsg_IsLess_ShouldPass) {
    int x       = 3;
    int y       = 4;
    int maximum = 8;
    int actual  = x + y;
    ASSERT_AT_MOST_MSG(maximum, actual);
}

TEST(CheckAtMostMsg_IsEqual_ShouldPass) {
    int x       = 3;
    int y       = 4;
    int maximum = 7;
    int actual  = x + y;
    ASSERT_AT_MOST_MSG(maximum, actual);
}

TEST(CheckAtMostMsg_IsGreater_ShouldFail) {
    int x       = 3;
    int y       = 4;
    int maximum = 6;
    int actual  = x + y;
    ASSERT_AT_MOST_MSG(maximum, actual);
}

TEST(CheckAtLeast_IsGreater_ShouldPass) {
    int x       = 3;
    int y       = 4;
    int minimum = 6;
    int actual  = x + y;
    ASSERT_AT_LEAST(minimum, actual);
}

TEST(CheckAtLeast_IsEqual_ShouldPass) {
    int x       = 3;
    int y       = 4;
    int minimum = 7;
    int actual  = x + y;
    ASSERT_AT_LEAST(minimum, actual);
}

TEST(CheckAtLeast_IsLess_ShouldFail) {
    int x       = 3;
    int y       = 4;
    int minimum = 8;
    int actual  = x + y;
    ASSERT_AT_LEAST(minimum, actual);
}

TEST(CheckAtLeastMsg_IsGreater_ShouldPass) {
    int x       = 3;
    int y       = 4;
    int minimum = 6;
    int actual  = x + y;
    ASSERT_AT_LEAST_MSG(minimum, actual);
}

TEST(CheckAtLeastMsg_IsEqual_ShouldPass) {
    int x       = 3;
    int y       = 4;
    int minimum = 7;
    int actual  = x + y;
    ASSERT_AT_LEAST_MSG(minimum, actual);
}

TEST(CheckAtLeastMsg_IsLess_ShouldFail) {
    int x       = 3;
    int y       = 4;
    int minimum = 8;
    int actual  = x + y;
    ASSERT_AT_LEAST_MSG(minimum, actual);
}

TEST(PrintUserMessage) {
    MESSAGE("Hello, this is a simple message.");
    MESSAGE("My name is %s!", "David");
}

class SampleFixture {
    public:
        SampleFixture ()
            : m_constructorInvoked(42) {
        }

    public:
        int m_constructorInvoked;
};

TEST_F(SampleFixture, SetupShouldBeInvoked) {
    ASSERT_EQ_MSG(42, this->m_constructorInvoked);
}

TEST_F(SampleFixture, ExpectFailure) {
    ASSERT_TRUE(false);
}

int main () {
    START(SampleTest);

    // Procedural
    RUN_TEST(CheckEmpty);
    EXPECT_FAIL(CheckFail_SimpleMessage);
    EXPECT_FAIL(CheckFail_ComplexMessage);
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
    RUN_TEST(CheckLt);
    EXPECT_FAIL(CheckLt_ExpectFailure_WhenEqual);
    EXPECT_FAIL(CheckLt_ExpectFailure_WhenGreater);
    RUN_TEST(CheckLtMsg);
    EXPECT_FAIL(CheckLtMsg_ExpectFailure_WhenEqual);
    EXPECT_FAIL(CheckLtMsg_ExpectFailure_WhenGreater);
    RUN_TEST(CheckGt);
    EXPECT_FAIL(CheckGt_ExpectFailure_WhenEqual);
    EXPECT_FAIL(CheckGt_ExpectFailure_WhenGreater);
    RUN_TEST(CheckGtMsg);
    EXPECT_FAIL(CheckGtMsg_ExpectFailure_WhenEqual);
    EXPECT_FAIL(CheckGtMsg_ExpectFailure_WhenGreater);
    RUN_TEST(CheckAtMost_IsLess_ShouldPass);
    RUN_TEST(CheckAtMost_IsEqual_ShouldPass);
    EXPECT_FAIL(CheckAtMost_IsGreater_ShouldFail);
    RUN_TEST(CheckAtMostMsg_IsLess_ShouldPass);
    RUN_TEST(CheckAtMostMsg_IsEqual_ShouldPass);
    EXPECT_FAIL(CheckAtMostMsg_IsGreater_ShouldFail);
    RUN_TEST(CheckAtLeast_IsGreater_ShouldPass);
    RUN_TEST(CheckAtLeast_IsEqual_ShouldPass);
    EXPECT_FAIL(CheckAtLeast_IsLess_ShouldFail);
    RUN_TEST(CheckAtLeastMsg_IsGreater_ShouldPass);
    RUN_TEST(CheckAtLeastMsg_IsEqual_ShouldPass);
    EXPECT_FAIL(CheckAtLeastMsg_IsLess_ShouldFail);
    RUN_TEST_F(SampleFixture, SetupShouldBeInvoked);
    EXPECT_FAIL_F(SampleFixture, ExpectFailure);

    COMPLETE();
}
