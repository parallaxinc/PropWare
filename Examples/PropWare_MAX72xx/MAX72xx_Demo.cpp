/**
 * @file    MAX72xx_Demo.cpp
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

#include <PropWare/hmi/output/max72xx.h>

using PropWare::SPI;
using PropWare::Port;
using PropWare::MAX72xx;

/**
 * @example     MAX72xx_Demo.cpp
 *
 * Count from -200 to 200 with a 20 millisecond delay between each number
 *
 * @include PropWare_MAX72xx/CMakeLists.txt
 */
int main () {
    SPI::get_instance().set_mosi(Port::P2);
    SPI::get_instance().set_sclk(Port::P0);
    const MAX72xx leds(Port::P4);

    leds.start();
    for (int i = -200; i <= 200; ++i) {
        leds.put_int(i, MAX72xx::BcdCharacter::ZERO, 4);
        waitcnt(20 * MILLISECOND + CNT);
    }

    return 0;
}

