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
#include <PropWare/gpio/simpleport.h>
#include <PropWare/sensor/gyroscope/l3g.h>

using PropWare::Port;
using PropWare::Pin;
using PropWare::SPI;
using PropWare::L3G;
using PropWare::SimplePort;

/** Pin number for MOSI (master out - slave in) */
static const Port::Mask MOSI = Port::Mask::P0;
/** Pin number for MISO (master in - slave out) */
static const Port::Mask MISO = Port::Mask::P1;
/** Pin number for the clock signal */
static const Port::Mask SCLK = Port::Mask::P2;
/** Pin number for chip select */
static const Port::Mask CS   = Port::Mask::P6;

void error(const PropWare::ErrorCode err);

/**
 * @example     L3G_Demo.cpp
 *
 * Read the gyrometer data and print it to the terminal
 *
 * @include PropWare_L3G/CMakeLists.txt
 */
int main() {
    int16_t rawGyroValues[3];
    float   gyroValues[3];

    SPI spi = SPI::get_instance();
    spi.set_mosi(MOSI);
    spi.set_miso(MISO);
    spi.set_sclk(SCLK);
    L3G gyro(spi, CS);

    gyro.start();

    // Though this functional call is not necessary (default value is 0), I
    // want to bring attention to this function. It will determine whether the
    // l3g_read* functions will always explicitly set the SPI modes before
    // each call, or assume that the SPI driver is still running in the proper
    // configuration
    gyro.always_set_spi_mode(1);

    while (1) {
        gyro.read_all(rawGyroValues);

        gyroValues[L3G::X] = gyro.convert_to_dps(rawGyroValues[L3G::X]);
        gyroValues[L3G::Y] = gyro.convert_to_dps(rawGyroValues[L3G::Y]);
        gyroValues[L3G::Z] = gyro.convert_to_dps(rawGyroValues[L3G::Z]);

        pwOut << "X: " << gyroValues[L3G::X] << '\t'
              << "Y: " << gyroValues[L3G::Y] << '\t'
              << "Z: " << gyroValues[L3G::Z] << '\n';

        waitcnt(100 * MILLISECOND + CNT);
    }
}

void error(const PropWare::ErrorCode err) {
    // Set the Quickstart LEDs for output (used to display the error code)
    SimplePort debugLEDs(Port::P16, 8, Pin::Dir::OUT);

    while (1) {
        debugLEDs.write((uint32_t) err);
        waitcnt(CLKFREQ / 5 + CNT);
        debugLEDs.write(0);
        waitcnt(CLKFREQ / 5 + CNT);
    }
}
