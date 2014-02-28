/**
 * @file    HD44780_Demo.c
 *
 * @project HD44780_Demo
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

#include "HD44780_Demo.h"

using namespace PropWare::GPIO;

// Main function
int main () {
    uint8_t err;

    char buffer[128];

    PropWare::HD44780 lcd;

    if ((err = lcd.start(DATA, RS, RW, EN, BITMODE, DIMENSIONS)))
        error(err);

    sprintf(buffer, "%u %s%07d 0x%x", 123456789, "Hello!", -12345, 0xabcdef);
    lcd.putStr(buffer);

    return 0;
}

void error (const uint8_t err) {
    uint32_t out = err;
    out <<= 16;

    set_dir(PropWare::BYTE_2, OUT);

    while (1) {
        pin_write(PropWare::BYTE_2, out);
        waitcnt(150*MILLISECOND + CNT);
        pin_clear(PropWare::BYTE_2);
        waitcnt(150*MILLISECOND + CNT);
    }
}
