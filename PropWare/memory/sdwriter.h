/**
 * @file        PropWare/memory/sdwriter.h
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

#include <PropWare/memory/blockstoragewriter.h>
#include <PropWare/memory/sd.h>

namespace PropWare {

class SDWriter : virtual public SD,
                 virtual public BlockStorageWriter {
    public:
        SDWriter (SPI &spi = SPI::get_instance()) : SD(spi) {
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
        SDWriter (SPI &spi, const Port::Mask mosi, const Port::Mask miso, const Port::Mask sclk, const Port::Mask cs)
                : SD(spi, mosi, miso, sclk, cs) {
        }

        virtual PropWare::ErrorCode write_data_block (uint32_t address, const uint8_t dat[]) const {
            PropWare::ErrorCode err;
            uint8_t             temp = 0;

            // Wait until the SD card is no longer busy
            while (!temp)
                temp = (uint8_t) this->m_spi->shift_in(8);

            this->m_cs.clear();
            this->send_command(CMD_WR_BLOCK, address, CRC_OTHER);

            check_errors(this->write_block(SECTOR_SIZE, dat));
            this->m_cs.set();

            return SD::NO_ERROR;
        }

        virtual void write_short (const uint16_t offset, uint8_t buf[], const uint16_t value) const {
            buf[offset + 1] = value >> 8;
            buf[offset]     = value;
        }

        virtual void write_long (const uint16_t offset, uint8_t buf[], const uint32_t value) const {
            buf[offset + 3] = (uint8_t) (value >> 24);
            buf[offset + 2] = (uint8_t) (value >> 16);
            buf[offset + 1] = (uint8_t) (value >> 8);
            buf[offset]     = (uint8_t) value;
        }

    private:
        /**
         * @brief       Write data to SD card via SPI
         *
         * @param[in]   bytes   Block address to read from SD card
         * @param[in]   *dat    Location in memory where data resides
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode write_block (uint16_t bytes, const uint8_t dat[]) const {
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
                // Received "active" response

                // Send data Start ID
                this->m_spi->shift_out(8, DATA_START_ID);

                // Send all bytes
                if (SECTOR_SIZE == bytes)
                    this->m_spi->shift_out_block_msb_first_fast(dat, SECTOR_SIZE);
                else
                    while (bytes--) {
                        this->m_spi->shift_out(8, *(dat++));
                    }

                // Receive and digest response token
                timeout = RESPONSE_TIMEOUT + CNT;
                do {
                    firstByte = (uint8_t) this->m_spi->shift_in(8);

                    // Check for timeout
                    if (abs(timeout - CNT) < SINGLE_BYTE_WIGGLE_ROOM)
                        return READ_TIMEOUT;

                    // wait for transmission end
                } while (0xff == firstByte);
                if (RSPNS_TKN_ACCPT != (firstByte & (uint8_t) RSPNS_TKN_BITS))
                    return INVALID_RESPONSE;
            }

            // After sending the data, provide the device with clocks signals until it has finished writing data
            // internally
            char temp;
            timeout = RESPONSE_TIMEOUT + CNT;
            do {
                temp = (char) this->m_spi->shift_in(8);

                // Check for timeout
                if (abs(timeout - CNT) < SINGLE_BYTE_WIGGLE_ROOM)
                    return READ_TIMEOUT;

                // wait for transmission end
            } while (0xff != temp);

            return SD::NO_ERROR;
        }
};

}
