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
#include <PropWare/hmi/output/printer.h>
#include <PropWare/hmi/output/hd44780.h>

// Control pins
static const PropWare::Port::Mask RS = PropWare::Port::Mask::P16;
static const PropWare::Port::Mask RW = PropWare::Port::Mask::P17;
static const PropWare::Port::Mask EN = PropWare::Port::Mask::P18;

// Data pins
static const PropWare::Pin::Mask           FIRST_DATA_PIN = PropWare::Pin::Mask::P19;
static const PropWare::HD44780::BusWidth   BITMODE        = PropWare::HD44780::BusWidth::WIDTH8;
static const PropWare::HD44780::Dimensions DIMENSIONS     = PropWare::HD44780::Dimensions::DIM_16x2;

/**
 * @example     HD44780_Demo.cpp
 *
 * Utilize the PropWare::Printer class to print formatted text to an LCD
 *
 * @include PropWare_HD44780/CMakeLists.txt
 */
int main () {
    // Create and initialize our LCD object
    PropWare::HD44780 lcd(FIRST_DATA_PIN, RS, RW, EN, BITMODE, DIMENSIONS);
    lcd.start();

    // Create a printer for easy, formatted writing to the LCD
    PropWare::Printer lcdPrinter(lcd);

    // Print to the LCD (exactly 32 characters so that we fill up both lines)
    lcdPrinter.printf("%u %s%d 0x%07X", 123456789, "Hello!", -12345, 0xabcdef);

    return 0;
}
