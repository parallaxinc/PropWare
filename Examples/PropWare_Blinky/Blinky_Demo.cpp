/**
 * @file    Blinky_Demo.cpp
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
#include <PropWare/gpio/pin.h>

using PropWare::Pin;
using PropWare::Port;

/**
 * @example     Blinky_Demo.cpp
 *
 * Basic usage of the PropWare::Pin class to blink an LED four times a second
 *
 * @include PropWare_Blinky/CMakeLists.txt
 */
int main () {
    // Use the hardware counter module to blink an LED at 4Hz
    const Pin led1(Port::P17, Pin::Dir::OUT);
    // Notice that this method is non-blocking, because the Propeller's built-in hardware counter does all the hard work
    led1.start_hardware_pwm(4);

    // We'll toggle this LED in software. This is also know as "bit banging"
    const Pin led2(Port::P16, Pin::Dir::OUT);
    while (1) {
        led2.toggle();
        waitcnt(CLKFREQ / 4 + CNT);
    }
}
