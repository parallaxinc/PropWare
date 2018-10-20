/**
 * @file    scanner_test.cpp
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
#include <PropWare/hmi/input/scanner.h>
#include <PropWare/utility/collection/charqueue.h>

using PropWare::Scanner;
using PropWare::CharQueue;

class ScannerTest {
    public:
        static const size_t BUFFER_SIZE = 256;

    public:
        ScannerTest ()
            : queue(rawBuffer),
              testable(queue) {
        }

    protected:
        char      rawBuffer[BUFFER_SIZE];
        CharQueue queue;
        Scanner   testable;
};

TEST_F(ScannerTest, get_char) {
    const char expected1 = 'x';
    const char expected2 = 'a';
    queue.put_char(expected1);
    queue.put_char(expected2);

    ASSERT_EQ_MSG(expected1, testable.get_char());
    ASSERT_EQ_MSG(expected2, testable.get_char());
}

TEST_F(ScannerTest, gets_default_delimeter) {
    const char expected1[] = "Hello, world!";
    queue.puts(expected1);
    queue.put_char(Scanner::DEFAULT_DELIMITER);

    const char expected2[] = "Goodbye,\tworld!";
    queue.puts(expected2);
    queue.put_char('\r');
    queue.put_char(Scanner::DEFAULT_DELIMITER);

    char actual[32];
    testable.gets(actual, 32);
    ASSERT_EQ_MSG(0, strcmp(actual, expected1));
    testable.gets(actual, 32);
    ASSERT_EQ_MSG(0, strcmp(actual, expected2));
}

TEST_F(ScannerTest, gets_non_default_delimeter) {
    const char delimiter = ',';

    const char expected1[] = "Hello";
    queue.puts(expected1);
    queue.put_char(delimiter);

    const char expected2[] = "cruel \r\nworld\r";
    queue.puts(expected2);
    queue.put_char(delimiter);

    char actual[32];
    testable.gets(actual, 32, delimiter);
    ASSERT_EQ_MSG(0, strcmp(actual, expected1));
    testable.gets(actual, 32, delimiter);
    ASSERT_EQ_MSG(0, strcmp(actual, expected2));
}

TEST_F(ScannerTest, get_int_newlineSeperated) {
    queue.puts("42\n");
    int actual;
    ASSERT_EQ_MSG(Scanner::NO_ERROR, testable.get(actual));
    ASSERT_EQ_MSG(42, actual);
}

TEST_F(ScannerTest, get_int_spaceSeperated) {
    queue.puts("43 ");
    int actual;
    ASSERT_EQ_MSG(Scanner::NO_ERROR, testable.get(actual));
    ASSERT_EQ_MSG(43, actual);
}

TEST_F(ScannerTest, get_int_tabSeperated) {
    queue.puts("44\t");
    int actual;
    ASSERT_EQ_MSG(Scanner::NO_ERROR, testable.get(actual));
    ASSERT_EQ_MSG(44, actual);
}

TEST_F(ScannerTest, streamOperator_multipleTokens) {
    queue.puts("1 2 3  \t 44\n");

    int a = 0;
    int b = 0;
    int c = 0;
    int d = 0;
    testable >> a >> b >> c >> d;
    ASSERT_EQ_MSG(1, a);
    ASSERT_EQ_MSG(2, b);
    ASSERT_EQ_MSG(3, c);
    ASSERT_EQ_MSG(44, d);
}

TEST_F(ScannerTest, get_float) {
    queue.puts("1.5\n");
    float actual;
    ASSERT_EQ_MSG(Scanner::NO_ERROR, testable.get(actual));
    ASSERT_EQ_MSG(1.5f, actual);
}

int main () {
    START(ScannerTest);

    RUN_TEST_F(ScannerTest, get_char);
    RUN_TEST_F(ScannerTest, gets_default_delimeter);
    RUN_TEST_F(ScannerTest, gets_non_default_delimeter);
    RUN_TEST_F(ScannerTest, get_int_newlineSeperated);
    RUN_TEST_F(ScannerTest, get_int_spaceSeperated);
    RUN_TEST_F(ScannerTest, get_int_tabSeperated);
    RUN_TEST_F(ScannerTest, streamOperator_multipleTokens);
    RUN_TEST_F(ScannerTest, get_float);

    COMPLETE();
}
