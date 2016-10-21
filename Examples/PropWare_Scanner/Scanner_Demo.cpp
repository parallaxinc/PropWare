/**
 * @file    Examples/PropWare_Scanner/Scanner_Demo.cpp
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
#include <PropWare/hmi/output/printer.h>
#include <PropWare/hmi/input/scanner.h>

static bool isAnswerNo (char const userInput[]);

/**
 * @example     Scanner_Demo.cpp
 *
 * Use a Scanner to read user input from the terminal and gather basic data about the user.
 *
 * @include PropWare_Scanner/CMakeLists.txt
 */
int main () {
    // A nice big buffer that can hold up to 63 characters from the user (the
    // 64th is used by the null-terminator)
    char         userInput[64];
    char         name[64];
    unsigned int age;

    pwOut << "Hello! I'd like to teach you how to use PropWare to read from the terminal!\n";

    do {
        pwOut << "First, what's your name?\n>>> ";
        pwIn >> name;

        pwOut << "And how old are you?\n>>> ";
        pwIn >> age;

        pwOut << "Is your name " << name << " and are you " << age << " years old?\n>>> ";
        pwIn.input_prompt("", "Please enter either 'yes' or 'no' (y/n)\n>>> ", userInput, sizeof(userInput),
                          YES_NO_COMP);
    } while (isAnswerNo(userInput));

    pwOut << "Hello, " << name << "!\n";
    return 0;
}

bool isAnswerNo (char const userInput[]) {
    return 0 == strcmp("n", userInput) || 0 == strcmp("no", userInput);
}
