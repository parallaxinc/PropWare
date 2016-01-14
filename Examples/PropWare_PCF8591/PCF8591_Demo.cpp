/**
 * @file    PCF8591_Demo.cpp
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

#include <PropWare/pcf8591.h>
#include <PropWare/printer/printer.h>

/**
 * @example     PCF8591_Demo.cpp
 *
 * Read an ADC channel from a PCF8591 device
 *
 * @include PropWare_PCF8591/CMakeLists.txt
 */
int main () {
    PropWare::PCF8591 pcf8591;

    bool pingSuccess = pcf8591.ping();
    pwOut << "PCF8591 ping = " << pingSuccess << '\n';

    if (pingSuccess) {
        uint8_t value = pcf8591.read_channel(PropWare::PCF8591::CHANNEL_0);
        while (1) {
            pwOut << "PCF: " << value << '\n';
            waitcnt(250 * MILLISECOND + CNT);
            value = pcf8591.read_channel(PropWare::PCF8591::CHANNEL_0);
        }
    }

    return 0;
}
