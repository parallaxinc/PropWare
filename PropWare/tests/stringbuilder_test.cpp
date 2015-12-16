/**
 * @file    stringbuilder_test.cpp
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

#include "PropWareTests.h"
#include <PropWare/stringbuilder.h>

using namespace PropWare;

static StringBuilder *testable;

void setUp () {
    testable = new StringBuilder();
}

TEARDOWN {
    delete testable;
}

TEST(ConstructorDestructor) {
    testable = new StringBuilder();

    ASSERT_NEQ_MSG(NULL, (unsigned int) testable->m_string);
    ASSERT_EQ_MSG(StringBuilder::DEFAULT_SPACE_ALLOCATED, testable->m_currentSpace);
    ASSERT_EQ_MSG(0, testable->m_size);
    ASSERT_EQ_MSG(0, strlen(testable->to_string()));

    tearDown();
}

TEST(PutChar_one) {
    const char testChar = 'a';

    testable->put_char(testChar);

    ASSERT_EQ_MSG(1, testable->get_size());
    ASSERT_EQ_MSG(strlen(testable->to_string()), testable->get_size());
    ASSERT_EQ_MSG(testChar, testable->to_string()[0]);
    ASSERT_EQ_MSG(StringBuilder::DEFAULT_SPACE_ALLOCATED, testable->m_currentSpace);

    tearDown();
}

TEST(PutChar_two) {
    setUp();

    testable->put_char('a');
    testable->put_char('b');

    ASSERT_EQ_MSG(2, testable->get_size());
    ASSERT_EQ_MSG(strlen(testable->to_string()), testable->get_size());
    ASSERT_EQ_MSG('a', testable->to_string()[0]);
    ASSERT_EQ_MSG('b', testable->to_string()[1]);
    ASSERT_EQ_MSG(StringBuilder::DEFAULT_SPACE_ALLOCATED, testable->m_currentSpace);

    tearDown();
}

TEST(PutChar_three) {
    setUp();

    testable->put_char('a');
    testable->put_char('b');
    testable->put_char('c');

    ASSERT_EQ_MSG(3, testable->get_size());
    ASSERT_EQ_MSG(strlen(testable->to_string()), testable->get_size());
    ASSERT_EQ_MSG('a', testable->to_string()[0]);
    ASSERT_EQ_MSG('b', testable->to_string()[1]);
    ASSERT_EQ_MSG('c', testable->to_string()[2]);
    ASSERT_EQ_MSG(StringBuilder::DEFAULT_SPACE_ALLOCATED, testable->m_currentSpace);

    tearDown();
}

TEST(PutChar_ExactlyFull_SpaceShouldDouble) {
    setUp();

    for (int i = 0; i < StringBuilder::DEFAULT_SPACE_ALLOCATED - 1; ++i)
        testable->put_char('a' + i);

    ASSERT_EQ_MSG(StringBuilder::DEFAULT_SPACE_ALLOCATED - 1, testable->get_size());
    ASSERT_EQ_MSG(strlen(testable->to_string()), testable->get_size());
    ASSERT_EQ_MSG(StringBuilder::DEFAULT_SPACE_ALLOCATED << 1, testable->m_currentSpace);
    for (int i = 0; i < StringBuilder::DEFAULT_SPACE_ALLOCATED - 1; ++i)
        ASSERT_EQ_MSG('a' + i, testable->to_string()[i]);

    tearDown();
}

TEST(PutChar_FirstNewAlloc) {
    setUp();

    const unsigned int originalStringAddr = (unsigned int) testable->to_string();

    for (int i = 0; i < StringBuilder::DEFAULT_SPACE_ALLOCATED; ++i)
        testable->put_char('a' + i);

    ASSERT_NEQ_MSG(originalStringAddr, (unsigned int) testable->to_string());
    ASSERT_EQ_MSG(StringBuilder::DEFAULT_SPACE_ALLOCATED, testable->get_size());
    ASSERT_EQ_MSG(strlen(testable->to_string()), testable->get_size());
    ASSERT_EQ_MSG(StringBuilder::DEFAULT_SPACE_ALLOCATED * 2, testable->m_currentSpace);
    for (int i = 0; i < StringBuilder::DEFAULT_SPACE_ALLOCATED; ++i)
        ASSERT_EQ_MSG('a' + i, testable->to_string()[i]);

    tearDown();
}

TEST(PutChar_HugeString) {
    setUp();

    const unsigned int originalStringAddr = (unsigned int) testable->to_string();

    const int STRING_SIZE = 0x1000 - 1;
    for (int  i           = 0; i < STRING_SIZE; ++i)
        testable->put_char('a');

    ASSERT_NEQ_MSG(originalStringAddr, (unsigned int) testable->to_string());
    ASSERT_EQ_MSG(STRING_SIZE, testable->get_size());
    ASSERT_EQ_MSG(strlen(testable->to_string()), testable->get_size());
    ASSERT_EQ_MSG((STRING_SIZE + 1) << 1, testable->m_currentSpace);
    for (int i = 0; i < STRING_SIZE; ++i)
        ASSERT_EQ_MSG('a', testable->to_string()[i]);

    tearDown();
}

TEST(Clear_empty) {
    setUp();

    testable->clear();

    ASSERT_NEQ_MSG(NULL, (unsigned int) testable->m_string);
    ASSERT_EQ_MSG(StringBuilder::DEFAULT_SPACE_ALLOCATED, testable->m_currentSpace);
    ASSERT_EQ_MSG(0, testable->m_size);
    ASSERT_EQ_MSG(0, strlen(testable->to_string()));

    tearDown();
}

TEST(Clear_OneChar) {
    setUp();

    testable->put_char('a');
    testable->clear();

    ASSERT_NEQ_MSG(NULL, (unsigned int) testable->m_string);
    ASSERT_EQ_MSG(StringBuilder::DEFAULT_SPACE_ALLOCATED, testable->m_currentSpace);
    ASSERT_EQ_MSG(0, testable->m_size);
    ASSERT_EQ_MSG(0, strlen(testable->to_string()));

    tearDown();
}

TEST(Clear_HugeString) {
    setUp();

    const int STRING_SIZE = 0x1000 - 1;
    for (int  i           = 0; i < STRING_SIZE; ++i)
        testable->put_char('a');

    testable->clear();

    ASSERT_NEQ_MSG(NULL, (unsigned int) testable->m_string);
    ASSERT_EQ_MSG(StringBuilder::DEFAULT_SPACE_ALLOCATED, testable->m_currentSpace);
    ASSERT_EQ_MSG(0, testable->m_size);
    ASSERT_EQ_MSG(0, strlen(testable->to_string()));

    tearDown();
}

TEST(Puts) {
    const char testString[] = "Hello, world! My name is David Zemon. This my super long sentence.";
    setUp();

    testable->puts(testString);

    ASSERT_NEQ_MSG(NULL, (unsigned int) testable->to_string());
    ASSERT_EQ_MSG(sizeof(testString) - 1, testable->get_size());
    ASSERT_EQ_MSG(0, strcmp(testString, testable->to_string()));

    tearDown();
}

int main () {
    START(StringBuilderTest);

    RUN_TEST(ConstructorDestructor);
    RUN_TEST(PutChar_one);
    RUN_TEST(PutChar_two);
    RUN_TEST(PutChar_three);
    RUN_TEST(PutChar_ExactlyFull_SpaceShouldDouble);
    RUN_TEST(PutChar_FirstNewAlloc);
    RUN_TEST(PutChar_HugeString);
    RUN_TEST(Clear_empty);
    RUN_TEST(Clear_OneChar);
    RUN_TEST(Clear_HugeString);
    RUN_TEST(Puts);

    COMPLETE();
}
