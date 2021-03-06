/**
 * @file    sd_test.cpp
 *
 * @author  David Zemon
 *
 * Hardware:
 *      SD card connected with the following pins:
 *          - MOSI = P0
 *          - MISO = P1
 *          - SCLK = P2
 *          - CS   = P4
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

using PropWare::SD;
using PropWare::SPI;
using PropWare::Port;

class SdTest {
    public:
        void sd_error_checker (const PropWare::ErrorCode err) {
            if (err)
                this->testable.print_error_str(pwOut, (SD::ErrorCode) err);
        }

    public:
        SD testable;
};

TEST_F(SdTest, DefaultConstructor_RELIES_ON_DNA_BOARD) {
    ASSERT_EQ_MSG((unsigned int) &SPI::get_instance(), (unsigned int) testable.m_spi);
    ASSERT_EQ_MSG((unsigned int) Port::Mask::P2, (unsigned int) testable.m_spi->m_mosi.get_mask());
    ASSERT_EQ_MSG((unsigned int) Port::Mask::P1, (unsigned int) testable.m_spi->m_sclk.get_mask());
    ASSERT_EQ_MSG((unsigned int) Port::Mask::P0, (unsigned int) testable.m_spi->m_miso.get_mask());
    ASSERT_EQ_MSG((unsigned int) Port::Mask::P3, (unsigned int) testable.m_cs.get_mask());
}

TEST_F(SdTest, Start) {
    PropWare::ErrorCode err = testable.start();
    sd_error_checker(err);
    ASSERT_EQ_MSG(SD::NO_ERROR, err);
}

TEST_F(SdTest, ReadDataBlock) {
    uint8_t buffer[SD::SECTOR_SIZE];

    PropWare::ErrorCode err = testable.start();
    sd_error_checker(err);
    ASSERT_EQ_MSG(0, err);

    // Initialize all values of the buffer to 0. Surely the first sector of the SD card won't be _all_ zeros!
    memset(buffer, 0, sizeof(buffer));

    // Read in a block...
    err = testable.read_data_block(0, buffer);
    sd_error_checker(err);
    ASSERT_EQ_MSG(SD::NO_ERROR, err);

    // And make sure at least _one_ of the bytes is non-zero
    bool              success = false;
    for (unsigned int j       = 0; j < sizeof(buffer); ++j)
        if (buffer[j])
            success = true;

    ASSERT_TRUE(success);
}

TEST_F(SdTest, WriteDataBlock) {
    uint8_t       originalBlock[SD::SECTOR_SIZE];
    uint8_t       moddedBlock[SD::SECTOR_SIZE];
    const uint8_t *myData     = 0;
    const uint8_t sdBlockAddr = 0;

    PropWare::ErrorCode err = testable.start();
    sd_error_checker(err);
    ASSERT_EQ_MSG(0, err);

    // Read in a block...
    err = testable.read_data_block(sdBlockAddr, originalBlock);
    sd_error_checker(err);
    ASSERT_EQ_MSG(SD::NO_ERROR, err);
    MESSAGE("WriteBlock: Original block read in");

    // Try writing a random block of memory
    err = testable.write_data_block(sdBlockAddr, myData);
    sd_error_checker(err);
    ASSERT_EQ_MSG(SD::NO_ERROR, err);
    MESSAGE("WriteBlock: Random block written");

    // Read the block back in to a new buffer. Make sure it matches the data written.
    err = testable.read_data_block(sdBlockAddr, moddedBlock);
    sd_error_checker(err);
    ASSERT_EQ_MSG(SD::NO_ERROR, err);
    MESSAGE("WriteBlock: Modded block read");
    ASSERT_EQ_MSG(0, memcmp(myData, moddedBlock, SD::SECTOR_SIZE));
    MESSAGE("WriteBlock: Modded block equals random block");

    // Write the original block back to the SD card
    err = testable.write_data_block(sdBlockAddr, originalBlock);
    sd_error_checker(err);
    ASSERT_EQ_MSG(SD::NO_ERROR, err);
    MESSAGE("WriteBlock: Original block written back");

    // Read the block back in to a new buffer. Make sure it matches the data written.
    err = testable.read_data_block(sdBlockAddr, moddedBlock);
    sd_error_checker(err);
    ASSERT_EQ_MSG(SD::NO_ERROR, err);
    MESSAGE("WriteBlock: Modded block read again");
    ASSERT_EQ_MSG(0, memcmp(originalBlock, moddedBlock, SD::SECTOR_SIZE));
    MESSAGE("WriteBlock: Modded block matches original");
}

int main () {
    START(SDTest);

    RUN_TEST_F(SdTest, DefaultConstructor_RELIES_ON_DNA_BOARD);
    RUN_TEST_F(SdTest, Start);
    RUN_TEST_F(SdTest, ReadDataBlock);
    RUN_TEST_F(SdTest, WriteDataBlock);

    COMPLETE();
}
