/**
 * @file        PropWare/memory/sdreader.h
 *
 * @author      David Zemon
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

#pragma once

#include <PropWare/memory/sd.h>
#include <PropWare/memory/blockstoragereader.h>

namespace PropWare {

class SDReader : virtual public SD,
                 virtual public BlockStorageReader {
    public:
        SDReader (SPI &spi = SPI::get_instance()) : SD(spi) {
        }

        /**
         * @brief       Construct an SD object with the given SPI parameters
         *
         * @param[in]   *spi    Address of a SPI instance. Its clock frequency and mode will be modified to fit the SD
         *                      card's needs
         * @param[in]   mosi    Pin mask for data line leaving the Propeller
         * @param[in]   miso    Pin mask for data line going in to the Propeller
         * @param[in]   sclk    Pin mask for clock line
         * @param[in]   cs      Pin mask for chip select
         */
        SDReader (SPI &spi, const Port::Mask mosi, const Port::Mask miso, const Port::Mask sclk, const Port::Mask cs)
                : SD(spi, mosi, miso, sclk, cs) {
        }

        virtual PropWare::ErrorCode read_data_block (const uint32_t address, uint8_t buf[]) const {
            PropWare::ErrorCode err;
            uint8_t             temp = 0;

            // Wait until the SD card is no longer busy
            while (!temp)
                temp = (uint8_t)
                        this->m_spi->shift_in(8);

            /**
             * Special error handling is needed to ensure that, if an error is thrown, chip select is set high again
             * before returning the error
             */
            this->m_cs.clear();
            this->send_command(CMD_RD_BLOCK, address, CRC_OTHER);
            err = this->read_block(SECTOR_SIZE, buf);
            this->m_cs.set();

            return err;
        }

        virtual uint16_t get_short (const uint16_t offset, const uint8_t buf[]) const {
            return (buf[offset + 1] << 8) + buf[offset];
        }

        virtual uint32_t get_long (const uint16_t offset, const uint8_t buf[]) const {
            return (buf[offset + 3] << 24) + (buf[offset + 2] << 16) + (buf[offset + 1] << 8) + buf[offset];
        }

    private:
        /**
         * @brief       Receive data from SD card via SPI
         *
         * @param[in]   bytes   Number of bytes to receive
         * @param[out]  dat[]   Location in memory with enough space to store `bytes` bytes of data
         *
         * @pre         Chip select must be activated prior to invocation
         *
         * @return      Returns 0 for success, else error code
         */
        PropWare::ErrorCode read_block (uint16_t bytes, uint8_t dat[]) const {
            uint8_t  checksum;
            uint32_t timeout;
            uint8_t  firstByte;

            // Read first byte - the R1 response
            timeout = RESPONSE_TIMEOUT + CNT;
            do {
                firstByte = (uint8_t) this->m_spi->shift_in(8);

                // Check for timeout
                if (abs(timeout - CNT) < SINGLE_BYTE_WIGGLE_ROOM)
                    return READ_TIMEOUT;

                // wait for transmission end
            } while (0xff == firstByte);

            // Ensure this response is "active"
            if (RESPONSE_ACTIVE == firstByte) {
                // Ignore blank data again
                timeout = RESPONSE_TIMEOUT + CNT;
                do {
                    dat[0] = (uint8_t)
                            this->m_spi->shift_in(8);

                    // Check for timeout
                    if (abs(timeout - CNT) < SINGLE_BYTE_WIGGLE_ROOM)
                        return READ_TIMEOUT;

                    // wait for transmission end
                } while (DATA_START_ID != dat[0]);

                // Check for the data start identifier and continue reading data
                if (DATA_START_ID == *dat) {
                    // Read in requested data bytes
                    if (SECTOR_SIZE == bytes)
                        this->m_spi->shift_in_block_mode0_msb_first_fast(dat, SECTOR_SIZE);
                    else
                        while (bytes--) {
                            *dat++ = (uint8_t) this->m_spi->shift_in(8);
                        }

                    // Continue reading bytes until you get something that isn't 0xff - it should be the checksum.
                    timeout = RESPONSE_TIMEOUT + CNT;
                    do {
                        checksum = (uint8_t)
                                this->m_spi->shift_in(8);

                        // Check for timeout
                        if ((timeout - CNT) < SINGLE_BYTE_WIGGLE_ROOM)
                            return READ_TIMEOUT;

                        // wait for transmission end
                    } while (0xff == checksum);

                    // The checksum is actually 2 bytes, not 1, so sending a total of 16 high bits takes care of the
                    // second checksum byte as well as an extra byte for good measure
                    this->m_spi->shift_out(16, 0xffff);
                } else {
                    return INVALID_DAT_START_ID;
                }
            } else
                return INVALID_RESPONSE;

            return SD::NO_ERROR;
        }
};

}
