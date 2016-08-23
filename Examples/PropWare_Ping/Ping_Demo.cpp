/**
 * @file    Ping_Demo.cpp
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

#include <PropWare/sensor/distance/ping.h>
#include <PropWare/hmi/output/printer.h>

/**
 * @example     Ping_Demo.cpp
 *
 * Basic usage of the PropWare::Ping class to measure distance via an ultrasonic sensor
 *
 * @include PropWare_Blinky/CMakeLists.txt
 */
int main () {
    const PropWare::Ping ping(PropWare::Port::Mask::P14, PropWare::Port::Mask::P15);

    while (1) {
        pwOut << "Millimeters: " << ping.get_millimeters() << '\n';
        pwOut << "Centimeters: " << ping.get_centimeters() << '\n';
        pwOut << "Inches:      " << ping.get_inches() << '\n';
        waitcnt(SECOND / 2 + CNT);
    }
}
