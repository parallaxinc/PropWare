/**
 * @file    L3G_Demo.cpp
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
#include <PropWare/l3g.h>
#include <PropWare/uart/simplexuart.h>

/** Pin number for MOSI (master out - slave in) */
const PropWare::Port::Mask MOSI = PropWare::Port::P0;
/** Pin number for MISO (master in - slave out) */
const PropWare::Port::Mask MISO = PropWare::Port::P1;
/** Pin number for the clock signal */
const PropWare::Port::Mask SCLK = PropWare::Port::P2;
/** Pin number for chip select */
const PropWare::Port::Mask CS   = PropWare::Port::P6;
/** Frequency (in Hertz) to run the SPI protocol */
const uint32_t             FREQ = 10000;

void error (const PropWare::ErrorCode err);

// Main function
int main () {
    int16_t       gyroValues[3];
    PropWare::SPI *spi = PropWare::SPI::get_instance();
    PropWare::L3G gyro(spi);

    gyro.start(MOSI, MISO, SCLK, CS);
    gyro.set_dps(PropWare::L3G::DPS_500);

    // Though this functional call is not necessary (default value is 0), I
    // want to bring attention to this function. It will determine whether the
    // l3g_read* functions will always explicitly set the SPI modes before
    // each call, or assume that the SPI cog is still running in the proper
    // configuration
    gyro.always_set_spi_mode(1);

    while (1) {
        gyro.read_all(gyroValues);
        //pwOut << "Gyro vals DPS... X: " << gyro.convert_to_dps(gyroValues[PropWare::L3G::X])
        //        << "\tY: " << gyro.convert_to_dps(gyroValues[PropWare::L3G::Y])
        //        << "\tZ: " << gyro.convert_to_dps(gyroValues[PropWare::L3G::Z]) << '\n';

        pwOut << "Gyro vals DPS... X: " << gyroValues[PropWare::L3G::X]
                << "\tY: " << gyroValues[PropWare::L3G::Y]
                << "\tZ: " << gyroValues[PropWare::L3G::Z] << '\n';

        waitcnt(50*MILLISECOND + CNT);
    }

    return 0;
}

void error (const PropWare::ErrorCode err) {
    // Set the Quickstart LEDs for output (used to display the error code)
    PropWare::SimplePort debugLEDs(PropWare::Port::P16, 8, PropWare::Pin::OUT);

    while (1) {
        debugLEDs.write((uint32_t) err);
        waitcnt(CLKFREQ / 5 + CNT);
        debugLEDs.write(0);
        waitcnt(CLKFREQ / 5 + CNT);
    }
}
