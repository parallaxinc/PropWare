/**
 * @file    SPI_Demo.c
 *
 * @project SPI_Demo
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

#include "SPI_Demo.h"

/**
 * @brief   Write "Hello world!\n" out via SPI protocol and receive an echo
 */
void main (void) {
    char string[] = "Hello world!\n";  // Create the test string
    char *s;    // Create a pointer variable that can be incremented in a loop
    char in;    // Create an input variable to store received values from SPI

    // Initialize SPI module, giving it pin masks for the physical pins,
    // frequency for the clock, mode of SPI, and bitmode
    spi_start(MOSI, MISO, SCLK, FREQ, MODE, BITMODE);

    // Set chip select as an output (Note: the SPI module does not control chip
    // select)
    gpio_set_dir(CS, GPIO_DIR_OUT);

    gpio_set_dir(BYTE_2, GPIO_DIR_OUT);

    while (1) {
        s = string;         // Set the pointer to the beginning of the string
        while (*s) {        // Loop until we read the null-terminator

            waitcnt(CLKFREQ/100 + CNT);

            gpio_pin_clear(CS);       // Enable the SPI slave attached to CS
            spi_shift_out(8, *s);  // Output the next character of the string

            // Be sure to wait until the SPI communication has FINISHED before
            // proceeding to set chip select high
            spi_wait();
            gpio_pin_set(CS);

            waitcnt(CLKFREQ/100 + CNT);
            in = 0xff;              // Reset input variable
            while (in != *s) {
                gpio_pin_clear(CS);
                spi_shift_in(8, &in, 1);  // Read in a value from the SPI device
                gpio_pin_set(CS);
            }

            // Increment the character pointer
            ++s;

            // Print the character to the screen
            putchar(in);
        }

        // Signal that the entire string has been sent
        gpio_pin_toggle(BYTE_2);
    }
}
