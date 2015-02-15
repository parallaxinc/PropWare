/**
 * @file    MCP3000_Demo.cpp
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
#include <PropWare/printer/printer.h>
#include <PropWare/mcp3000.h>

/**
* @brief       Report errors to the Debug LEDs for user interpretation
*
* @param[in]   err     Error value
*/
void error (const PropWare::SPI *spi, const PropWare::ErrorCode err);

/** Used for determining the bit-width of the ADC channel (10, 12, or 13 bit) */
const PropWare::MCP3000::PartNumber PART_NUMBER = PropWare::MCP3000::MCP300x;
const PropWare::MCP3000::Channel    CHANNEL     = PropWare::MCP3000::CHANNEL_1;

/** Pin number for MOSI (master out - slave in) */
const PropWare::Port::Mask          MOSI        = PropWare::Port::P0;
/** Pin number for MISO (master in - slave out) */
const PropWare::Port::Mask          MISO        = PropWare::Port::P1;
/** Pin number for the clock signal */
const PropWare::Port::Mask          SCLK        = PropWare::Port::P2;
/** Pin number for chip select */
const PropWare::Port::Mask          CS          = PropWare::Port::P3;
const uint32_t                      FREQ        = 100000;

// Main function
int main () {
    const uint16_t      DIVISOR = 1024 / 8;
    PropWare::ErrorCode err;
    uint16_t            data;
    uint32_t            loopCounter;
    uint8_t             scaledValue, i;
    uint32_t            ledOutput;
    PropWare::SPI       *spi    = PropWare::SPI::get_instance();
    PropWare::MCP3000   adc(spi, PART_NUMBER);

    // Set the Quickstart LEDs for output (used as a secondary display)
    PropWare::SimplePort scale(PropWare::Port::P16, 8, PropWare::Pin::OUT);

    if ((err = adc.start(MOSI, MISO, SCLK, CS)))
        error(spi, err);

    // Retrieve the SPI module and manually set the clock frequency
    spi->set_clock(FREQ);

    // Though this functional call is not necessary (default value is 0), I
    // want to bring attention to this function. It will determine whether the
    // adc.read* functions will always explicitly set the SPI modes before
    // each call, or assume that the SPI cog is still running in the proper
    // configuration
    adc.always_set_spi_mode(0);

    pwOut.printf("Welcome to the MCP3000 demo!" CRLF);

    while (1) {
        loopCounter = SECOND / 2 + CNT;

        // Loop over the LED output very quickly, until we are within 1
        // millisecond of total period
        while (abs(loopCounter - CNT) > MILLISECOND) {
            if ((err = adc.read(CHANNEL, &data)))
                error(spi, err);

            // Turn on LEDs proportional to the analog value
            scaledValue = (uint8_t) ((data + DIVISOR / 2 - 1) / DIVISOR);
            ledOutput = 0;
            for (i = 0; i < scaledValue; ++i)
                ledOutput = (ledOutput << 1) | 1;
            scale.write(ledOutput);
        }

        pwOut.printf("Channel %d is reading: %d" CRLF, CHANNEL, data);
    }
}

void error (const PropWare::SPI *spi, const PropWare::ErrorCode err) {
    PropWare::SimplePort debugLEDs(PropWare::Port::P16, 8, PropWare::Pin::OUT);

    if (PropWare::SPI::BEG_ERROR <= err && err < PropWare::SPI::END_ERROR)
        spi->print_error_str(&pwOut, (PropWare::SPI::ErrorCode const) err);
    else
        pwOut.printf("Unknown error: %u", err);

    while (1) {
        debugLEDs.write((uint32_t) err);
        waitcnt(150*MILLISECOND + CNT);
        debugLEDs.write(0);
        waitcnt(150*MILLISECOND + CNT);
    }
}
