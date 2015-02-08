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
#include <PropWare/filesystem/sd.h>
#include <PropWare/filesystem/fatfs.h>

using namespace PropWare;

FatFS *testable;

const Pin::Mask MOSI = Pin::P0;
const Pin::Mask MISO = Pin::P1;
const Pin::Mask SCLK = Pin::P2;
const Pin::Mask CS   = Pin::P4;

BlockStorage *getDriver () {
    return new SD(SPI::get_instance(), MOSI, MISO, SCLK, CS);
}

void error_checker (const ErrorCode err) {
    if (SPI::BEG_ERROR <= err && err <= SPI::END_ERROR)
        SPI::get_instance()->print_error_str(&pwOut, (const SPI::ErrorCode) err);
    else if (SD::BEG_ERROR <= err && err <= SD::END_ERROR)
        ((SD *) testable->m_driver)->print_error_str(&pwOut, (const SD::ErrorCode) err);
    else if (FatFS::BEG_ERROR <= err && err <= FatFS::END_ERROR)
        pwOut.printf("No print string yet for FatFS's error #%d (raw = %d)" CRLF, err - FatFS::BEG_ERROR, err);
}

SETUP {
    testable = new FatFS(getDriver());
}

TEARDOWN {
    delete testable->m_driver;
    delete testable;
}

TEST(Constructor) {
    setUp();

    tearDown();
}

TEST(ReadMasterBootRecord) {
    setUp();

    ErrorCode err;

    err = testable->m_driver->start();
    error_checker(err);
    ASSERT_EQ_MSG(FatFS::NO_ERROR, err);

    testable->m_buf.buf = (uint8_t *) malloc(testable->m_driver->get_sector_size());

    FatFS::InitFATInfo fatInfo;
    fatInfo.bootSector = 0;
    err = testable->read_boot_sector(fatInfo, 0);
    error_checker(err);
    ASSERT_EQ_MSG(FatFS::NO_ERROR, err);

    // We're just going to assume the boot sector is not at sector 0
    ASSERT_NEQ_MSG(0, fatInfo.bootSector);

    tearDown();
}

TEST(Mount_defaultParameters) {
    setUp();

    ErrorCode err;

    err = testable->mount();
    error_checker(err);
    ASSERT_EQ_MSG(FatFS::NO_ERROR, err);

    tearDown();
}

TEST(Mount_withParameter0) {
    setUp();

    ErrorCode err;

    err = testable->mount(0);
    error_checker(err);
    ASSERT_EQ_MSG(FatFS::NO_ERROR, err);

    tearDown();
}

TEST(Mount_withParameter4) {
    setUp();

    ErrorCode err;

    err = testable->mount(4);
    ASSERT_EQ_MSG(FatFS::INVALID_FILESYSTEM, err);

    tearDown();
}

int main () {
    START(FatFSTest);

    RUN_TEST(Constructor);
    RUN_TEST(ReadMasterBootRecord);
    RUN_TEST(Mount_defaultParameters);
    RUN_TEST(Mount_withParameter0);
    RUN_TEST(Mount_withParameter4);

    COMPLETE();
}
