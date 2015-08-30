/**
 * @file    spi_test.cpp
 *
 * @author  David Zemon
 *
 * Hardware:
 *      Connect a logic analyzer or oscilloscope to pins 0, 1 and 2
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

#include "PropWareTests.h"
#include <PropWare/spi.h>

const PropWare::Pin::Mask MOSI_MASK = PropWare::Port::P0;
const PropWare::Pin::Mask MISO_MASK = PropWare::Port::P1;
const PropWare::Pin::Mask SCLK_MASK = PropWare::Port::P2;
const PropWare::Pin       CS(PropWare::Port::P3, PropWare::Pin::OUT);

const unsigned int FREQUENCY = 900000;

PropWare::SPI *testable;

SETUP {
    testable = new PropWare::SPI(MOSI_MASK, MISO_MASK, SCLK_MASK, FREQUENCY, PropWare::SPI::MODE_0,
                                 PropWare::SPI::MSB_FIRST);
    CS.clear();
};

TEARDOWN {
    CS.set();
    if (NULL != testable) {
        delete testable;
        testable = NULL;
    }
};

TEST(ShiftOut_MsbFirst) {
    setUp();

    testable->shift_out(8, 0x55);
    testable->shift_out(8, 0x55);
    testable->shift_out(8, 0x55);

    tearDown();
}

TEST(ShiftOut_LsbFirst) {
    setUp();

    testable->set_bit_mode(PropWare::SPI::LSB_FIRST);

    testable->shift_out(8, 0xAA);
    testable->shift_out(8, 0xAA);
    testable->shift_out(8, 0xAA);

    tearDown();
}

int main () {
    CS.set();
    START(SPITest);

    RUN_TEST(ShiftOut_MsbFirst);
    RUN_TEST(ShiftOut_LsbFirst);

    COMPLETE();
}
