/**
 * @file    PropWare/serial/can/mcp2515.h
 *
 * @author  David Zemon
 *
 * Inspired by MCP_CAN from https://github.com/coryjfowler/MCP_CAN_lib
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
#include <PropWare/serial/spi/spi.h>

/*
 *   Begin mt
 */
#define TIMEOUTVALUE        50
#define MCP_SIDH            0
#define MCP_SIDL            1
#define MCP_EID8            2
#define MCP_EID0            3

#define MCP_TXB_EXIDE_M     0x08    // In TXBnSIDL
#define MCP_DLC_MASK        NIBBLE_0
#define MCP_RTR_MASK        BIT_6

#define MCP_RXB_RX_ANY      0x60
#define MCP_RXB_RX_EXT      0x40
#define MCP_RXB_RX_STD      0x20
#define MCP_RXB_RX_STDEXT   0x00
#define MCP_RXB_RX_MASK     0x60
#define MCP_RXB_BUKT_MASK   BIT_2

/*
** Bits in the TXBnCTRL registers.
*/
#define MCP_TXB_TXBUFE_M    0x80
#define MCP_TXB_ABTF_M      0x40
#define MCP_TXB_MLOA_M      0x20
#define MCP_TXB_TXERR_M     0x10
#define MCP_TXB_TXREQ_M     0x08
#define MCP_TXB_TXIE_M      0x04
#define MCP_TXB_TXP10_M     0x03

#define MCP_TXB_RTR_M       0x40    // In TXBnDLC
#define MCP_RXB_IDE_M       0x08    // In RXBnSIDL
#define MCP_RXB_RTR_M       0x40    // In RXBnDLC

#define MCP_EFLG_RX1OVR     BIT_7
#define MCP_EFLG_RX0OVR     BIT_6
#define MCP_EFLG_TXBO       BIT_5
#define MCP_EFLG_TXEP       BIT_4
#define MCP_EFLG_RXEP       BIT_3
#define MCP_EFLG_TXWAR      BIT_2
#define MCP_EFLG_RXWAR      BIT_1
#define MCP_EFLG_EWARN      BIT_0
#define MCP_EFLG_ERRORMASK  (0xF8)  // 5 MS-Bits


#define MCP_TX_INT          0x1C    // Enable all transmit interrup ts
#define MCP_TX01_INT        0x0C    // Enable TXB0 and TXB1 interru pts
#define MCP_RX_INT          0x03    // Enable receive interrupts
#define MCP_NO_INT          0x00    // Disable all interrupts

#define MCP_TX01_MASK       0x14
#define MCP_TX_MASK         0x54

#define MCP_N_TXBUFFERS (3)

#define CANUSELOOP 0

#define CANSENDTIMEOUT (200)    // milliseconds

/*
 *   initial value of gCANAutoProcess
 */
#define CANAUTOPROCESS (1)
#define CANAUTOON  (1)
#define CANAUTOOFF (0)

#define CAN_STDID (0)
#define CAN_EXTID (1)

#define CANDEFAULTIDENT    (0x55CC)
#define CANDEFAULTIDENTEXT (CAN_EXTID)

namespace PropWare {

/**
 * @brief   Control and communicate with the Microchip MCP2515 CAN bus controller
 */
class MCP2515 {
    public:
        typedef enum {
            RXF0SIDH = 0x00,
            RXF0SIDL = 0x01,
            RXF0EID8 = 0x02,
            RXF0EID0 = 0x03,
            RXF1SIDH = 0x04,
            RXF1SIDL = 0x05,
            RXF1EID8 = 0x06,
            RXF1EID0 = 0x07,
            RXF2SIDH = 0x08,
            RXF2SIDL = 0x09,
            RXF2EID8 = 0x0A,
            RXF2EID0 = 0x0B,
            CANSTAT  = 0x0E,
            CANCTRL  = 0x0F,
            RXF3SIDH = 0x10,
            RXF3SIDL = 0x11,
            RXF3EID8 = 0x12,
            RXF3EID0 = 0x13,
            RXF4SIDH = 0x14,
            RXF4SIDL = 0x15,
            RXF4EID8 = 0x16,
            RXF4EID0 = 0x17,
            RXF5SIDH = 0x18,
            RXF5SIDL = 0x19,
            RXF5EID8 = 0x1A,
            RXF5EID0 = 0x1B,
            TEC      = 0x1C,
            REC      = 0x1D,
            RXM0SIDH = 0x20,
            RXM0SIDL = 0x21,
            RXM0EID8 = 0x22,
            RXM0EID0 = 0x23,
            RXM1SIDH = 0x24,
            RXM1SIDL = 0x25,
            RXM1EID8 = 0x26,
            RXM1EID0 = 0x27,
            CNF3     = 0x28,
            CNF2     = 0x29,
            CNF1     = 0x2A,
            CANINTE  = 0x2B,
            CANINTF  = 0x2C,
            EFLG     = 0x2D,
            TXB0CTRL = 0x30,
            TXB1CTRL = 0x40,
            TXB2CTRL = 0x50,
            RXB0CTRL = 0x60,
            RXB0SIDH = 0x61,
            RXB1CTRL = 0x70,
            RXB1SIDH = 0x71
        } RegisterAddress;

        typedef enum {
            RX0IF = BIT_0,
            RX1IF = BIT_1,
            TX0IF = BIT_2,
            TX1IF = BIT_3,
            TX2IF = BIT_4,
            ERRIF = BIT_5,
            WAKIF = BIT_6,
            MERRF = BIT_7
        } CANINTFBits;

        typedef enum {
            WRITE       = 0x02,
            READ        = 0x03,
            BITMOD      = 0x05,
            LOAD_TX0    = 0x40,
            LOAD_TX1    = 0x42,
            LOAD_TX2    = 0x44,
            RTS_TX0     = 0x81,
            RTS_TX1     = 0x82,
            RTS_TX2     = 0x84,
            RTS_ALL     = 0x87,
            READ_RX0    = 0x90,
            READ_RX1    = 0x94,
            READ_STATUS = 0xA0,
            RX_STATUS   = 0xB0,
            RESET       = 0xC0
        } SPIInstructionSet;

        enum class BufferNumber {
                BUFFER_0,
                BUFFER_1
        };

        enum class FilterNumber {
                FILTER_0,
                FILTER_1,
                FILTER_2,
                FILTER_3,
                FILTER_4,
                FILTER_5
        };

        typedef enum {
            NORMAL     = 0,
            SLEEP      = BIT_5,
            LOOPBACK   = BIT_6,
            LISTENONLY = BIT_6 | BIT_5,
            CONFIG     = BIT_7,
            POWERUP    = BIT_7 | BIT_6 | BIT_5
        } Mode;

        typedef enum {
            NO_ERROR,
            MODE_SET_FAILURE,
            NO_MESSAGE,
            CONTROL_ERROR,
            GET_TX_BUFFER_TIMEOUT,
            SEND_MESSAGE_TIMEOUT,
            MESSAGE_TOO_LONG,
            ALLTXBUSY
        } ErrorCode;

        static const Mode DEFAULT_MODE = Mode::NORMAL;

        static const uint8_t MODE_MASK = BIT_7 | BIT_6 | BIT_5;

/*
 *   CANCTRL Register Values
 */
#define ABORT_TX        0x10
#define MODE_ONESHOT    0x08
#define CLKOUT_ENABLE   0x04
#define CLKOUT_DISABLE  0x00
#define CLKOUT_PS1      0x00
#define CLKOUT_PS2      0x01
#define CLKOUT_PS4      0x02
#define CLKOUT_PS8      0x03

        enum class BaudRate {
                BAUD_5KBPS,
                BAUD_10KBPS,
                BAUD_20KBPS,
                BAUD_31K25BPS,
                BAUD_40KBPS,
                BAUD_50KBPS,
                BAUD_80KBPS,
                BAUD_100KBPS,
                BAUD_125KBPS,
                BAUD_200KBPS,
                BAUD_250KBPS,
                BAUD_500KBPS,
                BAUD_1000KBPS
        };

        /*
         *   CNF1 Register Values
         */

        static const uint8_t SJW1 = NULL_BIT;
        static const uint8_t SJW2 = BIT_6;
        static const uint8_t SJW3 = BIT_7;
        static const uint8_t SJW4 = BIT_7 | BIT_6;


        /*
         *   CNF2 Register Values
         */

        static const uint8_t SAMPLE_1X = NULL_BIT;
        static const uint8_t SAMPLE_3X = BIT_6;
        static const uint8_t BTLMODE   = BIT_7;


        /*
         *   CNF3 Register Values
         */

        static const uint8_t SOF_ENABLE     = 0x80;
        static const uint8_t SOF_DISABLE    = 0x00;
        static const uint8_t WAKFIL_ENABLE  = 0x40;
        static const uint8_t WAKFIL_DISABLE = 0x00;

        static const uint8_t CNF1_16MHz_1000kBPS = SJW1;
        static const uint8_t CNF2_16MHz_1000kBPS = 0xD0;
        static const uint8_t CNF3_16MHz_1000kBPS = 0x82;

        static const uint8_t CNF1_16MHz_500kBPS = SJW1;
        static const uint8_t CNF2_16MHz_500kBPS = 0xF0;
        static const uint8_t CNF3_16MHz_500kBPS = 0x86;

        static const uint8_t CNF1_16MHz_250kBPS = SJW2 | 1;
        static const uint8_t CNF2_16MHz_250kBPS = 0xF1;
        static const uint8_t CNF3_16MHz_250kBPS = 0x85;

        static const uint8_t CNF1_16MHz_200kBPS = SJW1 | 1;
        static const uint8_t CNF2_16MHz_200kBPS = 0xFA;
        static const uint8_t CNF3_16MHz_200kBPS = 0x87;

        static const uint8_t CNF1_16MHz_125kBPS = SJW1 | 3;
        static const uint8_t CNF2_16MHz_125kBPS = 0xF0;
        static const uint8_t CNF3_16MHz_125kBPS = 0x86;

        static const uint8_t CNF1_16MHz_100kBPS = SJW1 | 3;
        static const uint8_t CNF2_16MHz_100kBPS = 0xFA;
        static const uint8_t CNF3_16MHz_100kBPS = 0x87;

        static const uint8_t CNF1_16MHz_80kBPS = SJW1 | 3;
        static const uint8_t CNF2_16MHz_80kBPS = 0xFF;
        static const uint8_t CNF3_16MHz_80kBPS = 0x87;

        static const uint8_t CNF1_16MHz_50kBPS = SJW1 | 7;
        static const uint8_t CNF2_16MHz_50kBPS = 0xFA;
        static const uint8_t CNF3_16MHz_50kBPS = 0x87;

        static const uint8_t CNF1_16MHz_40kBPS = SJW1 | 7;
        static const uint8_t CNF2_16MHz_40kBPS = 0xFF;
        static const uint8_t CNF3_16MHz_40kBPS = 0x87;

        static const uint8_t CNF1_16MHz_31k25BPS = SJW1 | 15;
        static const uint8_t CNF2_16MHz_31k25BPS = 0xF1;
        static const uint8_t CNF3_16MHz_31k25BPS = 0x85;

        static const uint8_t CNF1_16MHz_20kBPS = SJW1 | 15;
        static const uint8_t CNF2_16MHz_20kBPS = 0xFF;
        static const uint8_t CNF3_16MHz_20kBPS = 0x87;

        static const uint8_t CNF1_16MHz_10kBPS = SJW1 | 31;
        static const uint8_t CNF2_16MHz_10kBPS = 0xFF;
        static const uint8_t CNF3_16MHz_10kBPS = 0x87;

        static const uint8_t CNF1_16MHz_5kBPS = SJW1 | 63;
        static const uint8_t CNF2_16MHz_5kBPS = 0xFF;
        static const uint8_t CNF3_16MHz_5kBPS = 0x87;

        /**
         * The MCP2515 only has an 8-byte data buffer
         */
        static const uint8_t MAX_DATA_BYTES = 8;

    public:
        MCP2515 (const Pin::Mask cs)
                : m_spi(&SPI::get_instance()),
                  m_cs(cs, Pin::Dir::OUT) {
            this->m_cs.set();
        }

        MCP2515 (const SPI &spi, const Pin::Mask cs)
                : m_spi(&spi),
                  m_cs(cs, Pin::Dir::OUT) {
            this->m_cs.set();
        }

        /**
         * @brief       Initialize the controller
         *
         * @param[in]   baudRate    CAN bus baud rate assuming a 16 MHz clock frequency
         * @param[in]   mode        Allows selection between normal, loopback and other modes. Configuration can not
         *                          be changed until the controller is restarted
         *
         * @return      Zero upon success, error code otherwise
         */
        PropWare::ErrorCode start (const BaudRate baudRate, const Mode mode = DEFAULT_MODE) {
            PropWare::ErrorCode err;

            this->reset();

            check_errors(this->set_control_mode(Mode::CONFIG));

            this->set_baud(baudRate);
            this->initialize_buffers();
            // interrupt mode
            this->set_register(CANINTE, RX0IF | RX1IF);

#if (DEBUG_RXANY == 1)
            // enable both receive-buffers to receive any message and enable rollover
            this->modifyRegister(RXB0CTRL,
                                 MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
                                 MCP_RXB_RX_ANY | MCP_RXB_BUKT_MASK);
            this->modifyRegister(RXB1CTRL, MCP_RXB_RX_MASK, MCP_RXB_RX_ANY);
#else
            // enable both receive-buffers to receive messages with std. and ext. identifiers and enable rollover
            this->modify_register(RXB0CTRL,
                                  MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
                                  MCP_RXB_RX_STDEXT | MCP_RXB_BUKT_MASK);
            this->modify_register(RXB1CTRL, MCP_RXB_RX_MASK, MCP_RXB_RX_STDEXT);
#endif

            /* enter normal mode            */
            this->m_mode = mode;
            check_errors(this->set_control_mode(this->m_mode));

            return NO_ERROR;
        }

        PropWare::ErrorCode set_mask (const BufferNumber bufferNumber, const uint32_t id,
                                      const bool extendedID = false) const {
            PropWare::ErrorCode err;

            check_errors(this->set_control_mode(Mode::CONFIG));

            if (BufferNumber::BUFFER_0 == bufferNumber)
                this->write_id(RXM0SIDH, id, extendedID);
            else
                this->write_id(RXM1SIDH, id, extendedID);

            return this->set_control_mode(this->m_mode);
        }

        PropWare::ErrorCode set_filter (const FilterNumber num,
                                        const uint32_t id, const bool extendedID = false) const {
            PropWare::ErrorCode err;
            check_errors(this->set_control_mode(Mode::CONFIG));

            switch (num) {
                case FilterNumber::FILTER_0:
                    this->write_id(RXF0SIDH, id, extendedID);
                    break;
                case FilterNumber::FILTER_1:
                    this->write_id(RXF1SIDH, id, extendedID);
                    break;
                case FilterNumber::FILTER_2:
                    this->write_id(RXF2SIDH, id, extendedID);
                    break;
                case FilterNumber::FILTER_3:
                    this->write_id(RXF3SIDH, id, extendedID);
                    break;
                case FilterNumber::FILTER_4:
                    this->write_id(RXF4SIDH, id, extendedID);
                    break;
                case FilterNumber::FILTER_5:
                    this->write_id(RXF5SIDH, id, extendedID);
                    break;
            }

            return this->set_control_mode(this->m_mode);
        }

        /**
         * @brief   Send a message
         *
         * @param[in] id            ID of the sender
         * @param[in] len           Number of bytes in the message. Must be less than or equal to MAX_CHAR_IN_MESSAGE
         * @param[in] buf           Address where the message data is stored
         * @param[in] extendedID    True for 29-bit, false for 11-bit
         */
        PropWare::ErrorCode send_message (const uint32_t id, const uint8_t len, const uint8_t buf[],
                                          const bool extendedID = false) {
            this->set_message(id, len, buf, extendedID);
            return this->send_message();
        }

        /**
         * @brief   Read a message from either buffer
         *
         * Buffer 0 is checked first. If no message is available on buffer 0, buffer 1 will be checked.
         *
         * @param[out]  buf     Address where incoming message will be stored
         * @param[out]  len     Number of bytes received
         *
         * @return      0 upon success. NO_MESSAGE if neither buffer has a message available
         */
        PropWare::ErrorCode read_message (uint8_t *len, uint8_t *buf) {
            PropWare::ErrorCode err;
            check_errors(this->read_message());
            *len = this->m_dataLength;
            memcpy(buf, this->m_messageBuffer, this->m_dataLength);
            return NO_ERROR;
        }

        /**
         * @brief   Read a message from a specific buffer
         *
         * @param[in]   bufferNumber    Buffer to read from
         * @param[out]  buf             Address where incoming message will be stored
         * @param[out]  len             Number of bytes received
         *
         * @return      0 upon success. NO_MESSAGE if the requested buffer does not contain a message
         */
        PropWare::ErrorCode read_message (const BufferNumber bufferNumber, uint8_t *len, uint8_t *buf) {
            PropWare::ErrorCode err;
            check_errors(this->read_message(bufferNumber));
            *len = this->m_dataLength;
            memcpy(buf, this->m_messageBuffer, this->m_dataLength);
            return NO_ERROR;
        }

        /**
         * @brief   Determine if a message is available for reading on either buffer
         *
         * @return  True when a message is available, false otherwise
         */
        bool check_receive_buffer () const {
            return static_cast<bool>(this->read_status() & (RX0IF | RX1IF));
        }

        /**
         * @brief       Determine if a message is ready to be read from the requested buffer
         *
         * @param[in]   bufferNumber    Number of the buffer to check
         *
         * @return      True when a message is available, false otherwise
         */
        bool check_receive_buffer (const BufferNumber bufferNumber) const {
            return static_cast<bool>(this->read_status() & (RX0IF + static_cast<unsigned int>(bufferNumber)));
        }

        PropWare::ErrorCode check_error () const {
            const uint8_t eflg = this->read_register(EFLG);

            if (eflg & MCP_EFLG_ERRORMASK)
                return CONTROL_ERROR;
            else
                return NO_ERROR;
        }

        uint32_t get_id () const {
            return this->m_id;
        }

    private:
        void reset () const {
            this->m_cs.clear();
            this->m_spi->shift_out(8, SPIInstructionSet::RESET);
            this->m_cs.set();
            waitcnt(10 * MILLISECOND + CNT);
        }

        uint8_t read_register (const uint8_t address) const {
            uint8_t ret;

            this->m_cs.clear();
            this->m_spi->shift_out(8, SPIInstructionSet::READ);
            this->m_spi->shift_out(8, address);
            ret = (uint8_t) this->m_spi->shift_in(8);
            this->m_cs.set();

            return ret;
        }

        void read_registers (const uint8_t address, uint8_t *values, const uint8_t n) {
            const uint32_t tmp          = SPIInstructionSet::READ << 8;
            const uint32_t combinedBits = tmp | address;
            this->m_cs.clear();
            this->m_spi->shift_out(16, combinedBits);
            // mcp2515 has auto-increment of address-pointer
            this->m_spi->shift_in_block_mode0_msb_first_fast(values, n);
            this->m_cs.set();
        }

        void set_register (const uint8_t address, const uint8_t value) const {
            const uint32_t tmp          = SPIInstructionSet::WRITE << 16;
            const uint32_t combinedBits = tmp | (address << 8) | value;
            this->m_cs.clear();
            this->m_spi->shift_out(24, combinedBits);
            this->m_cs.set();
        }

        void set_registers (const uint8_t address, const uint8_t values[], const uint8_t n) const {
            const uint32_t tmp          = SPIInstructionSet::WRITE << 8;
            const uint32_t combinedBits = tmp | address;
            this->m_cs.clear();
            this->m_spi->shift_out(16, combinedBits);
            this->m_spi->shift_out_block_msb_first_fast(values, n);
            this->m_cs.set();
        }

        void initialize_buffers () const {
            uint8_t i, a1, a2, a3;

            uint32_t ulMask = 0x00, ulFilt = 0x00;


            this->write_id(RXM0SIDH, ulMask, true);                        /*Set both masks to 0           */
            this->write_id(RXM1SIDH, ulMask, true);                        /*Mask register ignores ext bit */

            /* Set all filters to 0         */
            this->write_id(RXF0SIDH, ulFilt, true);                        /* RXB0: extended               */
            this->write_id(RXF1SIDH, ulFilt, false);                        /* RXB1: standard               */
            this->write_id(RXF2SIDH, ulFilt, true);                        /* RXB2: extended               */
            this->write_id(RXF3SIDH, ulFilt, false);                        /* RXB3: standard               */
            this->write_id(RXF4SIDH, ulFilt, true);
            this->write_id(RXF5SIDH, ulFilt, false);

            /* Clear, deactivate the three  */
            /* transmit buffers             */
            /* TXBnCTRL -> TXBnD7           */
            a1     = TXB0CTRL;
            a2     = TXB1CTRL;
            a3     = TXB2CTRL;
            for (i = 0; i < 14; i++) {                                          /* in-buffer loop               */
                this->set_register(a1, 0);
                this->set_register(a2, 0);
                this->set_register(a3, 0);
                a1++;
                a2++;
                a3++;
            }
            this->set_register(RXB0CTRL, 0);
            this->set_register(RXB1CTRL, 0);
        }

        void modify_register (const uint8_t address, const uint8_t mask, const uint8_t data) const {
            this->m_cs.clear();
            this->m_spi->shift_out(8, SPIInstructionSet::BITMOD);
            this->m_spi->shift_out(8, address);
            this->m_spi->shift_out(8, mask);
            this->m_spi->shift_out(8, data);
            this->m_cs.set();
        }

        uint8_t read_status () const {
            this->m_cs.clear();
            this->m_spi->shift_out(8, SPIInstructionSet::READ_STATUS);
            const uint8_t i = this->m_spi->shift_in(8);
            this->m_cs.set();
            return i;
        }

        PropWare::ErrorCode set_control_mode (const Mode mode) const {
            this->modify_register(CANCTRL, MODE_MASK, mode);

            const Mode actualMode = static_cast<Mode>(this->read_register(CANCTRL) & MODE_MASK);
            if (actualMode == mode)
                return NO_ERROR;
            else
                return MODE_SET_FAILURE;
        }

        void set_baud (const BaudRate baudRate) const {
            uint8_t cnf1 = 0;
            uint8_t cnf2 = 0;
            uint8_t cnf3 = 0;
            switch (baudRate) {
                case BaudRate::BAUD_5KBPS:
                    cnf1 = CNF1_16MHz_5kBPS;
                    cnf2 = CNF2_16MHz_5kBPS;
                    cnf3 = CNF3_16MHz_5kBPS;
                    break;
                case BaudRate::BAUD_10KBPS:
                    cnf1 = CNF1_16MHz_10kBPS;
                    cnf2 = CNF2_16MHz_10kBPS;
                    cnf3 = CNF3_16MHz_10kBPS;
                    break;
                case BaudRate::BAUD_20KBPS:
                    cnf1 = CNF1_16MHz_20kBPS;
                    cnf2 = CNF2_16MHz_20kBPS;
                    cnf3 = CNF3_16MHz_20kBPS;
                    break;
                case BaudRate::BAUD_31K25BPS:
                    cnf1 = CNF1_16MHz_31k25BPS;
                    cnf2 = CNF2_16MHz_31k25BPS;
                    cnf3 = CNF3_16MHz_31k25BPS;
                    break;
                case BaudRate::BAUD_40KBPS:
                    cnf1 = CNF1_16MHz_40kBPS;
                    cnf2 = CNF2_16MHz_40kBPS;
                    cnf3 = CNF3_16MHz_40kBPS;
                    break;
                case BaudRate::BAUD_50KBPS:
                    cnf1 = CNF1_16MHz_50kBPS;
                    cnf2 = CNF2_16MHz_50kBPS;
                    cnf3 = CNF3_16MHz_50kBPS;
                    break;
                case BaudRate::BAUD_80KBPS:
                    cnf1 = CNF1_16MHz_80kBPS;
                    cnf2 = CNF2_16MHz_80kBPS;
                    cnf3 = CNF3_16MHz_80kBPS;
                    break;
                case BaudRate::BAUD_100KBPS:
                    cnf1 = CNF1_16MHz_100kBPS;
                    cnf2 = CNF2_16MHz_100kBPS;
                    cnf3 = CNF3_16MHz_100kBPS;
                    break;
                case BaudRate::BAUD_125KBPS:
                    cnf1 = CNF1_16MHz_125kBPS;
                    cnf2 = CNF2_16MHz_125kBPS;
                    cnf3 = CNF3_16MHz_125kBPS;
                    break;
                case BaudRate::BAUD_200KBPS:
                    cnf1 = CNF1_16MHz_200kBPS;
                    cnf2 = CNF2_16MHz_200kBPS;
                    cnf3 = CNF3_16MHz_200kBPS;
                    break;
                case BaudRate::BAUD_250KBPS:
                    cnf1 = CNF1_16MHz_250kBPS;
                    cnf2 = CNF2_16MHz_250kBPS;
                    cnf3 = CNF3_16MHz_250kBPS;
                    break;
                case BaudRate::BAUD_500KBPS:
                    cnf1 = CNF1_16MHz_500kBPS;
                    cnf2 = CNF2_16MHz_500kBPS;
                    cnf3 = CNF3_16MHz_500kBPS;
                    break;
                case BaudRate::BAUD_1000KBPS:
                    cnf1 = CNF1_16MHz_1000kBPS;
                    cnf2 = CNF2_16MHz_1000kBPS;
                    cnf3 = CNF3_16MHz_1000kBPS;
                    break;
            }

            this->set_register(CNF1, cnf1);
            this->set_register(CNF2, cnf2);
            this->set_register(CNF3, cnf3);
        }

        void write_id (const uint8_t address, const uint32_t id, const bool extendedID) const {
            uint8_t        buffer[4];
            const uint16_t canIDLow = static_cast<uint16_t>(id & WORD_0);

            if (extendedID) {
                const uint16_t canIDHigh = static_cast<uint16_t>(id >> 16);

                buffer[MCP_EID0] = static_cast<uint8_t>(canIDLow & BYTE_0);
                buffer[MCP_EID8] = static_cast<uint8_t>(canIDLow >> 8);

                buffer[MCP_SIDL] = static_cast<uint8_t>(canIDHigh & 0x03);
                buffer[MCP_SIDL] += static_cast<uint8_t>((canIDHigh & 0x1C) << 3);
                buffer[MCP_SIDL] |= MCP_TXB_EXIDE_M;
                buffer[MCP_SIDH] = static_cast<uint8_t>(canIDHigh >> 5);
            } else {
                buffer[MCP_SIDH] = static_cast<uint8_t>(canIDLow >> 3);
                buffer[MCP_SIDL] = static_cast<uint8_t>(canIDLow << 5);

                buffer[MCP_EID0] = 0;
                buffer[MCP_EID8] = 0;
            }
            this->set_registers(address, buffer, 4);
        }

        void read_id (const uint8_t address, uint32_t *id, bool *extendedID) {
            uint8_t buffer[4];

            this->read_registers(address, buffer, 4);

            *id = (buffer[MCP_SIDH] << 3) + (buffer[MCP_SIDL] >> 5);
            if ((buffer[MCP_SIDL] & MCP_TXB_EXIDE_M) == MCP_TXB_EXIDE_M) {
                /* extended id                  */
                *id         = (*id << 2) + (buffer[MCP_SIDL] & 0x03);
                *id         = (*id << 8) + buffer[MCP_EID8];
                *id         = (*id << 8) + buffer[MCP_EID0];
                *extendedID = true;
            } else
                *extendedID = false;
        }

        void write_can_message (const uint8_t bufferSIDHAddress) {
            uint8_t mcp_addr;
            mcp_addr = bufferSIDHAddress;
            this->set_registers(mcp_addr + 5, this->m_messageBuffer, this->m_dataLength);
            if (this->m_rtr == 1)
                this->m_dataLength |= MCP_RTR_MASK;
            // write the RTR and DLC
            this->set_register((mcp_addr + 4), this->m_dataLength);
            this->write_id(mcp_addr, this->m_id, this->m_extendedID);

        }

        void read_can_message (const uint8_t bufferSIDHAddress) {
            this->read_id(bufferSIDHAddress, &this->m_id, &this->m_extendedID);

            const uint8_t ctrl = this->read_register(bufferSIDHAddress - 1);
            this->m_dataLength = this->read_register(bufferSIDHAddress + 4);

            if ((ctrl & 0x08))
                this->m_rtr = 1;
            else
                this->m_rtr = 0;

            this->m_dataLength &= MCP_DLC_MASK;
            read_registers(bufferSIDHAddress + 5, &(this->m_messageBuffer[0]), this->m_dataLength);
        }

        void start_transmit (const uint8_t address) {
            this->modify_register(address - 1, MCP_TXB_TXREQ_M, MCP_TXB_TXREQ_M);
        }

        PropWare::ErrorCode get_next_free_tx_buffer (uint8_t *txbuf_n) const {
            const uint8_t controlRegisters[] = {TXB0CTRL, TXB1CTRL, TXB2CTRL};

            /* check all 3 TX-Buffers       */
            for (uint_fast8_t i = 0; i < Utility::size_of_array(controlRegisters); i++) {
                const uint8_t controlValue = read_register(controlRegisters[i]);
                if ((controlValue & MCP_TXB_TXREQ_M) == 0) {
                    // return SIDH-address of buffer
                    *txbuf_n = static_cast<uint8_t>(controlRegisters[i] + 1);
                    return NO_ERROR;
                }
            }
            return ALLTXBUSY;
        }

        uint8_t set_message (const uint32_t id, const uint8_t length, const uint8_t data[], const bool extendedID) {
            this->m_extendedID = extendedID;
            this->m_id         = id;
            this->m_dataLength = length;

            if (MAX_DATA_BYTES < length) {
                return MESSAGE_TOO_LONG;
            } else {
                memcpy(this->m_messageBuffer, data, length);
                return NO_ERROR;
            }
        }

        void clear_message () {
            this->m_id         = 0;
            this->m_dataLength = 0;
            this->m_extendedID = 0;
            this->m_rtr        = 0;
            memset(this->m_messageBuffer, 0, this->m_dataLength);
        }

        PropWare::ErrorCode read_message () {
            const uint8_t stat = this->read_status();

            if (Utility::bit_read(stat, (Bit) RX0IF)) {
                this->read_can_message((RXB0SIDH));
                this->modify_register(CANINTF, RX0IF, 0);
                return NO_ERROR;
            } else if (Utility::bit_read(stat, (Bit) RX1IF)) {
                this->read_can_message((RXB1SIDH));
                this->modify_register(CANINTF, RX1IF, 0);
                return NO_ERROR;
            } else
                return NO_MESSAGE;
        }

        PropWare::ErrorCode read_message (const BufferNumber bufferNumber) {
            const uint8_t stat = this->read_status();

            const Bit interruptFlag = (Bit) (RX0IF + static_cast<uint8_t>(bufferNumber));
            uint8_t   bufferAddress = RXB0SIDH;
            if (BufferNumber::BUFFER_1 == bufferNumber)
                bufferAddress += 0x10;

            if (Utility::bit_read(stat, interruptFlag)) {
                this->read_can_message(bufferAddress);
                this->modify_register(CANINTF, interruptFlag, 0);
                return NO_ERROR;
            } else
                return NO_MESSAGE;
        }

        PropWare::ErrorCode send_message () {
            PropWare::ErrorCode err;
            uint8_t             txbuf_n;
            uint16_t            retryCount = 0;

            do {
                // info = addr
                err = this->get_next_free_tx_buffer(&txbuf_n);
                retryCount++;
            } while (err == ALLTXBUSY && (TIMEOUTVALUE > retryCount));

            if (TIMEOUTVALUE == retryCount)
                return GET_TX_BUFFER_TIMEOUT;
            else {
                retryCount = 0;
                this->write_can_message(txbuf_n);
                this->start_transmit(txbuf_n);

                bool messageSent;
                do {
                    retryCount++;

                    // read send buff ctrl reg
                    messageSent = this->read_register(txbuf_n) & BIT_3;
                } while (messageSent && (TIMEOUTVALUE > retryCount));

                if (retryCount == TIMEOUTVALUE)
                    return SEND_MESSAGE_TIMEOUT;
                else
                    return NO_ERROR;
            }
        }

    private:
        const SPI *m_spi;
        const Pin m_cs;

        Mode m_mode;

        /**
         * @brief   identifier xxxID
         *
         * either extended (the 29 LSB) or standard (the 11 LSB)
         */
        bool     m_extendedID;
        uint32_t m_id;
        uint8_t  m_dataLength;
        uint8_t  m_messageBuffer[MAX_DATA_BYTES];
        uint8_t  m_rtr;
};

};
