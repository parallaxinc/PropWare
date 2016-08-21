/**
 * @file    FullDuplexSerial_Demo.cpp
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

#include <PropWare/hmi/output/printer.h>
#include <PropWare/hmi/input/scanner.h>
#include <PropWare/serial/uart/fullduplexserial.h>

bool isAnswerNo (char const userInput[]);

int main () {
    PropWare::FullDuplexSerial serial;
    serial.start();

    PropWare::Printer printer(serial);
    PropWare::Scanner scanner(serial);

    // A nice big buffer that can hold up to 63 characters from the user (the
    // 64th is used by the null-terminator)
    char         userInput[64];
    char         name[64];
    unsigned int age;

    printer << "Hello! I'd like to teach you how to use PropWare to read from the terminal!\n";

    do {
        printer << "First, what's your name?\n>>> ";
        scanner >> name;

        printer << "And how old are you?\n>>> ";
        scanner >> age;

        printer << "Is your name " << name << " and are you " << age << " years old?\n>>> ";
        scanner.input_prompt("", "Please enter either 'yes' or 'no' (y/n)\n>>> ", userInput, sizeof(userInput),
                             YES_NO_COMP);
    } while (isAnswerNo(userInput));

    printer << "Hello, " << name << "!\n";

    while (1);
}

bool isAnswerNo (char const userInput[]) {
    return 0 == strcmp("n", userInput) || 0 == strcmp("no", userInput);
}
