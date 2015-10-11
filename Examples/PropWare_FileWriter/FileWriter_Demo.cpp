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

#define TEST_PROPWARE

//#define TEST_SIMPLE

#if (defined TEST_PROPWARE)

#include <PropWare/filesystem/sd.h>
#include <PropWare/filesystem/fat/fatfs.h>
#include <PropWare/filesystem/fat/fatfilewriter.h>
#include <PropWare/filesystem/fat/fatfilereader.h>

#elif (defined TEST_SIMPLE)
#include <simple/simpletools.h>
#endif

using namespace PropWare;

#define error_checker(x) err = x; if (err) {pwOut << __FILE__ << ':' << __LINE__ << " Error: " << err << '\n'; return 1;}

int main () {
#ifdef TEST_PROPWARE
    ErrorCode err;
    const SD driver;
    FatFS    filesystem(&driver);

    error_checker(filesystem.mount());

    FatFileReader reader(filesystem, "fat_test.txt");

    FatFileWriter writer(filesystem, "new2.txt");
    if (writer.exists()) {
        pwOut << "File already exists: " << writer.get_name() << '\n';
        pwOut << "Deleting now\n";
        error_checker(writer.remove());
        error_checker(writer.flush());
    }

    error_checker(writer.open());

    while (!reader.eof()) {
        char c;
        error_checker(reader.safe_get_char(c))
        error_checker(writer.safe_put_char(c));
    }

    writer.close();
    filesystem.unmount();
#elif (defined TEST_SIMPLE)
    sd_mount(0, 1, 2, 3);

    FILE *f = fopen("fat_test.txt", "r");

    while (!feof(f))
        pwOut << (char) fgetc(f);
#endif

    pwOut << "All done!\n";
    return 0;
}
