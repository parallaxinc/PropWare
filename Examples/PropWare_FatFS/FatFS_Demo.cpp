/**
 * @file    FatFS_Demo.cpp
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
#include <PropWare/filesystem/fatfs.h>
#include <PropWare/filesystem/sd.h>
#include <simple/simpletools.h>

using namespace PropWare;

int main () {
    const SD driver(SPI::get_instance(), Pin::P0, Pin::P1, Pin::P2, Pin::P4);
    FatFS filesystem(&driver);

    filesystem.mount();

//    sd_mount(1, 2, 0, 4);
//
//    DIR *dir = opendir("/");
//    pwOut.printf("Dir = %08X" CRLF, (unsigned int) dir);
//
//    dirent *ent;
//    while ((ent = readdir(dir)))
//        pwOut.printf("%s" CRLF, ent->d_name);

    return 0;
}
