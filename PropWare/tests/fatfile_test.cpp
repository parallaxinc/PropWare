/**
 * @file    fatfile_test.cpp
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
#include <PropWare/filesystem/fatfile.h>

using namespace PropWare;

const Pin::Mask MOSI = Pin::P0;
const Pin::Mask MISO = Pin::P1;
const Pin::Mask SCLK = Pin::P2;
const Pin::Mask CS   = Pin::P4;

const char FILE_NAME[] = "fat_test.txt";
FatFS *g_fs;
FatFile *testable;

void error_checker (const ErrorCode err) {
    if (SPI::BEG_ERROR <= err && err <= SPI::END_ERROR)
        SPI::get_instance()->print_error_str(&pwOut, (const SPI::ErrorCode) err);
    else if (SD::BEG_ERROR <= err && err <= SD::END_ERROR)
        ((SD *) g_fs->m_driver)->print_error_str(&pwOut, (const SD::ErrorCode) err);
    else if (FatFS::BEG_ERROR <= err && err <= FatFS::END_ERROR)
        pwOut.printf("No print string yet for FatFS's error #%d (raw = %d)" CRLF, err - FatFS::BEG_ERROR, err);
}

SETUP {
    testable = new FatFile(*g_fs, FILE_NAME, File::READ);
    testable->open();
}

TEARDOWN {
    delete testable;
}

TEST(ConstructorDestructor) {
    testable = new FatFile(*g_fs, FILE_NAME, File::READ);

    ASSERT_EQ_MSG((unsigned int) testable->m_fs, (unsigned int) g_fs);
    ASSERT_EQ_MSG(0, strcmp(FILE_NAME, testable->m_name));
    ASSERT_EQ_MSG(File::READ, testable->m_mode);
    ASSERT_EQ_MSG(false, testable->m_mod);
    ASSERT_EQ_MSG((unsigned int) &pwOut, (unsigned int) testable->m_logger);

    tearDown();
}

TEST(OpenClose) {
    setUp();

    ErrorCode err;

    err = testable->open();
    error_checker(err);
    ASSERT_EQ_MSG(0, err);

    err = testable->close();
    error_checker(err);
    ASSERT_EQ_MSG(0, err);

    tearDown();
}

TEST(Get_char) {
    setUp();

    const char c = testable->get_char();

    tearDown();
}

int main () {
    START(FatFileTest);

    PropWare::ErrorCode err;

    FatFS fs(new SD(SPI::get_instance(), MOSI, MISO, SCLK, CS));
    if ((err = fs.mount())) {
        error_checker(err);
        passed = false;
        COMPLETE();
    }
    g_fs = &fs;

    RUN_TEST(ConstructorDestructor);
    RUN_TEST(OpenClose);

    COMPLETE();
}
