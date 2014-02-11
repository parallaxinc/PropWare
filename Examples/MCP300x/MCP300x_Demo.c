/**
 * @file    MCP300x_Demo.c
 *
 * @project MCP300x_Demo
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

#include "MCP300x_Demo.h"

// Main function
void main (void) {
    int8_t err;
    uint16_t data;
    uint32_t loopCounter;
    uint16_t divisor = 1024 / 8;
    uint8_t scaledValue, i;
    uint32_t ledOutput;

    if ((err = mcp300x_start(MOSI, MISO, SCLK, CS)))
        error(err);
    spi_set_clock(FREQ);

    // Set the Quickstart LEDs for output (used as a secondary display)
    gpio_set_dir(DEBUG_LEDS, GPIO_DIR_OUT);

    // Though this functional call is not necessary (default value is 0), I
    // want to bring attention to this function. It will determine whether the
    // mcp300x_read* functions will always explicitly set the SPI modes before
    // each call, or assume that the SPI cog is still running in the proper
    // configuration
    mcp300x_always_set_spi_mode(0);

    __simple_printf("Welcome to the MCP300x demo!\n");

    while (1) {
        loopCounter = CLKFREQ / 2 + CNT;

        // Loop over the LED output very quickly, until we are within 1
        // millisecond of total period
        while (abs(loopCounter - CNT) > CLKFREQ / 1000) {
            if ((err = mcp300x_read(CHANNEL, &data)))
                error(err);

            // Turn on LEDs proportional to the analog value
            scaledValue = (data + divisor / 2 - 1) / divisor;
            ledOutput = 0;
            for (i = 0; i < scaledValue; ++i)
                ledOutput = (ledOutput << 1) | 1;
            ledOutput <<= 16;
            gpio_pin_write(DEBUG_LEDS, ledOutput);
        }

        __simple_printf("Channel %u is reading: %u\n", CHANNEL, data);
    }
}

void error (int8_t err) {
    uint32_t shiftedValue = err;
    shiftedValue <<= 16;  // Shift the error bits by 16 to put them atop the QUICKSTART LEDs

    gpio_set_dir(DEBUG_LEDS, GPIO_DIR_OUT);

    while (1) {
        gpio_pin_write(DEBUG_LEDS, shiftedValue);
        waitcnt(CLKFREQ/5 + CNT);
        gpio_pin_clear(DEBUG_LEDS);
        waitcnt(CLKFREQ/5 + CNT);
    }
}

