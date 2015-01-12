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

#include "PropWareTests.h"
#include <PropWare/sd.h>
#include <PropWare/fatfs.h>

using namespace PropWare;

FatFS        *testable;

const Pin::Mask MOSI = Pin::P0;
const Pin::Mask MISO = Pin::P1;
const Pin::Mask SCLK = Pin::P2;
const Pin::Mask CS   = Pin::P4;

BlockStorage* getDriver() {
    return new SD(SPI::get_instance(), MOSI, MISO, SCLK, CS);
}

SETUP {
    testable = new FatFS(getDriver());
    testable->mount();
}

TEARDOWN {
    delete testable->m_driver;
    delete testable;
}

TEST(Constructor) {
    testable = new FatFS(getDriver());

    tearDown();
}

TEST(ReadBootSector) {
    testable = new FatFS(getDriver());
    ASSERT_EQ_MSG(FatFS::NO_ERROR, testable->m_driver->start());

    testable->m_buf.buf = (uint8_t *) malloc(testable->m_driver->get_sector_size());

    FatFS::InitFATInfo fatInfo;
    fatInfo.bootSector = 0;
    ASSERT_EQ_MSG(FatFS::NO_ERROR, testable->read_boot_sector(&fatInfo));

    // We're just going to assume the boot sector is not at sector 0
    ASSERT_NEQ_MSG(0, fatInfo.bootSector);

    tearDown();
}

TEST(Mount) {
    setUp();

    ASSERT_EQ_MSG(FatFS::NO_ERROR, testable->mount());

    tearDown();
}

int main () {
    START(FatFSTest);

    RUN_TEST(Constructor);
    RUN_TEST(ReadBootSector);
    RUN_TEST(Mount);

    COMPLETE();
}
