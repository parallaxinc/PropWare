/**
 * @file    MAX6675_Demo.cpp
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

#include <PropWare/PropWare.h>
#include <PropWare/sensor/temperature/max6675.h>

/** Pin number for MOSI (master out - slave in) */
const PropWare::Port::Mask MOSI = PropWare::Port::Mask::P0;
/** Pin number for MISO (master in - slave out) */
const PropWare::Port::Mask MISO = PropWare::Port::Mask::P1;
/** Pin number for the clock signal */
const PropWare::Port::Mask SCLK = PropWare::Port::Mask::P2;
/** Pin number for chip select */
const PropWare::Port::Mask CS   = PropWare::Port::Mask::P5;
const uint32_t             FREQ = 10000;

/**
 * @example     MAX6675_Demo.cpp
 *
 * Read the current temperature and print it to the terminal
 *
 * @include PropWare_MAX6675/CMakeLists.txt
 */
int main() {
    PropWare::SPI     spi = PropWare::SPI::get_instance();
    PropWare::MAX6675 thermo(spi, MOSI, MISO, SCLK, CS);

    // Though this functional call is not necessary (default value is 0), I
    // want to bring attention to this function. It will determine whether the
    // thermo.read* functions will always explicitly set the SPI modes before
    // each call, or assume that the SPI driver is still running in the proper
    // configuration
    thermo.always_set_spi_mode(1);

    pwOut << "Welcome to the MAX6675 demo!\n";

    uint16_t data;
    while (1) {
        data = thermo.read();
        pwOut.printf("Temp: %u.%uC\n", data >> 2, (data & 0x3) * 25);
        waitcnt(CLKFREQ / 2 + CNT);
    }
}
