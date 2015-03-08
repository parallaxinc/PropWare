/**
 * @file    FileReader_Demo.cpp
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
#include <PropWare/printer/printer.h>

// 24,964 bytes loaded via propeller-load
#define TEST_PROPWARE

// 24,416 bytes loaded via propeller-load
//#define TEST_SIMPLE

#if (defined TEST_PROPWARE)
#include <PropWare/filesystem/sd.h>
#include <PropWare/filesystem/fat/fatfs.h>
#include <PropWare/filesystem/fat/fatfilereader.h>
#elif (defined TEST_SIMPLE)
#include <simple/simpletools.h>
#endif

using namespace PropWare;

int main () {
#ifdef TEST_PROPWARE
    const SD driver(SPI::get_instance(), Pin::P0, Pin::P1, Pin::P2, Pin::P4);
    FatFS filesystem(&driver);
    filesystem.mount();

    FatFileReader reader(filesystem, "fat_test.txt");
    reader.open();

    while (!reader.eof()) {
        char c = reader.get_char();
        if ('\n' == c)
            pwOut << CRLF;
        else
            pwOut << c;
    }
#elif (defined TEST_SIMPLE)
    sd_mount(1, 2, 0, 4);

    FILE *f = fopen("fat_test.txt", "r");

    while (!feof(f)) {
        int c = fgetc(f);
        if ('\n' == c)
            pwOut << CRLF;
        else
            pwOut << (char) c;
    }
#endif

    return 0;
}
