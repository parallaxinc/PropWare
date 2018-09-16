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
#include <PropWare/serial/spi/spi.h>

using PropWare::Pin;
using PropWare::Port;
using PropWare::SPI;

const Pin::Mask MOSI_MASK = Port::P0;
const Pin::Mask MISO_MASK = Port::P1;
const Pin::Mask SCLK_MASK = Port::P2;
const Pin       CS(Port::P3, Pin::Dir::OUT);

const unsigned int FREQUENCY = 900000;

class SpiTest {
    public:
        SpiTest () {
            this->testable = new SPI(MOSI_MASK, MISO_MASK, SCLK_MASK, FREQUENCY, SPI::Mode::MODE_0,
                                     SPI::BitMode::MSB_FIRST);
            CS.clear();
        }

        ~SpiTest () {
            CS.set();
        }

    public:
        SPI *testable;
};


TEST_F(SpiTest, ShiftOut_MsbFirst) {
    testable->shift_out(8, 0x55);
    testable->shift_out(8, 0x55);
    testable->shift_out(8, 0x55);
}

TEST_F(SpiTest, ShiftOut_LsbFirst) {
    testable->set_bit_mode(SPI::BitMode::LSB_FIRST);

    testable->shift_out(8, 0xAA);
    testable->shift_out(8, 0xAA);
    testable->shift_out(8, 0xAA);
}

TEST_F(SpiTest, ShiftOutBlock) {
    const int BUFFER_SIZE = 16;

    uint8_t      buffer[BUFFER_SIZE];
    for (uint8_t i        = 0; i < BUFFER_SIZE; ++i)
        buffer[i] = i;

    testable->shift_out_block_msb_first_fast(buffer, sizeof(buffer));
}

int main () {
    CS.set();
    START(SPITest_MUST_USE_LOGIC_ANALYZER);

    RUN_TEST_F(SpiTest, ShiftOut_MsbFirst);
    RUN_TEST_F(SpiTest, ShiftOut_LsbFirst);
    RUN_TEST_F(SpiTest, ShiftOutBlock);

    COMPLETE();
}
