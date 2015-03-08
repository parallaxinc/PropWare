/**
 * @file    Scanner_Demo.cpp
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

// Includes
#include <PropWare/PropWare.h>
#include <PropWare/utility.h>
#include <PropWare/port.h>
#include <PropWare/printer/printer.h>
#include <PropWare/scanner.h>

class YesNoComparator : public PropWare::Comparator<char> {
    public:
        YesNoComparator () {}

        virtual bool valid (const char *userInput) const {
            char buffer[16];
            strcpy(buffer, userInput);
            PropWare::Utility::to_lower(buffer);
            return 0 == strcmp("n", buffer) ||
                    0 == strcmp("no", buffer) ||
                    0 == strcmp("y", buffer) ||
                    0 == strcmp("yes", buffer);
        }
};
const YesNoComparator yesNoComparator;

bool isAnswerNo (char const userInput[]);
void error (const PropWare::ErrorCode err);

int main () {
    // A nice big buffer that can hold up to 63 characters from the user (the
    // 64th is used by the null-terminator)
    char userInput[64];
    char name[64];

    pwOut.printf("Hello! I'd like to teach you how to use PropWare to read from the terminal!\n");

    do {
        pwOut.printf("First, what's your name?\n>>> ");
        pwIn.gets(userInput, sizeof(userInput));
        strcpy(name, userInput);

        pwOut.printf("I received [%s].\n", name);
        pwIn.input_prompt("Is that right?\n>>> ", "Please enter either 'yes' or 'no' (y/n)\n>>> ",
                          userInput, sizeof(userInput), yesNoComparator);
    } while (isAnswerNo(userInput));

    pwOut.printf("Hello, %s!\n", name);
    return 0;
}

bool isAnswerNo (char const userInput[]) {
    return 0 == strcmp("n", userInput) || 0 == strcmp("no", userInput);
}

void error (const PropWare::ErrorCode err) {
    const PropWare::SimplePort debugLEDs(PropWare::Port::P16, 8, PropWare::Port::OUT);

    while (1) {
        debugLEDs.write((uint32_t) err);
        waitcnt(100 * MILLISECOND);
        debugLEDs.write(0);
        waitcnt(100 * MILLISECOND);
    }
}
