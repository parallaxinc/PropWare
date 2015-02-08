/**
 * @file    SimplexUART_Demo.cpp
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
#include <PropWare/uart/simplexuart.h>
#include <PropWare/printer/printer.h>

void error (const PropWare::ErrorCode err);

const int32_t BAUD_RATE = 115200;
const int32_t DELAY     = 200;

/**
 * @brief   Write "Hello world!" out via UART protocol
 */
int main () {
    PropWare::ErrorCode err;
    PropWare::SimplexUART uart;

    // Create an easy-to-test number pattern - useful when testing with a logic
    // analyzer
    char numberPattern[] = {
            0x01,
            0x02,
            0x03,
            0x45,
            0xe5,
            0xaa,
            0xff,
            0x80,
            0x00 }; // Make sure we have a null-terminator for puts

    // Create the test string - useful when testing with a terminal
    char string[] = "Hello world! This is my most favoritest sentence "
            "ever!!!" CRLF;

    // Typical RS232 settings (default settings for PropGCC serial comms)
    uart.set_baud_rate(BAUD_RATE);
    if ((err = uart.set_data_width(8)))
        error(err);
    if ((err = uart.set_stop_bit_width(1)))
        error(err);
    uart.set_parity(PropWare::UART::NO_PARITY);

    while (1) {
        // Test the number pattern
        uart.puts(numberPattern);
        waitcnt(DELAY * MILLISECOND + CNT);

        // Test a basic string
        uart.puts(string);
        waitcnt(DELAY * MILLISECOND + CNT);
    }
}

void error (const PropWare::ErrorCode err) {
    PropWare::SimplePort debugLEDs(PropWare::Port::P16, 8, PropWare::Pin::OUT);

    while (1) {
        debugLEDs.write((uint32_t) err);
        waitcnt(100*MILLISECOND);
        debugLEDs.write(0);
        waitcnt(100*MILLISECOND);
    }
}
