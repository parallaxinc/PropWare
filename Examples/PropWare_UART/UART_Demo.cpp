/**
 * @file    UART_Demo.cpp
 *
 * @project UART_Demo
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

#include "UART_Demo.h"

/**
 * @brief   Write "Hello world!\n" out via SPI protocol and receive an echo
 */
int main () {
    char string[] = {0x01, 0x02, 0x03, 0x45, 0xe5, 0xaa, 0xff, 0x80, 0x00};// "Hello world!\n";  // Create the test string
    char *s;    // Create a pointer variable that can be incremented in a loop
    PropWare::SimplexUART uart(TX);
    PropWare::SimplePort debugLEDs(PropWare::Port::P16, 8, PropWare::Pin::OUT);

    uart.set_baud_rate(BAUD);
    uart.set_data_width(8);
    uart.set_stop_bit_width(1);
    uart.set_parity(PropWare::UART::NO_PARITY);

    while (1) {
        s = string;         // Set the pointer to the beginning of the string
        while (*s) {        // Loop until we read the null-terminator
            uart.send(*s);  // Output the next character of the string

            // Increment the character pointer
            ++s;
        }

        // Signal that the entire string has been sent
//        debugLEDs.toggle();
    }

    return 0;
}

void error (const PropWare::ErrorCode err) {
    PropWare::SimplePort debugLEDs(PropWare::Port::P16, 8, PropWare::Pin::OUT);

    printf("Unknown error %u\n", err);

    while (1) {
        debugLEDs.write(err);
        waitcnt(100*MILLISECOND);
        debugLEDs.write(0);
        waitcnt(100*MILLISECOND);
    }
}