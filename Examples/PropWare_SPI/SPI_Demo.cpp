/**
 * @file    SPI_Demo.cpp
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
#include <PropWare/spi.h>
#include <PropWare/uart/simplexuart.h>

void error (const PropWare::ErrorCode err, const PropWare::SPI *spi);

/** Pin number for MOSI (master out - slave in) */
const PropWare::Port::Mask MOSI = PropWare::Port::P0;
/** Pin number for MISO (master in - slave out) */
const PropWare::Port::Mask MISO = PropWare::Port::P1;
/** Pin number for the clock signal */
const PropWare::Port::Mask SCLK = PropWare::Port::P2;
/** Pin number for chip select */
const PropWare::Port::Mask CS   = PropWare::Port::P6;

/** Frequency (in hertz) to run the SPI module */
const uint32_t               FREQ    = 100000;
/** The SPI mode to run */
const PropWare::SPI::Mode    MODE    = PropWare::SPI::MODE_0;
/** Determine if the LSB or MSB should be sent first for each byte */
const PropWare::SPI::BitMode BITMODE = PropWare::SPI::MSB_FIRST;

/**
 * @brief   Write "Hello world!" out via SPI protocol and receive an echo
 */
int main () {
    char string[] = "Hello world!" CRLF;  // Create the test string
    char *s;    // Create a pointer variable that can be incremented in a loop
    char in;    // Create an input variable to store received values from SPI
    PropWare::SPI *spi = PropWare::SPI::get_instance();

    // Initialize SPI module, giving it pin masks for the physical pins,
    // frequency for the clock, mode of SPI, and bitmode
    spi->start(MOSI, MISO, SCLK, FREQ, MODE, BITMODE);

    // Set chip select as an output (Note: the SPI module does not control chip
    // select)
    PropWare::Pin cs(CS, PropWare::Pin::OUT);

    PropWare::SimplePort debugLEDs(PropWare::Port::P16, 8, PropWare::Pin::OUT);

    while (1) {
        s = string;         // Set the pointer to the beginning of the string
        while (*s) {        // Loop until we read the null-terminator

            waitcnt(CLKFREQ/100 + CNT);

            cs.clear();  // Enable the SPI slave attached to CS
            spi->shift_out(8, (uint32_t) *s);  // Output the next character of the string

            // Be sure to wait until the SPI communication has FINISHED before
            // proceeding to set chip select high
            spi->wait();
            cs.set();

            waitcnt(CLKFREQ/100 + CNT);
            in = (char) 0xff;              // Reset input variable
            while (in != *s) {
                cs.clear();
                spi->shift_in(8, &in, 1);  // Read in a value from the SPI device
                cs.set();
            }

            // Increment the character pointer
            ++s;

            // Print the character to the screen
            pwOut.put_char(in);
        }

        // Signal that the entire string has been sent
        debugLEDs.toggle();
    }

    return 0;
}

void error (const PropWare::ErrorCode err, const PropWare::SPI *spi) {
    PropWare::SimplePort debugLEDs(PropWare::Port::P16, 8, PropWare::Pin::OUT);

    if (PropWare::SPI::BEG_ERROR <= err && err < PropWare::SPI::END_ERROR) {
        spi->print_error_str(&pwOut, (PropWare::SPI::ErrorCode) err);
    } else
        pwOut.printf("Unknown error %u" CRLF, err);

    while (1) {
        debugLEDs.write(err);
        waitcnt(100*MILLISECOND);
        debugLEDs.write(0);
        waitcnt(100*MILLISECOND);
    }
}
