/**
 * @file    HD44780_Demo.cpp
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
#include <PropWare/string/printer/printer.h>
#include <PropWare/hmi/output/hd44780.h>

// Control pins
const PropWare::Port::Mask RS = PropWare::Port::P16;
const PropWare::Port::Mask RW = PropWare::Port::P17;
const PropWare::Port::Mask EN = PropWare::Port::P18;

// Data pins
const PropWare::Port::Mask          FIRST_DATA_PIN = PropWare::Port::P19;
const PropWare::HD44780::Bitmode    BITMODE        = PropWare::HD44780::BM_8;
const PropWare::HD44780::Dimensions DIMENSIONS     = PropWare::HD44780::DIM_16x2;

/**
 * @example     HD44780_Demo.cpp
 *
 * Utilize the PropWare::Printer class to print formatted text to an LCD
 *
 * @include PropWare_HD44780/CMakeLists.txt
 */
int main () {
    // Create and initialize our LCD object
    PropWare::HD44780 lcd;
    lcd.start(FIRST_DATA_PIN, RS, RW, EN, BITMODE, DIMENSIONS);

    // Create a printer for easy, formatted writing to the LCD
    PropWare::Printer lcdPrinter(lcd);

    // Print to the LCD (exactly 32 characters so that we fill up both lines)
    lcdPrinter.printf("%u %s%d 0x%07X", 123456789, "Hello!", -12345, 0xabcdef);

    return 0;
}
