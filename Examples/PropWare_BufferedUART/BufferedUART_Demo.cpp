/**
 * @file    BufferedUART_Demo.cpp
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
#include <PropWare/gpio/pin.h>
#include "BufferedUART_Demo.h"

static BufferedUARTMailbox transmitMailbox;
static BufferedUARTMailbox receiveMailbox;

static const size_t BUFFER_SIZE = 256;

static char transmitBuffer[BUFFER_SIZE];
static char receiveBuffer[BUFFER_SIZE];

void runScannerDemo (PropWare::Printer &queuePrinter, PropWare::Scanner &queueScanner);

bool isAnswerNo (char const userInput[]);

/**
 * @example     BufferedUARTRX_Demo.cpp
 *
 * Communicate via independent and buffered UART drivers. Note that this is just an easy way to show buffered UART
 * drivers, not a recommended application. For interaction with a human, the static variables `pwOut` and `pwIn` will
 * be more effecient. Buffered UART drivers are great for interacting with other machines/microcontrollers/sensors/etc.
 *
 * @include Examples/PropWare_UARTRX/CMakeLists.txt
 * @include Examples/PropWare_UARTRX/BufferedUART_Demo.h
 * @include Examples/PropWare_UARTRX/buffereduarttx.cogcpp
 * @include Examples/PropWare_UARTRX/buffereduartrx.cogcpp
 */
int main () {
    PropWare::CharQueue transmitQueue(transmitBuffer, BUFFER_SIZE, locknew());
    extern unsigned int         _load_start_buffereduarttx_cog[];
    transmitMailbox.queue = &transmitQueue;
    cognew(_load_start_buffereduarttx_cog, &transmitMailbox.queue);
    PropWare::Printer queuePrinter(transmitQueue);

    PropWare::CharQueue receiveQueue(receiveBuffer);
    extern unsigned int _load_start_buffereduartrx_cog[];
    receiveMailbox.queue = &receiveQueue;
    cognew(_load_start_buffereduartrx_cog, &receiveMailbox.queue);
    PropWare::Scanner queueScanner(receiveQueue, &queuePrinter);

    // This should look very familiar - it is the same code as Examples/PropWare_Scanner/Scanner_Demo.cpp
    runScannerDemo(queuePrinter, queueScanner);

    while (1);
}

void runScannerDemo (PropWare::Printer &queuePrinter, PropWare::Scanner &queueScanner) {
    // A nice big buffer that can hold up to 63 characters from the user (the
    // 64th is used by the null-terminator)
    char         userInput[64];
    char         name[64];
    unsigned int age;

    queuePrinter << "Hello! I'd like to teach you how to use PropWare to read from the terminal!\n";

    do {
        queuePrinter << "First, what's your name?\n>>> ";
        queueScanner >> name;

        queuePrinter << "And how old are you?\n>>> ";
        queueScanner >> age;

        queuePrinter << "Is your name " << name << " and are you " << age << " years old?\n>>> ";
        queueScanner.input_prompt("", "Please enter either 'yes' or 'no' (y/n)\n>>> ", userInput, sizeof(userInput),
                                  YES_NO_COMP);
    } while (isAnswerNo(userInput));

    queuePrinter << "Hello, " << name << "!\n";
}

bool isAnswerNo (char const userInput[]) {
    return 0 == strcmp("n", userInput) || 0 == strcmp("no", userInput);
}
