/**
 * @file    PropWareTests.h
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

#pragma once

#include <PropWare/PropWare.h>
#include <PropWare/printer.h>
#include "c++allocate.h"

void _runPropWareUnitTest (bool (*test) (void), const char testName[],
                           const bool expectValue, bool *result) {
    if (expectValue == test())
        pwOut.printf("#\tSUCCESS: %s" CRLF, testName);
    else {
        pwOut.printf("#\t***FAIL: %s" CRLF, testName);
        *result = false;
    }
}

#define MESSAGE(...) \
    pwOut.printf("#\t- " __VA_ARGS__); \
    pwOut.puts(CRLF)

#define START(testSuiteName) \
    bool passed = true; \
    const char suiteName[] = #testSuiteName; \
    pwOut.printf( \
        "####################" \
        "####################" \
        "####################" \
        "####################" CRLF); \
    pwOut.printf("# Test suite: %s" CRLF, suiteName)

#define COMPLETE() \
    if (!passed) \
        pwOut.printf("# Test FAILURE" CRLF); \
    else \
        pwOut.print("done..." CRLF); \
    return 0

#define TEST(testName) \
    bool TEST_ ## testName ()

#define RUN_TEST(testName) \
    _runPropWareUnitTest(TEST_ ## testName, #testName, true, &passed)

#define EXPECT_FAIL(testName) \
    _runPropWareUnitTest(TEST_ ## testName, #testName, false, &passed)

#define FAIL(message) \
    return false;

#define ASSERT(actual) \
    if (!actual) return false;

#define ASSERT_TRUE(actual) \
    ASSERT(actual)

#define ASSERT_FALSE(actual) \
    ASSERT(false == actual)

#define ASSERT_EQ(expected, actual) \
    if ((expected) != (actual)) \
        return false

#define ASSERT_NEQ(lhs, rhs) \
    if ((lhs) == (rhs)) \
        return false

#define ASSERT_EQ_MSG(expected, actual) \
    if ((expected) != (actual)) { \
        pwOut.puts("#\tExpected: `"); \
        pwOut.print(expected); \
        pwOut.puts("`; Acutal: `"); \
        pwOut.print(actual); \
        pwOut.puts("`" CRLF); \
        return false; \
    }

#define ASSERT_NEQ_MSG(lhs, rhs) \
    if ((lhs) == (rhs)) { \
        pwOut.puts("#\tExpected mismatch. Got: `"); \
        pwOut.print(lhs); \
        pwOut.puts("` == `"); \
        pwOut.print(rhs); \
        pwOut.puts("`" CRLF); \
        return false; \
    }

#define MSG_IF_FAIL(id, assertion, ...)             \
    class _AssertionCheck ## id {                   \
        public:                                     \
            static bool checkAssert () {            \
                assertion;                          \
                return true;                        \
            }                                       \
    };                                              \
                                                    \
    if (!_AssertionCheck ## id::checkAssert()) {    \
        MESSAGE(__VA_ARGS__);                       \
        return false;                               \
    }



#define SETUP \
    void setUp ()

#define TEARDOWN \
    void _tearDown ()

#define tearDown() \
    _tearDown(); \
    return true
