/**
 * @file    fatfilewriter_test.cpp
 *
 * @author  David Zemon
 *
 * Prerequisites:
 *      - SD card connected with the following pins:
 *          - MOSI = P0
 *          - MISO = P1
 *          - SCLK = P2
 *          - CS   = P4
 *      - FAT16 or FAT32 Filesystem on the first partition of the SD card
 *      - File named "fat_test.txt" in this directory should be loaded into the root directory
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
#include <PropWare/filesystem/fat/fatfs.h>
#include <PropWare/filesystem/fat/fatfilewriter.h>

using namespace PropWare;

static const char    FILE_NAME[]       = "fat_test.txt";
static const char    FILE_NAME_UPPER[] = "FAT_TEST.TXT";
static const char    BOGUS_FILE_NAME[] = "bogus.txt";
static FatFS         *g_fs;
static FatFileWriter *testable;

void error_checker (const ErrorCode err) {
    if (SPI::BEG_ERROR <= err && err <= SPI::END_ERROR)
        SPI::get_instance()->print_error_str(&pwOut, (const SPI::ErrorCode) err);
    else if (SD::BEG_ERROR <= err && err <= SD::END_ERROR)
        ((SD *) g_fs->m_driver)->print_error_str(pwOut, (const SD::ErrorCode) err);
    else if (Filesystem::BEG_ERROR <= err && err <= Filesystem::END_ERROR)
        FatFS::print_error_str(pwOut, (const Filesystem::ErrorCode) err);
    else if (FatFS::BEG_ERROR <= err && err <= FatFS::END_ERROR)
        pwOut << "No print string yet for FatFS's error #" << err - FatFS::BEG_ERROR << " (raw = " << err << ")\n";
    else if (err)
        pwOut << "Unknown error: " << err << '\n';
}

void clear_buffer (File *file) {
    BlockStorage::Buffer *buffer = file->m_buf;
    file->m_driver->flush(buffer);
    for (unsigned int i = 0; i < file->m_driver->get_sector_size(); ++i)
        buffer->buf[i] = 0;
    buffer->id = -2;
}

SETUP {
    PropWare::ErrorCode err;
    testable = new FatFileWriter(*g_fs, FILE_NAME);
    err      = testable->open();
    if (err) {
        MESSAGE("Setup failed!");
        error_checker(err);
    }
}

TEARDOWN {
    clear_buffer(testable);
    delete testable;
}

TEST(ConstructorDestructor) {
    // Ensure the requested filename was not all upper case (that wouldn't be a very good test if it were)
    ASSERT_NEQ_MSG(0, strcmp(FILE_NAME, FILE_NAME_UPPER));

    testable = new FatFileWriter(*g_fs, FILE_NAME);

    ASSERT_EQ_MSG(0, strcmp(FILE_NAME_UPPER, testable->get_name()));
    ASSERT_EQ_MSG((unsigned int) &pwOut, (unsigned int) testable->m_logger);
    ASSERT_EQ_MSG((unsigned int) g_fs->get_driver(), (unsigned int) testable->m_driver);
    ASSERT_EQ_MSG((unsigned int) &g_fs->m_buf, (unsigned int) testable->m_buf);
    ASSERT_EQ_MSG((unsigned int) testable->m_fs, (unsigned int) g_fs);
    ASSERT_EQ_MSG(-1, testable->get_length());
    ASSERT_EQ_MSG(false, testable->m_mod);

    tearDown();
}

TEST(Exists_doeesNotExist) {
    testable = new FatFileWriter(*g_fs, BOGUS_FILE_NAME);
    ASSERT_FALSE(testable->exists());
    tearDown();
}

TEST(Exists_doesExist) {
    testable = new FatFileWriter(*g_fs, FILE_NAME);

    PropWare::ErrorCode err;
    const bool exists = testable->exists(err);
    error_checker(err);
    ASSERT_TRUE(exists)
    tearDown();
}

TEST(OpenClose_ExistingFile) {
    ErrorCode err;

    testable = new FatFileWriter(*g_fs, FILE_NAME);

    if ((err = testable->open()))
        error_checker(err);
    ASSERT_EQ_MSG(0, err);

    ASSERT_NEQ_MSG(0, testable->get_length());

    if ((err = testable->close()));
    error_checker(err);
    ASSERT_EQ_MSG(0, err);

    tearDown();
}

TEST(OpenCloseDelete_NonExistingFile) {
    ErrorCode err;

    testable = new FatFileWriter(*g_fs, BOGUS_FILE_NAME);

    ASSERT_FALSE(testable->exists());

    if ((err = testable->open()))
        error_checker(err);
    ASSERT_EQ_MSG(0, err);

    ASSERT_EQ_MSG(0, testable->get_length());



    tearDown();
}

int main () {
    PropWare::ErrorCode err;

    START(FatFileReaderTest);

    g_fs = new FatFS(new SD());
    if ((err = g_fs->mount())) {
        error_checker(err);
        passed = false;
        COMPLETE();
    }

    RUN_TEST(ConstructorDestructor);
    RUN_TEST(Exists_doeesNotExist);
    RUN_TEST(Exists_doesExist);
    RUN_TEST(OpenClose_ExistingFile);
//    RUN_TEST(OpenCloseDelete_NonExistingFile);

    delete g_fs->get_driver();
    delete g_fs;

    COMPLETE();
}
