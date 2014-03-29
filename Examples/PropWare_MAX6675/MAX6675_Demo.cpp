/**
 * @file    MAX6675_Demo.cpp
 *
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

int main () {
    PropWare::ErrorCode err;
    uint16_t data;
    uint32_t loopCounter;
    char buffer[128];

    PropWare::HD44780 lcd;
    PropWare::SPI *spi = PropWare::SPI::getInstance();
    PropWare::MAX6675 thermo(spi);

    if ((err = thermo.start(MOSI, MISO, SCLK, CS)))
        error(err);

    if ((err = lcd.start(FIRST_DATA_PIN, RS, RW, EN, BITMODE, DIMENSIONS)))
        error(err);

    // Though this functional call is not necessary (default value is 0), I
    // want to bring attention to this function. It will determine whether the
    // thermo.read* functions will always explicitly set the SPI modes before
    // each call, or assume that the SPI cog is still running in the proper
    // configuration
    thermo.always_set_spi_mode(1);

    lcd.putStr("Welcome to the MAX6675 demo!\n");

    while (1) {
        loopCounter = CLKFREQ / 2 + CNT;

        if ((err = thermo.read(&data)))
            error(err);

        lcd.clear();
        sprintf(buffer, "Temp: %u.%uC\n", data >> 2, (data & 0x3) * 25);
        lcd.putStr(buffer);

        waitcnt(loopCounter);
    }

    return 0;
}

void error (const PropWare::ErrorCode err) {
    // Set the Quickstart LEDs for output (used to display the error code)
    PropWare::SimplePort debugLEDs(PropWare::Pin::P16, 8, PropWare::Pin::OUT);

    while (1) {
        debugLEDs.write(err);
        waitcnt(CLKFREQ/5 + CNT);
        debugLEDs.write(0);
        waitcnt(CLKFREQ/5 + CNT);
    }
}
