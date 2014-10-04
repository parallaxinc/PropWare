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
#include "L3G_Demo.h"

// Main function
int main () {
    PropWare::ErrorCode err;
    int16_t             gyroVals[3];
    PropWare::SPI       *spi = PropWare::SPI::get_instance();
    PropWare::L3G       gyro(spi);

    if ((err = gyro.start(MOSI, MISO, SCLK, CS)))
        error(err);
    if ((err = gyro.set_dps(PropWare::L3G::DPS_500)))
        error(err);

    // Though this functional call is not necessary (default value is 0), I
    // want to bring attention to this function. It will determine whether the
    // l3g_read* functions will always explicitly set the SPI modes before
    // each call, or assume that the SPI cog is still running in the proper
    // configuration
    gyro.always_set_spi_mode(1);

    while (1) {
        if ((err = gyro.read_all(gyroVals)))
            error(err);
//        print("Gyro vals DPS... X: %2.3f\tY: %2.3f\tZ: %2.3f" CRLF,
//              gyro.convert_to_dps(gyroVals[PropWare::L3G::X]),
//              gyro.convert_to_dps(gyroVals[PropWare::L3G::Y]),
//              gyro.convert_to_dps(gyroVals[PropWare::L3G::Z]));

        print("Gyro vals... X: %d\tY: %d\tZ: %d" CRLF,
              gyroVals[PropWare::L3G::X],
              gyroVals[PropWare::L3G::Y],
              gyroVals[PropWare::L3G::Z]);

//        waitcnt(50*MILLISECOND + CNT);
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
