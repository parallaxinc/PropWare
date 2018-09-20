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

#include <PropWare/c++allocate.h>
#include <PropWare/hmi/output/printer.h>

#define PROPWARE_TEST
#define protected public
#define private   public

#define MESSAGE(...) \
    pwOut.printf("#\t- " __VA_ARGS__); \
    pwOut << '\n';

#define START(testSuiteName) \
    uint8_t    failures    = 0; \
    const char suiteName[] = #testSuiteName; \
    pwOut.println( \
        "####################" \
        "####################" \
        "####################" \
        "####################"); \
    pwOut << "# Test suite: " << suiteName << '\n';

#define COMPLETE() \
    if (failures) \
        pwOut << "# Test FAILURES = " << (unsigned int) failures << '\n'; \
    else \
        pwOut.println("done..."); \
    pwOut << (char) 0xff << (char) 0x00 << (char) failures; \
    return failures

/* Procedural tests */

#define TEST(testName) \
    void testName (bool &_pwTest_status)

#define RUN_TEST(testName) \
    _runPropWareUnitTest(testName, #testName, true, &failures)

#define EXPECT_FAIL(testName) \
    _runPropWareUnitTest(testName, #testName, false, &failures)

void _runPropWareUnitTest (void (*test) (bool &), const char testName[], const bool expectValue, uint8_t *failures) {
    bool result = true;
    test(result);
    if (expectValue == result)
        pwOut << "#\tSUCCESS: " << testName << '\n';
    else {
        pwOut << "#\t***FAIL: " << testName << '\n';
        ++(*failures);
    }
}

/* Test fixtures */

#define TEST_F(clazz, testName) \
    class clazz ## testName : public clazz { \
        public: \
            clazz ## testName () : clazz(), _pwTest_status(true) { } \
            void testName (); \
        public: \
            bool _pwTest_status; \
    }; \
    bool clazz ## _ ## testName () { \
        clazz ## testName testable; \
        testable.testName(); \
        return testable._pwTest_status; \
    } \
    void clazz ## testName::testName ()

#define RUN_TEST_F(clazz, testName) \
    _runPropWareUnitTestFixture(clazz ## _ ## testName, #clazz "." #testName, true, &failures)

#define EXPECT_FAIL_F(clazz, testName) \
    _runPropWareUnitTestFixture(clazz ## _ ## testName, #clazz "." #testName, false, &failures)

void _runPropWareUnitTestFixture (bool (*test) (void), const char testName[], const bool expectValue,
                                  uint8_t *failures) {
    if (expectValue == test())
        pwOut << "#\tSUCCESS: " << testName << '\n';
    else {
        pwOut << "#\t***FAIL: " << testName << '\n';
        ++(*failures);
    }
}

/* Assertions */

#define FAIL(...) \
    MESSAGE(__VA_ARGS__) \
    _pwTest_status = false; \
    return; \

#define ASSERT(actual) \
    if (!(actual)) { \
        pwOut << "#\t[" << __FILE__ << ':' << __LINE__ << "]: "; \
        pwOut << "Assertion failed: `ASSERT(" << #actual << ")`\n"; \
        _pwTest_status = false; \
        return; \
    }

#define ASSERT_TRUE(actual) \
    if (true == !(actual)) { \
        pwOut << "#\t[" << __FILE__ << ':' << __LINE__ << "]: "; \
        pwOut << "Expected true, got false: `ASSERT_TRUE(" << #actual << ")`\n"; \
        _pwTest_status = false; \
        return; \
    }

#define ASSERT_FALSE(actual) \
    if (false == !(actual)) { \
        pwOut << "#\t[" << __FILE__ << ':' << __LINE__ << "]: "; \
        pwOut << "Expected false, got true: `ASSERT_FALSE(" << #actual << ")`\n"; \
        _pwTest_status = false; \
        return; \
    }

#define ASSERT_NULL(actual) \
    if (NULL != (actual)) { \
        pwOut << "#\t[" << __FILE__ << ':' << __LINE__ << "]: "; \
        pwOut << "Expected null, got " << (void *) actual << ": `ASSERT_NULL(" << #actual << ")`\n"; \
        _pwTest_status = false; \
        return; \
    }

#define ASSERT_NOT_NULL(actual) \
    if (NULL == (actual)) { \
        pwOut << "#\t[" << __FILE__ << ':' << __LINE__ << "]: "; \
        pwOut << "Expected not-null, got null: `ASSERT_NOT_NULL(" << #actual << ")`\n"; \
        _pwTest_status = false; \
        return; \
    }

#define ASSERT_EQ(expected, actual) \
    if ((expected) != (actual)) { \
        pwOut << "#\t[" << __FILE__ << ':' << __LINE__ << "]: "; \
        pwOut << "Expected values to be equal, but they do not match: "; \
        pwOut << "`ASSERT_EQ(" << #expected << ", " << #actual << ")`\n"; \
        _pwTest_status = false; \
        return; \
    }

#define ASSERT_NEQ(lhs, rhs) \
    if ((lhs) == (rhs)) { \
        pwOut << "#\t[" << __FILE__ << ':' << __LINE__ << "]: "; \
        pwOut << "Expected values to different, but they match: "; \
        pwOut << "`ASSERT_NEQ(" << #lhs << ", " << #rhs << ")`\n"; \
        _pwTest_status = false; \
        return; \
    }

#define ASSERT_GT(greater, lesser) \
    if (!((greater) > (lesser))) { \
        pwOut << "#\t[" << __FILE__ << ':' << __LINE__ << "]: "; \
        pwOut << "Expected left-hand to be greater than right-hand, but was not: "; \
        pwOut << "`ASSERT_GT(" << #greater << ", " << #lesser << ")`\n"; \
        _pwTest_status = false; \
        return; \
    }

#define ASSERT_LT(lesser, greater) \
    if (!((lesser) < (greater))) { \
        pwOut << "#\t[" << __FILE__ << ':' << __LINE__ << "]: "; \
        pwOut << "Expected left-hand to be less than right-hand, but was not: "; \
        pwOut << "`ASSERT_LT(" << #lesser << ", " << #greater << ")`\n"; \
        _pwTest_status = false; \
        return; \
    }

#define ASSERT_AT_MOST(maximum, actual) \
    if (!((maximum) >= (actual))) { \
        pwOut << "#\t[" << __FILE__ << ':' << __LINE__ << "]: "; \
        pwOut << "Expected right-hand to be at most as much as left-hand, but was not: "; \
        pwOut << "`ASSERT_AT_MOST(" << #maximum << ", " << #actual << ")`\n"; \
        _pwTest_status = false; \
        return; \
    }

#define ASSERT_AT_LEAST(minimum, actual) \
    if (!((minimum) <= (actual))) { \
        pwOut << "#\t[" << __FILE__ << ':' << __LINE__ << "]: "; \
        pwOut << "Expected right-hand to be at least as much as left-hand, but was not: "; \
        pwOut << "`ASSERT_AT_LEAST(" << #minimum << ", " << #actual << ")`\n"; \
        _pwTest_status = false; \
        return; \
    }

#define ASSERT_EQ_MSG(expected, actual) \
    if ((expected) != (actual)) { \
        pwOut << "#\t[" << __FILE__ << ':' << __LINE__ << "]\t"; \
        pwOut << "Expected `" << expected << "`, got `" << actual << "`: "; \
        pwOut << "`ASSERT_EQ_MSG(" << #expected << ", " #actual ")`\n"; \
        _pwTest_status = false; \
        return; \
    }

#define ASSERT_NEQ_MSG(lhs, rhs) \
    if ((lhs) == (rhs)) { \
        pwOut << "#\t[" << __FILE__ << ':' << __LINE__ << "]\t"; \
        pwOut << "Expected mismatch, but both sides are `" << lhs << "`:"; \
        pwOut << "`ASSERT_NEQ_MSG(" << #lhs << ", " << #rhs << ")`\n"; \
        _pwTest_status = false; \
        return; \
    }

#define ASSERT_GT_MSG(greater, lesser) \
    if (!((greater) > (lesser))) { \
        pwOut << "#\t[" << __FILE__ << ':' << __LINE__ << "]: "; \
        pwOut << "Expected `" << greater << "` > `" << lesser << "`: "; \
        pwOut << "`ASSERT_GT_MSG(" << #greater << ", " #lesser ")`\n"; \
        _pwTest_status = false; \
        return; \
    }

#define ASSERT_LT_MSG(lesser, greater) \
    if (!((lesser) < (greater))) { \
        pwOut << "#\t[" << __FILE__ << ':' << __LINE__ << "]: "; \
        pwOut << "Expected `" << lesser << "` < `" << greater << "`: "; \
        pwOut << "`ASSERT_LT_MSG(" << #lesser << ", " #greater ")`\n"; \
        _pwTest_status = false; \
        return; \
    }

#define ASSERT_AT_LEAST_MSG(minimum, actual) \
    if (!((minimum) <= (actual))) { \
        pwOut << "#\t[" << __FILE__ << ':' << __LINE__ << "]: "; \
        pwOut << "Expected `" << minimum << "` <= `" << actual << "`: "; \
        pwOut << "`ASSERT_AT_LEAST_MSG(" << #minimum << ", " << #actual << ")`\n"; \
        _pwTest_status = false; \
        return; \
    }

#define ASSERT_AT_MOST_MSG(maximum, actual) \
    if (!((maximum) >= (actual))) { \
        pwOut << "#\t[" << __FILE__ << ':' << __LINE__ << "]: "; \
        pwOut << "Expected `" << maximum << "` >= `" << actual << "`: "; \
        pwOut << "`ASSERT_AT_MOST_MSG(" << #maximum << ", " << #actual << ")`\n"; \
        _pwTest_status = false; \
        return; \
    }
