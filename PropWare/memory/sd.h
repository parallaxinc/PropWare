/**
 * @file        PropWare/memory/sd.h
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

#include <PropWare/PropWare.h>
#include <PropWare/memory/blockstorage.h>
#include <PropWare/serial/spi/spi.h>
#include <PropWare/gpio/pin.h>
#include <PropWare/hmi/output/printer.h>

/**
 * @brief   Value is injected by `propeller-load` if set in the configuration file
 */
extern int _cfg_sdspi_config1;

/**
 * @brief   Value is injected by `propeller-load` if set in the configuration file
 */
extern int _cfg_sdspi_config2;

namespace PropWare {

/**
 * @brief   First byte response receives special treatment to allow for verbose debugging (not for public use)
 */
extern unsigned char _sd_firstByteResponse;

/**
 * @brief   A simple SD driver communicating over the SPI protocol.
 *
 * When using PropWare's default SPI class, this allows the entire SPI/SD card/FAT functionality to run in a single cog.
 */
class SD : public BlockStorage {
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
         * @brief   Use the default SPI instance and pins for connecting to the SD card
         *
         * If a board configuration file has been predefined in PropGCC (such as `dna.cfg` for the Propeller DNA board),
         * then an instance of `PropWare::SD` can be constructed without any arguments. This is very convenient for
         * anyone using a common Propeller board that comes pre-equipped with an SD card adapter.
         *
         * @param[in]   spi     SPI instance. Its pins, clock frequency and mode will be modified to fit the SD
         * card's needs
         */
        SD (SPI &spi = SPI::get_instance())
                : m_spi(&spi) {
            Pin::Mask pins[4];
            unpack_sd_pins((uint32_t *) pins);

            this->m_spi->set_mosi(pins[0]);
            this->m_spi->set_miso(pins[1]);
            this->m_spi->set_sclk(pins[2]);

            // Set CS for output and initialize high
            this->m_cs.set_mask(pins[3]);
            this->m_cs.set();
            this->m_cs.set_dir_out();
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
        SD (SPI &spi, const Port::Mask mosi, const Port::Mask miso, const Port::Mask sclk, const Port::Mask cs)
                : m_spi(&spi) {
            this->m_spi->set_mosi(mosi);
            this->m_spi->set_miso(miso);
            this->m_spi->set_sclk(sclk);

            // Set CS for output and initialize high
            this->m_cs.set_mask(cs);
            this->m_cs.set();
            this->m_cs.set_dir_out();
        }

        /**
         * @brief       Initialize SD card communication over SPI for 3.3V configuration
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode start () const {
            PropWare::ErrorCode err;
            uint8_t             response[16];

            this->m_spi->set_clock(SPI_INIT_FREQ);
            this->m_spi->set_mode(SPI_MODE);
            this->m_spi->set_bit_mode(SPI_BITMODE);

            // Try and get the card up and responding to commands first
            check_errors(this->reset_and_verify_v2_0(response));

            check_errors(this->activate(response));

            check_errors(this->increase_throttle());

            // We're finally done initializing everything. Set chip select high again to release the SPI port
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

        PropWare::ErrorCode read_data_block (const uint32_t address, uint8_t buf[]) const {
            PropWare::ErrorCode err;
            uint8_t             temp = 0;

            // Wait until the SD card is no longer busy
            while (!temp)
                temp = (uint8_t) this->m_spi->shift_in(8);

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

        PropWare::ErrorCode write_data_block (uint32_t address, const uint8_t dat[]) const {
            PropWare::ErrorCode err;
            uint8_t             temp = 0;

            // Wait until the SD card is no longer busy
            while (!temp)
                temp = (uint8_t) this->m_spi->shift_in(8);

            this->m_cs.clear();
            this->send_command(CMD_WR_BLOCK, address, CRC_OTHER);

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
            buf[offset]     = value;
        }

        void write_long (const uint16_t offset, uint8_t buf[], const uint32_t value) const {
            buf[offset + 3] = (uint8_t) (value >> 24);
            buf[offset + 2] = (uint8_t) (value >> 16);
            buf[offset + 1] = (uint8_t) (value >> 8);
            buf[offset]     = (uint8_t) value;
        }

        /**
         * @brief   Create a human-readable error string
         *
         * @param[in]   printer     Printer used for logging the message
         * @param[in]   err         Error number used to determine error string
         */
        void print_error_str (const Printer &printer, const ErrorCode err) {
            const uint8_t relativeError = err - BEG_ERROR;

            switch (err) {
                case INVALID_CMD:
                    printer << "SD Error " << relativeError << ": Invalid command\n";
                    break;
                case READ_TIMEOUT:
                    printer << "SD Error " << relativeError << ": Timed out during read\n";
                    break;
                case INVALID_NUM_BYTES:
                    printer << "SD Error " << relativeError << ": Invalid number of bytes\n";
                    break;
                case INVALID_RESPONSE:
                    printer << "SD Error " << relativeError << ": Invalid first-byte response\n";
                    printer << "\tReceived: " << _sd_firstByteResponse << '\n';
                    this->first_byte_expansion(printer);
                    break;
                case INVALID_INIT:
                    printer << "SD Error " << relativeError << ": Invalid response during initialization\n";
                    printer << "\tResponse: " << _sd_firstByteResponse << '\n';
                    break;
                case INVALID_DAT_START_ID:
                    printer << "SD Error " << relativeError << ": Invalid data-start ID\n";
                    printer << "\tReceived: " << _sd_firstByteResponse << '\n';
                    break;
                default:
                    return;
            }
        }

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
            for (i       = 0; i < 10 && !stageCleared; ++i) {
                // Initialization loop (reset SD card)
                for (j = 0; j < 10 && !stageCleared; ++j) {
                    this->m_cs.set();
                    this->power_up();
                    // Chip select goes low for the duration of initization
                    this->m_cs.clear();
                    this->reset(response, stageCleared);
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
        inline void power_up () const {
            uint8_t i;
            waitcnt(CLKFREQ / 10 + CNT);

            // Send at least 72 clock cycles to enable the SD card
            this->m_cs.set();
            for (i = 0; i < 32; ++i)
                this->m_spi->shift_out(24, (uint32_t) -1);
        }

        inline void reset (uint8_t response[], bool &isIdle) const {
            uint8_t firstByte;

            // Send SD into idle state, retrieve a response and ensure it is the
            // "idle" response
            this->send_command(CMD_IDLE, 0, CRC_IDLE);
            this->get_response(RESPONSE_LEN_R1, firstByte, response);

            // Check if idle
            if (RESPONSE_IDLE == firstByte)
                isIdle = true;
        }

        inline PropWare::ErrorCode verify_v2_0 (uint8_t response[], bool *stageCleared) const {
            PropWare::ErrorCode err;
            uint8_t             firstByte;

            // Inform SD card that the Propeller uses the 2.7-3.6V range;
            this->send_command(CMD_INTERFACE_COND, ARG_CMD8, CRC_CMD8);
            check_errors(this->get_response(RESPONSE_LEN_R7, firstByte, response));
            if (RESPONSE_IDLE == firstByte)
                *stageCleared = true;

            return NO_ERROR;
        }

        inline PropWare::ErrorCode activate (uint8_t response[]) const {
            PropWare::ErrorCode err;
            uint32_t            timeout;
            uint8_t             firstByte;
            uint32_t            longWiggleRoom = 3 * MILLISECOND;
            bool                stageCleared   = false;

            // Attempt to send active
            timeout = SEND_ACTIVE_TIMEOUT + CNT;  //
            do {
                // Send the application-specific pre-command
                this->send_command(CMD_APP, 0, CRC_ACMD_PREP);
                check_errors(this->get_response(RESPONSE_LEN_R1, firstByte, response));

                // Request that the SD card go active!
                this->send_command(CMD_WR_OP, BIT_30, 0);
                check_errors(this->get_response(RESPONSE_LEN_R1, firstByte, response));

                // If the card ACKed with the active state, we're all good!
                if (RESPONSE_ACTIVE == firstByte)
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
        void send_command (const uint8_t cmd, const uint32_t arg, const uint8_t crc) const {
            // Send out the command
            this->m_spi->shift_out(8, cmd);

            // Send argument
            this->m_spi->shift_out(16, (arg >> 16));
            this->m_spi->shift_out(16, arg & WORD_0);

            // Send sixth byte - CRC
            this->m_spi->shift_out(8, crc);
        }

        /**
         * @brief       receive response and data from SD card over SPI
         *
         * @param[in]   numBytes    Number of bytes to receive
         * @param[out]  firstByte   First byte of response (active/idle) stored into this variable
         * @param[out]  dat         Location in memory with enough space to store `bytes` bytes of data
         *
         * @pre         Chip select must be activated prior to invocation
         *
         * @return      Returns 0 for success, else error code
         */
        PropWare::ErrorCode get_response (uint8_t numBytes, uint8_t &firstByte, uint8_t *dat) const {
            uint32_t timeout;

            // Read first byte - the R1 response
            timeout = RESPONSE_TIMEOUT + CNT;
            do {
                firstByte = (uint8_t) this->m_spi->shift_in(8);

                // Check for timeout
                if (abs(timeout - CNT) < SINGLE_BYTE_WIGGLE_ROOM)
                    return READ_TIMEOUT;

                // wait for transmission end
            } while (0xff == firstByte);


            // First byte in a response should always be either IDLE or ACTIVE.
            // If this one wasn't, throw an error. If it was, decrement the
            // bytes counter and read in all remaining bytes
            if ((RESPONSE_IDLE == firstByte) || (RESPONSE_ACTIVE == firstByte)) {
                --numBytes;    // Decrement bytes counter

                // Read remaining bytes
                while (numBytes--)
                    *dat++ = (uint8_t) this->m_spi->shift_in(8);
            } else {
                _sd_firstByteResponse = firstByte;
                return INVALID_RESPONSE;
            }

            // Responses should always be followed up by outputting 8 clocks
            // with MOSI high
            this->m_spi->shift_out(16, (uint32_t) -1);
            this->m_spi->shift_out(16, (uint32_t) -1);
            this->m_spi->shift_out(16, (uint32_t) -1);
            this->m_spi->shift_out(16, (uint32_t) -1);

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
                    dat[0] = (uint8_t) this->m_spi->shift_in(8);

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
                        checksum = (uint8_t) this->m_spi->shift_in(8);

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

            return NO_ERROR;
        }

        /**
         * @brief       Write data to SD card via SPI
         *
         * @param[in]   bytes   Block address to read from SD card
         * @param[in]   dat[]   Location in memory where data resides
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

            return NO_ERROR;
        }

        void first_byte_expansion (const Printer &printer) {
            if (BIT_0 & _sd_firstByteResponse)
                printer.puts("\t0: Idle\n");
            if (BIT_1 & _sd_firstByteResponse)
                printer.puts("\t1: Erase reset\n");
            if (BIT_2 & _sd_firstByteResponse)
                printer.puts("\t2: Illegal command\n");
            if (BIT_3 & _sd_firstByteResponse)
                printer.puts("\t3: Communication CRC error\n");
            if (BIT_4 & _sd_firstByteResponse)
                printer.puts("\t4: Erase sequence error\n");
            if (BIT_5 & _sd_firstByteResponse)
                printer.puts("\t5: Address error\n");
            if (BIT_6 & _sd_firstByteResponse)
                printer.puts("\t6: Parameter error\n");
            if (BIT_7 & _sd_firstByteResponse)
                printer.puts("\t7: Something is really screwed up. This should always be 0.\n");
        }

    private:

        static void unpack_sd_pins (uint32_t pins[]) {
            __asm__ volatile (
#ifdef __PROPELLER_COG__
                    "       jmp #skipVars          \n\t"
#else
                    "       brw #skipVars          \n\t"
#endif

                    "       .compress off          \n"
                    "__cfg_sdspi_config1           \n\t"
                    "       nop                    \n"
                    "__cfg_sdspi_config2           \n\t"
                    "       nop                    \n\t"
                    "       .compress default      \n"
                    "skipVars:                     \n\t"
            );

            pins[0] = (uint32_t) (1 << ((_cfg_sdspi_config1 >> 24) & BYTE_0)); // MOSI
            pins[1] = (uint32_t) (1 << ((_cfg_sdspi_config1 >> 16) & BYTE_0)); // MISO
            pins[2] = (uint32_t) (1 << ((_cfg_sdspi_config1 >> 8) & BYTE_0)); // SCLK
            pins[3] = (uint32_t) (1 << ((_cfg_sdspi_config2 >> 24) & BYTE_0)); // CS
        }

    private:
        /*************************
         *** Private Constants ***
         *************************/
        static const uint16_t SECTOR_SIZE       = 512;
        static const uint8_t  SECTOR_SIZE_SHIFT = 9;

        /** Run SD initialization at 200 kHz */
        static const uint32_t     SPI_INIT_FREQ  = 200000;
        /** Default frequency to run the SPI module */
        static const uint32_t     FULL_SPEED_SPI = 900000;
        static const SPI::Mode    SPI_MODE       = SPI::Mode::MODE_0;
        static const SPI::BitMode SPI_BITMODE    = SPI::BitMode::MSB_FIRST;

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
        static const uint8_t CMD_APP            = 0x40 + 55;  // Inform card that following instruction is app specific
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
};

const uint32_t SD::RESPONSE_TIMEOUT        = 100 * MILLISECOND;
const uint32_t SD::SEND_ACTIVE_TIMEOUT     = 500 * MILLISECOND;
const uint32_t SD::SINGLE_BYTE_WIGGLE_ROOM = 150 * MICROSECOND;

}
