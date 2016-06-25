/**
 * @file    MCP2515_Demo.cpp
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
#include <PropWare/serial/can/mcp2515.h>
#include <PropWare/utility/runnable.h>

const PropWare::Port::Mask MOSI = PropWare::Port::P0;
const PropWare::Port::Mask MISO = PropWare::Port::P1;
const PropWare::Port::Mask SCLK = PropWare::Port::P2;
const PropWare::Port::Mask CS = PropWare::Port::P7;
const PropWare::Port::Mask CLOCK_MASK = PropWare::Port::P8;

/**
 * @example     MCP2515_Demo.cpp
 *
 * @include PropWare_MCP2515/CMakeLists.txt
 */
int main () {
    const PropWare::Pin clock(CLOCK_MASK, PropWare::Pin::OUT);
    clock.start_hardware_pwm(1000000);

    const PropWare::SPI spi(MOSI, MISO, SCLK);
    PropWare::MCP2515 can(spi, CS, &pwOut);

    const PropWare::ErrorCode error = can.start(PropWare::MCP2515::BAUD_1000KBPS);
    pwOut << "Error code: " << error << '\n';
}
