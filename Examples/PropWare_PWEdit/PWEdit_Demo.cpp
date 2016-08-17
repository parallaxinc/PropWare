/**
 * @file    PWEdit_Demo.cpp
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

#include <PropWare/memory/sd.h>
#include <PropWare/filesystem/fat/fatfs.h>
#include <PropWare/filesystem/fat/fatfilereader.h>
#include <PropWare/hmi/input/scanner.h>
#include <PropWare/hmi/pwedit.h>
#include <PropWare/serial/uart/uartrx.h>

#include <PropWare/hmi/output/hd44780.h>
#include <PropWare/filesystem/fat/fatfilewriter.h>

using namespace PropWare;

const char FILE_NAME[] = "small.txt";

#define error_checker(x) if ((err = (x))) throw_error(err, filesystem, driver);

void throw_error (const PropWare::ErrorCode err, const FatFS &fs, const SD &driver);

int main () {
    PropWare::ErrorCode err;

    HD44780 lcd;
    lcd.start(Pin::P0, Pin::P8, Pin::P9, Pin::P10, HD44780::WIDTH8, HD44780::DIM_20x4);
    Printer lcdPrinter(lcd);

    const SD driver;
    FatFS    filesystem(driver);
    error_checker(filesystem.mount());

    FatFileReader reader(filesystem, FILE_NAME);
    FatFileWriter writer(filesystem, FILE_NAME);
    UARTRX        uartrx;
    Scanner       scanner(uartrx);

    PWEdit editor(reader, writer, scanner, pwOut, &lcdPrinter);
    //PWEdit editor(reader, writer, scanner, lcdPrinter, &pwOut);
    error_checker(editor.run());

    return 0;
}

void throw_error (const PropWare::ErrorCode err, const FatFS &fs, const SD &driver) {
    pwOut << "Error " << err << '\n';

    if (SPI::BEG_ERROR <= err && err <= SPI::END_ERROR)
        SPI::get_instance().print_error_str(pwOut, (const SPI::ErrorCode) err);
    else if (SD::BEG_ERROR <= err && err <= SD::END_ERROR)
        driver.print_error_str(pwOut, (const SD::ErrorCode) err);
    else if (FatFS::BEG_ERROR <= err && err <= FatFS::END_ERROR)
        pwOut.printf("No print string yet for FatFS's error #%d (raw = %d)\n", err - FatFS::BEG_ERROR, err);

    const SimplePort leds(Port::P16, 8);
    leds.set_dir_out();
    while (1) {
        leds.clear();
        waitcnt(100 * MILLISECOND + CNT);
        leds.write((uint32_t) err);
        waitcnt(100 * MILLISECOND + CNT);
    }
}
