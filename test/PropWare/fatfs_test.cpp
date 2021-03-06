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
 *      FAT32 Filesystem on the first partition of the SD card
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
#include <PropWare/memory/sd.h>
#include <PropWare/filesystem/fat/fatfs.h>

using PropWare::SD;
using PropWare::FatFS;
using PropWare::SPI;

static SD g_driver;

void error_checker (const PropWare::ErrorCode err) {
    if (SPI::BEG_ERROR <= err && err <= SPI::END_ERROR)
        SPI::get_instance().print_error_str(pwOut, (const SPI::ErrorCode) err);
    else if (SD::BEG_ERROR <= err && err <= SD::END_ERROR)
        g_driver.print_error_str(pwOut, (const SD::ErrorCode) err);
    else if (FatFS::BEG_ERROR <= err && err <= FatFS::END_ERROR)
        pwOut.printf("No print string yet for FatFS's error #%d (raw = %d)\n", err - FatFS::BEG_ERROR, err);
}

class FatFsTest {
    public:
        FatFsTest ()
            : testable(g_driver) {
        }

    public:
        FatFS testable;
};

TEST_F(FatFsTest, ReadMasterBootRecord) {
    PropWare::ErrorCode err;

    err = g_driver.start();
    error_checker(err);
    ASSERT_EQ_MSG(FatFS::NO_ERROR, err);

    uint8_t buffer[g_driver.get_sector_size()];

    err = testable.read_boot_sector(0, buffer);
    error_checker(err);
    ASSERT_EQ_MSG(FatFS::NO_ERROR, err);

    // We're just going to assume the boot sector is not at sector 0
    ASSERT_NEQ_MSG(0, testable.m_initFatInfo.bootSector);
}

TEST_F(FatFsTest, Mount_defaultParameters) {
    PropWare::ErrorCode err;
    uint8_t             buffer[g_driver.get_sector_size()];

    err = testable.mount(buffer);
    error_checker(err);
    ASSERT_EQ_MSG(FatFS::NO_ERROR, err);
    // This test is meant to be run with a FAT32 filesystem
    ASSERT_EQ_MSG(FatFS::FAT_32, testable.m_filesystem);
}

TEST_F(FatFsTest, Mount_partition0) {
    PropWare::ErrorCode err;
    uint8_t             buffer[g_driver.get_sector_size()];

    err = testable.mount(buffer, 0);
    error_checker(err);
    ASSERT_EQ_MSG(FatFS::NO_ERROR, err);
}

TEST_F(FatFsTest, Mount_partition1) {
    PropWare::ErrorCode err;
    uint8_t             buffer[g_driver.get_sector_size()];

    err = testable.mount(buffer, 1);
    error_checker(err);
    ASSERT_EQ_MSG(FatFS::NO_ERROR, err);
}

TEST_F(FatFsTest, Mount_partition4) {
    PropWare::ErrorCode err;
    uint8_t             buffer[g_driver.get_sector_size()];

    err = testable.mount(buffer, 4);
    ASSERT_EQ_MSG(FatFS::UNSUPPORTED_FILESYSTEM, err);
}

TEST_F(FatFsTest, ClearChain) {
    // TODO: Write test (and don't forget to invoke it in main)
}

int main () {
    START(FatFSTest);

    RUN_TEST_F(FatFsTest, ReadMasterBootRecord);
    RUN_TEST_F(FatFsTest, Mount_defaultParameters);
    RUN_TEST_F(FatFsTest, Mount_partition0);
    RUN_TEST_F(FatFsTest, Mount_partition1);
    RUN_TEST_F(FatFsTest, Mount_partition4);

    COMPLETE();
}
