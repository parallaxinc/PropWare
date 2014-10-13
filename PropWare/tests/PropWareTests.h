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

void _runPropWareUnitTest (bool (*test) (void), const char testName[],
                           const bool expectValue) {
    if (expectValue == test())
        pwOut.printf("SUCCESS: %s" CRLF, testName);
    else
        pwOut.printf("FAILURE: %s" CRLF, testName);
}

#define TEST(testName) bool TEST_ ## testName ()

#define RUN_TEST(testName) _runPropWareUnitTest(TEST_ ## testName, #testName, \
    true)

#define EXPECT_FAIL(testName) _runPropWareUnitTest(TEST_ ## testName, \
    #testName, false)

#define FAIL(message) return false;

#define ASSERT(actual) if (!actual) return false;

#define ASSERT_TRUE(actual) ASSERT(actual)

#define ASSERT_EQ(expected, actual) \
    if (expected != actual) { \
        pwOut.puts("Expected: `"); \
        pwOut.print(expected); \
        pwOut.puts("`; Acutal: `"); \
        pwOut.print(actual); \
        pwOut.puts("`" CRLF); \
        return false; \
    }

#define ASSERT_NEQ(lhs, rhs) \
    if (lhs == rhs) { \
        pwOut.puts("Expected mismatch. Got: `"); \
        pwOut.print(lhs); \
        pwOut.puts("` == `"); \
        pwOut.print(rhs); \
        pwOut.puts("`" CRLF); \
        return false; \
    }