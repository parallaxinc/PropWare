/**
 * @file    MAX6675_Demo.c
 */
/**
 * @project MAX6675_Demo
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

#include "MAX6675_Demo.h"

void main (void) {
    int8_t err;
    uint16_t data;
    char buffer[32];
    uint32_t loopCounter;

    if ((err = MAX6675Start(MOSI, MISO, SCLK, CS)))
        error(err);

    if ((err = HD44780Start(DATA, RS, RW, EN, BITMODE, DIMENSIONS)))
        error(err);

    // Though this functional call is not necessary (default value is 0), I
    // want to bring attention to this function. It will determine whether the
    // MAX6675Read* functions will always explicitly set the SPI modes before
    // each call, or assume that the SPI cog is still running in the proper
    // configuration
    MAX6675AlwaysSetMode(1);

    HD44780_puts("Welcome to the MAX6675 demo!\n");

    while (1) {
        loopCounter = CLKFREQ / 2 + CNT;

        if ((err = MAX6675Read(&data)))
            error(err);

        sprintf(buffer, "Temp: %u.%uC\n", data >> 2, (data & 0x3) * 25);
        HD44780Clear();
        HD44780_puts(buffer);

        waitcnt(loopCounter);
    }
}

void error (int8_t err) {
    uint32_t shiftedValue = (uint8_t) err;

    // Shift the error bits by 16 to put them atop the QUICKSTART LEDs
    shiftedValue <<= 16;

    GPIODirModeSet(DEBUG_LEDS, GPIO_DIR_OUT);

    while (1) {
        GPIOPinWrite(DEBUG_LEDS, shiftedValue);
        waitcnt(CLKFREQ/5 + CNT);
        GPIOPinClear(DEBUG_LEDS);
        waitcnt(CLKFREQ/5 + CNT);
    }
}
