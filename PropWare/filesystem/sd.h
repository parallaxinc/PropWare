/**
 * @file        sd.h
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

#include <stdlib.h>
#include <string.h>
#include <PropWare/PropWare.h>
#include <PropWare/filesystem/blockstorage.h>
#include <PropWare/spi.h>
#include <PropWare/pin.h>
#include <PropWare/printer/printer.h>

namespace PropWare {

class SD: public BlockStorage {
    public:
        static const uint16_t SECTOR_SIZE       = 512;
        static const uint8_t  SECTOR_SIZE_SHIFT = 9;
        /** Default frequency to run the SPI module */
        static const uint32_t FULL_SPEED_SPI    = 900000;

    public:
        /**
         * Error codes - preceded by SPI
         */
        typedef enum {
            /** No error */           NO_ERROR    = 0,
            /** First SD error code */BEG_ERROR   = SPI::END_ERROR + 1,
            /** SD Error 0 */         INVALID_CMD = BEG_ERROR,
            /** SD Error 1 */         READ_TIMEOUT,
            /** SD Error 2 */         INVALID_NUM_BYTES,
            /** SD Error 3 */         INVALID_RESPONSE,
            /** SD Error 4 */         INVALID_INIT,
            /** SD Error 5 */         INVALID_DAT_START_ID,
            /** SD Error 6 */         CMD8_FAILURE,
            /** Last SD error code */ END_ERROR   = CMD8_FAILURE
        } ErrorCode;

    public:
        /**
         * @brief       Construct an SD object with the given SPI parameters
         */
        SD (SPI *spi, const Port::Mask mosi, const Port::Mask miso, const Port::Mask sclk, const Port::Mask cs) {
            this->m_mosi = mosi;
            this->m_miso = miso;
            this->m_sclk = sclk;

            // Set CS for output and initialize high
            this->m_cs.set_mask(cs);
            this->m_cs.set_dir(Pin::OUT);
            this->m_cs.set();

            this->m_spi = spi;
        }

        /**
         * @brief       Initialize SD card communication over SPI for 3.3V
         *              configuration
         *
         * Starts an SPI cog IFF an SPI cog has not already been started; If
         * one has been started, only the cs will have effect
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode start () const {
            PropWare::ErrorCode err;
            uint8_t             response[16];

            // Start SPI module
            if ((err = this->m_spi->start(this->m_mosi, this->m_miso, this->m_sclk, SPI_INIT_FREQ, SPI_MODE,
                                          SPI_BITMODE)))
                return err;

            // Try and get the card up and responding to commands first
            check_errors(this->reset_and_verify_v2_0(response));

            check_errors(this->activate(response));

            check_errors(this->increase_throttle());

            // We're finally done initializing everything. Set chip select high
            // again to release the SPI port
            this->m_cs.set();

            // Initialization complete
            return NO_ERROR;
        }

        uint16_t get_sector_size () const {
            return SECTOR_SIZE;
        }

        uint8_t get_sector_size_shift () const {
            return SECTOR_SIZE_SHIFT;
        }

        /**
         * @brief   Create a human-readable error string
         *
         * @param[in]   err     Error number used to determine error string
         */
        void print_error_str (const Printer &printer, const ErrorCode err) const {
            const uint8_t relativeError = err - BEG_ERROR;

            switch (err) {
                case INVALID_CMD:
                    printer <<  "SD Error " << relativeError << ": Invalid command\n";
                    break;
                case READ_TIMEOUT:
                    printer <<  "SD Error " << relativeError << ": Timed out during read\n";
                    break;
                case INVALID_NUM_BYTES:
                    printer <<  "SD Error " << relativeError << ": Invalid number of bytes\n";
                    break;
                case INVALID_RESPONSE:
                    printer <<  "SD Error " << relativeError << ": Invalid first-byte response\n";
                    printer << "\tReceived: " << this->m_firstByteResponse << '\n';
                    this->first_byte_expansion(printer);
                    break;
                case INVALID_INIT:
                    printer <<  "SD Error " << relativeError << ": Invalid response during initialization\n";
                    printer << "\tResponse: " << this->m_firstByteResponse << '\n';
                    break;
                case INVALID_DAT_START_ID:
                    printer <<  "SD Error " << relativeError << ": Invalid data-start ID\n";
                    printer << "\tReceived: " << this->m_firstByteResponse << '\n';
                    break;
                default:
                    return;
            }
        }

        /**
         * @brief       Read SD_SECTOR_SIZE-byte data block from SD card
         *
         * @param[in]   address     Number of bytes to send
         * @param[out]  dat[]       Location in chip memory to store data block;
         *                          If NULL is sent, the default internal buffer
         *                          will be used
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode read_data_block (const uint32_t address, uint8_t buf[]) const {
            PropWare::ErrorCode err;
            uint8_t             temp = 0;

            // Wait until the SD card is no longer busy
            while (!temp)
                this->m_spi->shift_in(8, &temp);

            /**
             * Special error handling is needed to ensure that, if an error is thrown, chip select is set high again
             * before returning the error
             */
            this->m_cs.clear();
            err     = this->send_command(CMD_RD_BLOCK, address, CRC_OTHER);
            if (!err)
                err = this->read_block(SECTOR_SIZE, buf);
            this->m_cs.set();

            return err;
        }

        /**
         * @brief       Write SD_SECTOR_SIZE-byte data block to SD card
         *
         * @param[in]   address     Block address to write to SD card
         * @param[in]   *dat        Location in chip memory to read data block
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode write_data_block (uint32_t address, const uint8_t dat[]) const {
            PropWare::ErrorCode err;
            uint8_t             temp = 0;

            // Wait until the SD card is no longer busy
            while (!temp)
                this->m_spi->shift_in(8, &temp);

            this->m_cs.clear();
            check_errors(this->send_command(CMD_WR_BLOCK, address, CRC_OTHER));

            check_errors(this->write_block(SECTOR_SIZE, dat));
            this->m_cs.set();

            return NO_ERROR;
        }

        uint16_t get_short (const uint16_t offset, const uint8_t buf[]) const {
            return (buf[offset + 1] << 8) + buf[offset];
        }

        uint32_t get_long (const uint16_t offset, const uint8_t buf[]) const {
            return (buf[offset + 3] << 24) + (buf[offset + 2] << 16) + (buf[offset + 1] << 8) + buf[offset];
        }

        void write_short (const uint16_t offset, uint8_t buf[], const uint16_t value) const {
            buf[offset + 1] = value >> 8;
            buf[offset] = value;
        }

        void write_long (const uint16_t offset, uint8_t buf[], const uint32_t value) const {
            buf[offset + 3] = value >> 24;
            buf[offset + 2] = value >> 16;
            buf[offset + 1] = value >> 8;
            buf[offset] = value;
        }

#ifdef SD_OPTION_FILE_WRITE
        /**
         * @brief       Write a byte-reversed 16-bit variable (SD cards store
         *              bytes little-endian therefore we must reverse them to
         *              use multi-byte variables)
         *
         * @param[out]  buf[]   Address to store the first byte of data
         * @param[in]   dat     Normal, 16-bit variable to be written to RAM in
         *                      reverse endian
         */
        void write_rev_dat16 (uint8_t buf[], const uint16_t dat) {
            buf[1] = (uint8_t) (dat >> 8);
            buf[0] = (uint8_t) dat;
        }

        /**
         * @brief       Write a byte-reversed 32-bit variable (SD cards store
         *              bytes little-endian therefore we must reverse them to
         *              use multi-byte variables)
         *
         * @param[out]  buf[]   Address to store the first byte of data
         * @param[in]   dat     Normal, 32-bit variable to be written to RAM in
         *                      reverse endian
         */
        void write_rev_dat32 (uint8_t buf[], const uint32_t dat) {
            buf[3] = (uint8_t) (dat >> 24);
            buf[2] = (uint8_t) (dat >> 16);
            buf[1] = (uint8_t) (dat >> 8);
            buf[0] = (uint8_t) dat;
        }
#endif

    private:
        /***********************
         *** Private Methods ***
         ***********************/
        inline PropWare::ErrorCode reset_and_verify_v2_0 (uint8_t response[]) const {
            PropWare::ErrorCode err;
            uint8_t             i, j;
            bool                stageCleared;

            // Attempt initialization no more than 10 times
            stageCleared = false;
            for (i = 0; i < 10 && !stageCleared; ++i) {
                // Initialization loop (reset SD card)
                for (j = 0; j < 10 && !stageCleared; ++j) {
                    check_errors(this->power_up());

                    check_errors(this->reset(response, stageCleared));
                }

                // If we couldn't go idle after 10 tries, give up
                if (!stageCleared)
                    return INVALID_INIT;

                stageCleared = false; // Reset stageCleared for the next mini-stage
                check_errors(this->verify_v2_0(response, &stageCleared));
            }

            // If CMD8 never succeeded, throw an error
            if (!stageCleared)
                return CMD8_FAILURE;

            // The card is idle, that's good. Let's make sure we get the correct response back
            if ((HOST_VOLTAGE_3V3 != response[2]) || (R7_CHECK_PATTERN != response[3]))
                return CMD8_FAILURE;

            return NO_ERROR;
        }

        /**
         * @brief   Send numerous clocks to the card to allow it to perform internal initialization
         */
        inline PropWare::ErrorCode power_up () const {
            uint8_t             i;
            PropWare::ErrorCode err;

            waitcnt(CLKFREQ / 10 + CNT);

            // Send at least 72 clock cycles to enable the SD card
            this->m_cs.set();
            for (i = 0; i < 128; ++i)
                check_errors(this->m_spi->shift_out(16, (uint32_t) -1));

            // Be very super 100% sure that all clocks have finished ticking
            // before setting chip select low
            check_errors(this->m_spi->wait());
            waitcnt(10 * MILLISECOND + CNT);

            // Chip select goes low for the duration of this function
            this->m_cs.clear();

            return NO_ERROR;
        }

        inline PropWare::ErrorCode reset (uint8_t response[], bool &isIdle) const {
            PropWare::ErrorCode err;

            // Send SD into idle state, retrieve a response and ensure it is the
            // "idle" response
            check_errors(this->send_command(CMD_IDLE, 0, CRC_IDLE));
            this->get_response(RESPONSE_LEN_R1, response);

            // Check if idle
            if (RESPONSE_IDLE == this->m_firstByteResponse)
                isIdle = true;

            return NO_ERROR;
        }

        inline PropWare::ErrorCode verify_v2_0 (uint8_t response[], bool *stageCleared) const {
            PropWare::ErrorCode err;

            // Inform SD card that the Propeller uses the 2.7-3.6V range;
            check_errors(this->send_command(CMD_INTERFACE_COND, ARG_CMD8, CRC_CMD8));
            check_errors(this->get_response(RESPONSE_LEN_R7, response));
            if (RESPONSE_IDLE == this->m_firstByteResponse)
                *stageCleared = true;

            return NO_ERROR;
        }

        inline PropWare::ErrorCode activate (uint8_t response[]) const {
            PropWare::ErrorCode err;
            uint32_t            timeout;
            uint32_t            longWiggleRoom = 3 * MILLISECOND;
            bool                stageCleared   = false;

            // Attempt to send active
            timeout = SEND_ACTIVE_TIMEOUT + CNT;  //
            do {
                // Send the application-specific pre-command
                check_errors(this->send_command(CMD_APP, 0, CRC_ACMD_PREP));
                check_errors(this->get_response(RESPONSE_LEN_R1, response));

                // Request that the SD card go active!
                check_errors(this->send_command(CMD_WR_OP, BIT_30, 0));
                check_errors(this->get_response(RESPONSE_LEN_R1, response));

                // If the card ACKed with the active state, we're all good!
                if (RESPONSE_ACTIVE == this->m_firstByteResponse)
                    stageCleared = true;

                // Check for timeout
                if (abs(timeout - CNT) < longWiggleRoom)
                    return READ_TIMEOUT;

                // Wait until we have received the active response
            } while (!stageCleared);

            return NO_ERROR;
        }

        /**
         * @brief   Initialization nearly complete, increase clock speed
         */
        inline PropWare::ErrorCode increase_throttle () const {
            return this->m_spi->set_clock(FULL_SPEED_SPI);
        }

        /**
         * @brief       Send a command and argument over SPI to the SD card
         *
         * @param[in]   cmd     6-bit value representing the command sent to the
         *                      SD card
         * @param[in]   arg     Any argument applicable to the command
         * @param[in]   crc     CRC for the command and argument
         *
         * @return      Returns 0 for success, else error code
         */
        PropWare::ErrorCode send_command (const uint8_t cmd, const uint32_t arg, const uint8_t crc) const {
            PropWare::ErrorCode err;

            // Send out the command
            check_errors(this->m_spi->shift_out(8, cmd));

            // Send argument
            check_errors(this->m_spi->shift_out(16, (arg >> 16)));
            check_errors(this->m_spi->shift_out(16, arg & WORD_0));

            // Send sixth byte - CRC
            check_errors(this->m_spi->shift_out(8, crc));

            return NO_ERROR;
        }

        /**
         * @brief       receive response and data from SD card over SPI
         *
         * @param[in]   numBytes    Number of bytes to receive
         * @param[out]  *dat        Location in memory with enough space to store `bytes` bytes of data
         *
         * @pre         Chip select must be activated prior to invocation
         *
         * @return      Returns 0 for success, else error code
         */
        PropWare::ErrorCode get_response (uint8_t numBytes, uint8_t *dat) const {
            PropWare::ErrorCode err;
            uint32_t            timeout;

            // Read first byte - the R1 response
            timeout = RESPONSE_TIMEOUT + CNT;
            do {
                check_errors(this->m_spi->shift_in(8, (uint8_t *) &this->m_firstByteResponse));

                // Check for timeout
                if (abs(timeout - CNT) < SINGLE_BYTE_WIGGLE_ROOM)
                    return READ_TIMEOUT;

                // wait for transmission end
            } while (0xff == this->m_firstByteResponse);

            // First byte in a response should always be either IDLE or ACTIVE.
            // If this one wasn't, throw an error. If it was, decrement the
            // bytes counter and read in all remaining bytes
            if ((RESPONSE_IDLE == this->m_firstByteResponse) || (RESPONSE_ACTIVE == this->m_firstByteResponse)) {
                --numBytes;    // Decrement bytes counter

                // Read remaining bytes
                while (numBytes--)
                    check_errors(this->m_spi->shift_in(8, dat++));
            } else
                return INVALID_RESPONSE;

            // Responses should always be followed up by outputting 8 clocks
            // with MOSI high
            check_errors(this->m_spi->shift_out(16, (uint32_t) -1));
            check_errors(this->m_spi->shift_out(16, (uint32_t) -1));
            check_errors(this->m_spi->shift_out(16, (uint32_t) -1));
            check_errors(this->m_spi->shift_out(16, (uint32_t) -1));

            return NO_ERROR;
        }

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
            uint8_t  i, err, checksum;
            uint32_t timeout;

            // Read first byte - the R1 response
            timeout = RESPONSE_TIMEOUT + CNT;
            do {
                check_errors(this->m_spi->shift_in(8, (uint8_t *) &this->m_firstByteResponse));

                // Check for timeout
                if (abs(timeout - CNT) < SINGLE_BYTE_WIGGLE_ROOM)
                    return READ_TIMEOUT;

                // wait for transmission end
            } while (0xff == this->m_firstByteResponse);

            // Ensure this response is "active"
            if (RESPONSE_ACTIVE == this->m_firstByteResponse) {
                // Ignore blank data again
                timeout = RESPONSE_TIMEOUT + CNT;
                do {
                    check_errors(this->m_spi->shift_in(8, dat));

                    // Check for timeout
                    if (abs(timeout - CNT) < SINGLE_BYTE_WIGGLE_ROOM)
                        return READ_TIMEOUT;

                    // wait for transmission end
                } while (DATA_START_ID != *dat);

                // Check for the data start identifier and continue reading data
                if (DATA_START_ID == *dat) {
                    // Read in requested data bytes
#if (defined SPI_FAST_SECTOR)
                    if (SECTOR_SIZE == bytes) {
                        this->m_spi->shift_in_sector(dat, 1);
                        bytes = 0;
                    }
#else
                    while (bytes--) {
                        check_errors(this->m_spi->shift_in_fast(8, dat++));
                    }
#endif
                    // Read two more bytes for checksum - throw away data
                    for (i = 0; i < 2; ++i) {
                        timeout = RESPONSE_TIMEOUT + CNT;
                        do {
                            check_errors(this->m_spi->shift_in(8, &checksum));

                            // Check for timeout
                            if ((timeout - CNT) < SINGLE_BYTE_WIGGLE_ROOM)
                                return READ_TIMEOUT;

                            // wait for transmission end
                        } while (0xff == checksum);
                    }

                    // Send final 0xff
                    check_errors(this->m_spi->shift_out(8, 0xff));
                } else {
                    return INVALID_DAT_START_ID;
                }
            } else
                return INVALID_RESPONSE;

            return NO_ERROR;
        }

        /**
         * @brief       Write data to SD card via SPI
         *
         * @param[in]   bytes   Block address to read from SD card
         * @param[in]   *dat    Location in memory where data resides
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode write_block (uint16_t bytes, const uint8_t dat[]) const {
            PropWare::ErrorCode err;
            uint32_t            timeout;

            // Read first byte - the R1 response
            timeout = RESPONSE_TIMEOUT + CNT;
            do {
                check_errors(this->m_spi->shift_in(8, (uint8_t *) &this->m_firstByteResponse));

                // Check for timeout
                if (abs(timeout - CNT) < SINGLE_BYTE_WIGGLE_ROOM)
                    return READ_TIMEOUT;

                // wait for transmission end
            } while (0xff == this->m_firstByteResponse);

            // Ensure this response is "active"
            if (RESPONSE_ACTIVE == this->m_firstByteResponse) {
                // Received "active" response

                // Send data Start ID
                check_errors(this->m_spi->shift_out(8, DATA_START_ID));

                // Send all bytes
                while (bytes--) {
                    check_errors(this->m_spi->shift_out_fast(8, *(dat++)));
                }

                // Receive and digest response token
                timeout = RESPONSE_TIMEOUT + CNT;
                do {
                    check_errors(this->m_spi->shift_in(8, (uint8_t *) &this->m_firstByteResponse));

                    // Check for timeout
                    if (abs(timeout - CNT) < SINGLE_BYTE_WIGGLE_ROOM)
                        return READ_TIMEOUT;

                    // wait for transmission end
                } while (0xff == this->m_firstByteResponse);
                if (RSPNS_TKN_ACCPT != (this->m_firstByteResponse & (uint8_t) RSPNS_TKN_BITS))
                    return INVALID_RESPONSE;
            }

            // After sending the data, provide the device with clocks signals until it has finished writing data
            // internally
            char temp;
            timeout = RESPONSE_TIMEOUT + CNT;
            do {
                check_errors(this->m_spi->shift_in(8, &temp));

                // Check for timeout
                if (abs(timeout - CNT) < SINGLE_BYTE_WIGGLE_ROOM)
                    return READ_TIMEOUT;

                // wait for transmission end
            } while (0xff != temp);

            return NO_ERROR;
        }

        void first_byte_expansion(const Printer &printer) const {
            if (BIT_0 & this->m_firstByteResponse)
                printer.puts("\t0: Idle\n");
            if (BIT_1 & this->m_firstByteResponse)
                printer.puts("\t1: Erase reset\n");
            if (BIT_2 & this->m_firstByteResponse)
                printer.puts("\t2: Illegal command\n");
            if (BIT_3 & this->m_firstByteResponse)
                printer.puts("\t3: Communication CRC error\n");
            if (BIT_4 & this->m_firstByteResponse)
                printer.puts("\t4: Erase sequence error\n");
            if (BIT_5 & this->m_firstByteResponse)
                printer.puts("\t5: Address error\n");
            if (BIT_6 & this->m_firstByteResponse)
                printer.puts("\t6: Parameter error\n");
            if (BIT_7 & this->m_firstByteResponse)
                printer.puts("\t7: Something is really screwed up. This "
                              "should always be 0.\n");
        }

    private:
        /*************************
         *** Private Constants ***
         *************************/
        // SPI config
        static const uint32_t     SPI_INIT_FREQ = 200000;  // Run SD initialization at 200 kHz
        static const SPI::Mode    SPI_MODE      = SPI::MODE_0;
        static const SPI::BitMode SPI_BITMODE   = SPI::MSB_FIRST;

        // Misc. SD Definitions
        static const uint32_t RESPONSE_TIMEOUT;  // Wait 0.1 seconds for a response before timing out
        static const uint32_t SEND_ACTIVE_TIMEOUT;
        static const uint32_t SINGLE_BYTE_WIGGLE_ROOM;

        // SD Commands
        static const uint8_t CMD_IDLE           = 0x40 + 0;   // Send card into idle state
        static const uint8_t CMD_INTERFACE_COND = 0x40 + 8;   // Send interface condition and host voltage range
        static const uint8_t CMD_RD_CSD         = 0x40 + 9;   // Request "Card Specific Data" block contents
        static const uint8_t CMD_RD_CID         = 0x40 + 10;  // Request "Card Identification" block contents
        static const uint8_t CMD_RD_BLOCK       = 0x40 + 17;  // Request data block
        static const uint8_t CMD_WR_BLOCK       = 0x40 + 24;  // Write data block
        static const uint8_t CMD_WR_OP          = 0x40 + 41;  // Send operating conditions for SDC
        static const uint8_t CMD_APP            = 0x40 + 55;  // Inform card that following instruction is application specific
        static const uint8_t CMD_READ_OCR       = 0x40 + 58;  // Request "Operating Conditions Register" contents

        // SD Arguments
        static const uint32_t HOST_VOLTAGE_3V3 = 0x01;
        static const uint32_t R7_CHECK_PATTERN = 0xAA;
        static const uint32_t ARG_CMD8         = ((HOST_VOLTAGE_3V3 << 8) | R7_CHECK_PATTERN);
        static const uint32_t ARG_LEN          = 5;

        // SD CRCs
        static const uint8_t CRC_IDLE      = 0x95;
        static const uint8_t CRC_CMD8      = 0x87;  // CRC only valid for CMD8 argument of 0x000001AA
        static const uint8_t CRC_ACMD_PREP = 0x65;
        static const uint8_t CRC_ACMD      = 0x77;
        static const uint8_t CRC_OTHER     = 0x01;

        // SD Responses
        static const uint8_t RESPONSE_IDLE   = 0x01;
        static const uint8_t RESPONSE_ACTIVE = 0x00;
        static const uint8_t DATA_START_ID   = 0xFE;
        static const uint8_t RESPONSE_LEN_R1 = 1;
        static const uint8_t RESPONSE_LEN_R3 = 5;
        static const uint8_t RESPONSE_LEN_R7 = 5;
        static const uint8_t RSPNS_TKN_BITS  = 0x0f;
        static const uint8_t RSPNS_TKN_ACCPT = (0x02 << 1) | 1;
        static const uint8_t RSPNS_TKN_CRC   = (0x05 << 1) | 1;
        static const uint8_t RSPNS_TKN_WR    = (0x06 << 1) | 1;

    private:
        /*******************************
         *** Private Member Variable ***
         *******************************/
        SPI *m_spi;
        Pin m_cs;  // Chip select pin

        Pin::Mask m_mosi;
        Pin::Mask m_miso;
        Pin::Mask m_sclk;

        // First byte response receives special treatment to allow for verbose debugging
        uint8_t m_firstByteResponse;
};

const uint32_t SD::RESPONSE_TIMEOUT        = 100 * MILLISECOND;
const uint32_t SD::SEND_ACTIVE_TIMEOUT     = 500 * MILLISECOND;
const uint32_t SD::SINGLE_BYTE_WIGGLE_ROOM = 150 * MICROSECOND;

}
