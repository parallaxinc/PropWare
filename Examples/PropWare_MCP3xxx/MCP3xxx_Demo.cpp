/**
 * @file    MCP3xxx_Demo.cpp
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
#include <PropWare/hmi/output/printer.h>
#include <PropWare/gpio/simpleport.h>
#include <PropWare/sensor/analog/mcp3xxx.h>

using PropWare::MCP3xxx;
using PropWare::Port;
using PropWare::SPI;
using PropWare::SimplePort;
using PropWare::Pin;

/** Used for determining the bit-width of the ADC channel (10, 12, or 13 bit) */
static const MCP3xxx::PartNumber PART_NUMBER = MCP3xxx::PartNumber::MCP300x;
static const MCP3xxx::Channel    CHANNEL     = MCP3xxx::Channel::CHANNEL_1;

/** Pin number for MOSI (master out - slave in) */
static const Port::Mask MOSI = Port::Mask::P0;
/** Pin number for MISO (master in - slave out) */
static const Port::Mask MISO = Port::Mask::P1;
/** Pin number for the clock signal */
static const Port::Mask SCLK = Port::Mask::P2;
/** Pin number for chip select */
static const Port::Mask CS   = Port::Mask::P3;

/**
 * @example     MCP3xxx_Demo.cpp
 *
 * Continuously read the ADC value from a channel of the MCP3xxx and print it to the terminal.
 *
 * @include PropWare_MCP3xxx/CMakeLists.txt
 */
int main () {
    const uint16_t    DIVISOR = 1024 / 8;
    uint16_t          data = 0;
    uint32_t          loopCounter;
    uint8_t           scaledValue, i;
    uint32_t          ledOutput;
    SPI     spi(MOSI, MISO, SCLK);
    MCP3xxx adc(spi, CS, PART_NUMBER);

    // Set the Quickstart LEDs for output (used as a secondary display)
    SimplePort scale(Port::P16, 8, Pin::Dir::OUT);

    // Though this functional call is not necessary (default value is 0), I
    // want to bring attention to this function. It will determine whether the
    // adc.read* functions will always explicitly set the SPI modes before
    // each call, or assume that the SPI cog is still running in the proper
    // configuration
    adc.always_set_spi_mode(0);

    pwOut << "Welcome to the MCP3xxx demo!\n";

    while (1) {
        loopCounter = SECOND / 2 + CNT;

        // Loop over the LED output very quickly, until we are within 1
        // millisecond of total period
        while (abs(loopCounter - CNT) > MILLISECOND) {
            data = adc.read(CHANNEL);

            // Turn on LEDs proportional to the analog value
            scaledValue = (uint8_t) ((data + DIVISOR / 2 - 1) / DIVISOR);
            ledOutput   = 0;
            for (i      = 0; i < scaledValue; ++i)
                ledOutput = (ledOutput << 1) | 1;
            scale.write(ledOutput);
        }

        pwOut.printf("Channel %d is reading: %d\n", static_cast<int>(CHANNEL), data);
    }
}
