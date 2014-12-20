/**
 * @file    fatfs_test.cpp
 *
 * @author  David Zemon
 *
 * Hardware:
 *      SD card connected with the following pins:
 *          - MOSI = P0
 *          - MISO = P1
 *          - SCLK = P2
 *          - CS   = P4
 *      FAT16 or FAT32 Filesystem on the first partition of the SD card
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

#include <PropWare/sd.h>
#include <PropWare/fatfs.h>
#include "PropWareTests.h"

PropWare::BlockStorage *driver;
PropWare::FatFS        *testable;

const PropWare::Pin::Mask MOSI = PropWare::Pin::P0;
const PropWare::Pin::Mask MISO = PropWare::Pin::P1;
const PropWare::Pin::Mask SCLK = PropWare::Pin::P2;
const PropWare::Pin::Mask CS   = PropWare::Pin::P4;

SETUP {
    driver = new PropWare::SD(PropWare::SPI::get_instance(), MOSI, MISO,
                              SCLK, CS);
}

TEARDOWN {
    delete driver;
}

TEST(Constructor) {
    setUp();

    testable = new PropWare::FatFS(driver);

    tearDown();
}

TEST(Mount) {
    setUp();

    testable = new PropWare::FatFS(driver);
    testable->mount();

    tearDown();
}

int main () {
    START(FatFSTest);

    RUN_TEST(Constructor);
    RUN_TEST(Mount);

    COMPLETE();
}
