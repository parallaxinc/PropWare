/**
 * @file    WS2812_Demo.cpp
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

#include <PropWare/ws2812.h>

const PropWare::Pin::Mask LED_TX_PIN_MASK = PropWare::Pin::P27;
/**
 * @example     WS2812_Demo.cpp
 *
 * Continuously cycle an RGB LED between bright and dim white
 *
 * @include PropWare_WS2812/CMakeLists.txt
 */
int main () {
    const PropWare::WS2812 led(LED_TX_PIN_MASK, PropWare::WS2812::GRB);

    const int delay = 40 * MILLISECOND;
    while (1) {
        unsigned int i;

        // Go bright
        for (i = 0; i < 0x101010; i += 0x010101) {
            led.send(i);
            waitcnt(delay + CNT);
        }

        // Go dim
        for (; i; i -= 0x010101) {
            led.send(i);
            waitcnt(delay + CNT);
        }
    }
}

