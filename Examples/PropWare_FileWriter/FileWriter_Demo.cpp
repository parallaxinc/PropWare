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

#include <PropWare/printer/printer.h>

#define TEST_PROPWARE
//#define TEST_SIMPLE

#if (defined TEST_PROPWARE)
#include <PropWare/filesystem/sd.h>
#include <PropWare/filesystem/fat/fatfs.h>
#include <PropWare/filesystem/fat/fatfilewriter.h>
#include <PropWare/filesystem/fat/fatfilereader.h>
using namespace PropWare;
#elif (defined TEST_SIMPLE)
#include <simple/simpletools.h>
#endif

void run_test();

int main() {
    volatile unsigned int startTime = CNT;
    run_test();
    volatile unsigned int runtime = PropWare::Utility::measure_time_interval(startTime);
    pwOut << "All done! " << runtime / 1000 << "ms\n";

    return 0;
}

#ifdef TEST_PROPWARE
void run_test() {
    const SD  driver;
    FatFS     filesystem(&driver);

    SD::Buffer   writeBuffer;
    SD::MetaData writeMetaData;
    uint8_t bufferData[driver.get_sector_size()];
    writeBuffer.buf  = bufferData;
    writeBuffer.meta = &writeMetaData;

    filesystem.mount();

    FatFileReader reader(filesystem, "fat_test.txt");
    FatFileWriter writer(filesystem, "new2.txt", &writeBuffer);

    reader.open();
    writer.open();

    while (!reader.eof())
        writer.put_char(reader.get_char());

    writer.close();
    reader.close();
    filesystem.unmount();
}
#else
void run_test() {
    sd_mount(0, 1, 2, 3);

    FILE *reader = fopen("fat_test.txt", "r");
    FILE *writer = fopen("new2.txt", "w");

    int c;
    while (EOF != (c = fgetc(reader)))
        fputc((char) c, writer);

    fclose(reader);
    fclose(writer);
}
#endif
