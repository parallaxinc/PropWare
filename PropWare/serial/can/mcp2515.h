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

#define MAX_CHAR_IN_MESSAGE 8

/*
 *   Begin mt
 */
#define TIMEOUTVALUE    50
#define MCP_SIDH        0
#define MCP_SIDL        1
#define MCP_EID8        2
#define MCP_EID0        3

#define MCP_TXB_EXIDE_M     0x08                                        /* In TXBnSIDL                  */
#define MCP_DLC_MASK        0x0F                                        /* 4 LSBits                     */
#define MCP_RTR_MASK        0x40                                        /* (1<<6) Bit 6                 */

#define MCP_RXB_RX_ANY      0x60
#define MCP_RXB_RX_EXT      0x40
#define MCP_RXB_RX_STD      0x20
#define MCP_RXB_RX_STDEXT   0x00
#define MCP_RXB_RX_MASK     0x60
#define MCP_RXB_BUKT_MASK   (1<<2)

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

#define MCP_TXB_RTR_M       0x40                                        /* In TXBnDLC                   */
#define MCP_RXB_IDE_M       0x08                                        /* In RXBnSIDL                  */
#define MCP_RXB_RTR_M       0x40                                        /* In RXBnDLC                   */

#define MCP_STAT_RXIF_MASK   (0x03)
#define MCP_STAT_RX0IF (1<<0)
#define MCP_STAT_RX1IF (1<<1)

#define MCP_EFLG_RX1OVR (1<<7)
#define MCP_EFLG_RX0OVR (1<<6)
#define MCP_EFLG_TXBO   (1<<5)
#define MCP_EFLG_TXEP   (1<<4)
#define MCP_EFLG_RXEP   (1<<3)
#define MCP_EFLG_TXWAR  (1<<2)
#define MCP_EFLG_RXWAR  (1<<1)
#define MCP_EFLG_EWARN  (1<<0)
#define MCP_EFLG_ERRORMASK  (0xF8)                                      /* 5 MS-Bits                    */


/*
 *   Define MCP2515 register addresses
 */

#define MCP_RXF0SIDH    0x00
#define MCP_RXF0SIDL    0x01
#define MCP_RXF0EID8    0x02
#define MCP_RXF0EID0    0x03
#define MCP_RXF1SIDH    0x04
#define MCP_RXF1SIDL    0x05
#define MCP_RXF1EID8    0x06
#define MCP_RXF1EID0    0x07
#define MCP_RXF2SIDH    0x08
#define MCP_RXF2SIDL    0x09
#define MCP_RXF2EID8    0x0A
#define MCP_RXF2EID0    0x0B
#define MCP_CANSTAT     0x0E
#define MCP_CANCTRL     0x0F
#define MCP_RXF3SIDH    0x10
#define MCP_RXF3SIDL    0x11
#define MCP_RXF3EID8    0x12
#define MCP_RXF3EID0    0x13
#define MCP_RXF4SIDH    0x14
#define MCP_RXF4SIDL    0x15
#define MCP_RXF4EID8    0x16
#define MCP_RXF4EID0    0x17
#define MCP_RXF5SIDH    0x18
#define MCP_RXF5SIDL    0x19
#define MCP_RXF5EID8    0x1A
#define MCP_RXF5EID0    0x1B
#define MCP_TEC            0x1C
#define MCP_REC            0x1D
#define MCP_RXM0SIDH    0x20
#define MCP_RXM0SIDL    0x21
#define MCP_RXM0EID8    0x22
#define MCP_RXM0EID0    0x23
#define MCP_RXM1SIDH    0x24
#define MCP_RXM1SIDL    0x25
#define MCP_RXM1EID8    0x26
#define MCP_RXM1EID0    0x27
#define MCP_CNF3        0x28
#define MCP_CNF2        0x29
#define MCP_CNF1        0x2A
#define MCP_CANINTE        0x2B
#define MCP_CANINTF        0x2C
#define MCP_EFLG        0x2D
#define MCP_TXB0CTRL    0x30
#define MCP_TXB1CTRL    0x40
#define MCP_TXB2CTRL    0x50
#define MCP_RXB0CTRL    0x60
#define MCP_RXB0SIDH    0x61
#define MCP_RXB1CTRL    0x70
#define MCP_RXB1SIDH    0x71


#define MCP_TX_INT          0x1C                                    // Enable all transmit interrup ts
#define MCP_TX01_INT        0x0C                                    // Enable TXB0 and TXB1 interru pts
#define MCP_RX_INT          0x03                                    // Enable receive interrupts
#define MCP_NO_INT          0x00                                    // Disable all interrupts

#define MCP_TX01_MASK       0x14
#define MCP_TX_MASK        0x54

/*
 *   Define SPI Instruction Set
 */

#define MCP_WRITE           0x02

#define MCP_READ            0x03

#define MCP_BITMOD          0x05

#define MCP_LOAD_TX0        0x40
#define MCP_LOAD_TX1        0x42
#define MCP_LOAD_TX2        0x44

#define MCP_RTS_TX0         0x81
#define MCP_RTS_TX1         0x82
#define MCP_RTS_TX2         0x84
#define MCP_RTS_ALL         0x87

#define MCP_READ_RX0        0x90
#define MCP_READ_RX1        0x94

#define MCP_READ_STATUS     0xA0

#define MCP_RX_STATUS       0xB0

#define MCP_RESET           0xC0


/*
 *   CANCTRL Register Values
 */

#define MODE_NORMAL     0x00
#define MODE_SLEEP      0x20
#define MODE_LOOPBACK   0x40
#define MODE_LISTENONLY 0x60
#define MODE_CONFIG     0x80
#define MODE_POWERUP    0xE0
#define MODE_MASK       0xE0
#define ABORT_TX        0x10
#define MODE_ONESHOT    0x08
#define CLKOUT_ENABLE   0x04
#define CLKOUT_DISABLE  0x00
#define CLKOUT_PS1      0x00
#define CLKOUT_PS2      0x01
#define CLKOUT_PS4      0x02
#define CLKOUT_PS8      0x03


/*
 *   CNF1 Register Values
 */

#define SJW1            0x00
#define SJW2            0x40
#define SJW3            0x80
#define SJW4            0xC0


/*
 *   CNF2 Register Values
 */

#define BTLMODE         0x80
#define SAMPLE_1X       0x00
#define SAMPLE_3X       0x40


/*
 *   CNF3 Register Values
 */

#define SOF_ENABLE      0x80
#define SOF_DISABLE     0x00
#define WAKFIL_ENABLE   0x40
#define WAKFIL_DISABLE  0x00


/*
 *   CANINTF Register Bits
 */

#define MCP_RX0IF       0x01
#define MCP_RX1IF       0x02
#define MCP_TX0IF       0x04
#define MCP_TX1IF       0x08
#define MCP_TX2IF       0x10
#define MCP_ERRIF       0x20
#define MCP_WAKIF       0x40
#define MCP_MERRF       0x80

/*
 *  speed 16M
 */
#define MCP_16MHz_1000kBPS_CFG1 (0x00)
#define MCP_16MHz_1000kBPS_CFG2 (0xD0)
#define MCP_16MHz_1000kBPS_CFG3 (0x82)

#define MCP_16MHz_500kBPS_CFG1 (0x00)
#define MCP_16MHz_500kBPS_CFG2 (0xF0)
#define MCP_16MHz_500kBPS_CFG3 (0x86)

#define MCP_16MHz_250kBPS_CFG1 (0x41)
#define MCP_16MHz_250kBPS_CFG2 (0xF1)
#define MCP_16MHz_250kBPS_CFG3 (0x85)

#define MCP_16MHz_200kBPS_CFG1 (0x01)
#define MCP_16MHz_200kBPS_CFG2 (0xFA)
#define MCP_16MHz_200kBPS_CFG3 (0x87)

#define MCP_16MHz_125kBPS_CFG1 (0x03)
#define MCP_16MHz_125kBPS_CFG2 (0xF0)
#define MCP_16MHz_125kBPS_CFG3 (0x86)

#define MCP_16MHz_100kBPS_CFG1 (0x03)
#define MCP_16MHz_100kBPS_CFG2 (0xFA)
#define MCP_16MHz_100kBPS_CFG3 (0x87)

#define MCP_16MHz_80kBPS_CFG1 (0x03)
#define MCP_16MHz_80kBPS_CFG2 (0xFF)
#define MCP_16MHz_80kBPS_CFG3 (0x87)

#define MCP_16MHz_50kBPS_CFG1 (0x07)
#define MCP_16MHz_50kBPS_CFG2 (0xFA)
#define MCP_16MHz_50kBPS_CFG3 (0x87)

#define MCP_16MHz_40kBPS_CFG1 (0x07)
#define MCP_16MHz_40kBPS_CFG2 (0xFF)
#define MCP_16MHz_40kBPS_CFG3 (0x87)

#define MCP_16MHz_31k25BPS_CFG1 (0x0F)
#define MCP_16MHz_31k25BPS_CFG2 (0xF1)
#define MCP_16MHz_31k25BPS_CFG3 (0x85)

#define MCP_16MHz_20kBPS_CFG1 (0x0F)
#define MCP_16MHz_20kBPS_CFG2 (0xFF)
#define MCP_16MHz_20kBPS_CFG3 (0x87)

#define MCP_16MHz_10kBPS_CFG1 (0x1F)
#define MCP_16MHz_10kBPS_CFG2 (0xFF)
#define MCP_16MHz_10kBPS_CFG3 (0x87)

#define MCP_16MHz_5kBPS_CFG1 (0x3F)
#define MCP_16MHz_5kBPS_CFG2 (0xFF)
#define MCP_16MHz_5kBPS_CFG3 (0x87)


#define MCPDEBUG        (0)
#define MCPDEBUG_TXBUF  (0)
#define MCP_N_TXBUFFERS (3)

#define MCP_RXBUF_0 (MCP_RXB0SIDH)
#define MCP_RXBUF_1 (MCP_RXB1SIDH)

#define MCP2515_OK         (0)
#define MCP2515_FAIL       (1)
#define MCP_ALLTXBUSY      (2)

#define CANDEBUG   1

#define CANUSELOOP 0

#define CANSENDTIMEOUT (200)                                            /* milliseconds                 */

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

#define CAN_MAX_CHAR_IN_MESSAGE (8)

namespace PropWare {

class MCP2515 {
    public:
        typedef enum {
            NO_ERROR               = 0,
            INITIALIZATION_FAILURE = 1,
            MESSAGE_AVAILABLE      = 3,
            NO_MESSAGE             = 4,
            CONTROL_ERROR          = 5,
            GET_TX_BF_TIMEOUT      = 6,
            SEND_MSG_TIMEOUT       = 7
        } ErrorCode;

        typedef enum {
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
        } BaudRate;

    public:
        MCP2515 (const Pin::Mask cs, const Printer *logger = NULL)
                : m_spi(&SPI::get_instance()),
                  m_cs(cs, Pin::OUT),
                  m_logger(logger) {
            this->m_cs.set();
        }

        MCP2515 (const SPI &spi, const Pin::Mask cs, const Printer *logger = NULL)
                : m_spi(&spi),
                  m_cs(cs, Pin::OUT),
                  m_logger(logger) {
            this->m_cs.set();
        }

        PropWare::ErrorCode start (const BaudRate baudRate) const {
            PropWare::ErrorCode err;

            this->reset();

            check_errors(this->set_control_mode(MODE_CONFIG));

            if (NULL != this->m_logger)
                this->m_logger->print("Enter setting mode success \n");

            /* set baudrate                 */
            check_errors(this->set_baud(baudRate));

            if (NULL != this->m_logger)
                this->m_logger->print("set rate success!!\n");


            /* init canbuffers              */
            this->init_buffers();

            /* interrupt mode               */
            this->set_register(MCP_CANINTE, MCP_RX0IF | MCP_RX1IF);

#if (DEBUG_RXANY == 1)
            // enable both receive-buffers to receive any message and enable rollover
            this->modifyRegister(MCP_RXB0CTRL,
                                 MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
                                 MCP_RXB_RX_ANY | MCP_RXB_BUKT_MASK);
            this->modifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK,
                                 MCP_RXB_RX_ANY);
#else
            // enable both receive-buffers to receive messages with std. and ext. identifiers and enable rollover
            this->modify_register(MCP_RXB0CTRL,
                                  MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
                                  MCP_RXB_RX_STDEXT | MCP_RXB_BUKT_MASK);
            this->modify_register(MCP_RXB1CTRL, MCP_RXB_RX_MASK,
                                  MCP_RXB_RX_STDEXT);
#endif

            /* enter normal mode            */
            check_errors(this->set_control_mode(MODE_NORMAL));

            if (NULL != this->m_logger)
                this->m_logger->print("Enter Normal Mode Success!!\n");

            return NO_ERROR;
        }

        uint8_t init_mask (uint8_t num, uint8_t ext, uint32_t ulData) {

            if (NULL != this->m_logger)
                this->m_logger->print("Begin to set Mask!!\n");

            uint8_t res = this->set_control_mode(MODE_CONFIG);
            if (res > 0) {

                if (NULL != this->m_logger)
                    this->m_logger->print("Enter setting mode fall\n");

                return res;
            }

            if (num == 0) {
                this->write_id(MCP_RXM0SIDH, ext, ulData);

            }
            else if (num == 1) {
                this->write_id(MCP_RXM1SIDH, ext, ulData);
            }
            else
                res = MCP2515_FAIL;

            res = this->set_control_mode(MODE_NORMAL);
            if (res > 0) {

                if (NULL != this->m_logger)
                    this->m_logger->print("Enter normal mode fall\n");

                return res;
            }

            if (NULL != this->m_logger)
                this->m_logger->print("set Mask success!!\n");

            return res;
        }

        uint8_t init_Filt (uint8_t num, uint8_t ext, uint32_t ulData) {

            if (NULL != this->m_logger)
                this->m_logger->print("Begin to set Filter!!\n");

            uint8_t res = this->set_control_mode(MODE_CONFIG);
            if (res > 0) {

                if (NULL != this->m_logger)
                    this->m_logger->print("Enter setting mode fall\n");

                return res;
            }

            switch (num) {
                case 0:
                    this->write_id(MCP_RXF0SIDH, ext, ulData);
                    break;

                case 1:
                    this->write_id(MCP_RXF1SIDH, ext, ulData);
                    break;

                case 2:
                    this->write_id(MCP_RXF2SIDH, ext, ulData);
                    break;

                case 3:
                    this->write_id(MCP_RXF3SIDH, ext, ulData);
                    break;

                case 4:
                    this->write_id(MCP_RXF4SIDH, ext, ulData);
                    break;

                case 5:
                    this->write_id(MCP_RXF5SIDH, ext, ulData);
                    break;

                default:
                    res = MCP2515_FAIL;
            }

            res = this->set_control_mode(MODE_NORMAL);
            if (res > 0) {

                if (NULL != this->m_logger)
                    this->m_logger->print("Enter normal mode fall\nSet filter fail!!\n");

                return res;
            }

            if (NULL != this->m_logger)
                this->m_logger->print("set Filter success!!\n");


            return res;
        }

        void send_message (uint32_t id, uint8_t ext, uint8_t len, uint8_t *buf) {
            set_message(id, ext, len, buf);
            send_message();
        }

        void read_message (uint8_t *len, uint8_t *buf) {
            read_message();
            *len       = this->m_dlc;
            for (int i = 0; i < this->m_dlc; i++) {
                buf[i] = this->m_data[i];
            }
        }

        uint8_t check_receive_buffer () {
            // RXnIF in Bit 1 and 0
            const uint8_t res = this->read_status();

            if (res & MCP_STAT_RXIF_MASK)
                return MESSAGE_AVAILABLE;
            else
                return NO_MESSAGE;
        }

        PropWare::ErrorCode check_error () {
            const uint8_t eflg = this->read_register(MCP_EFLG);

            if (eflg & MCP_EFLG_ERRORMASK)
                return CONTROL_ERROR;
            else
                return NO_ERROR;
        }

        uint32_t get_id () {
            return this->m_id;
        }

/*
*  mcp2515 driver function
*/
    private:

        void reset () const {
            this->m_cs.clear();
            this->m_spi->shift_out(8, MCP_RESET);
            this->m_cs.set();
            waitcnt(10 * MILLISECOND + CNT);
        }

        uint8_t read_register (const uint8_t address) const {
            uint8_t ret;

            this->m_cs.clear();
            this->m_spi->shift_out(8, MCP_READ);
            this->m_spi->shift_out(8, address);
            ret = (uint8_t) this->m_spi->shift_in(8);
            this->m_cs.set();

            return ret;
        }

        void read_registers (const uint8_t address, uint8_t *values, const uint8_t n) {
            uint8_t i;
            this->m_cs.clear();
            this->m_spi->shift_out(8, MCP_READ);
            this->m_spi->shift_out(8, address);
            // mcp2515 has auto-increment of address-pointer
            for (i = 0; i < n; i++) {
                values[i] = (uint8_t) this->m_spi->shift_in(8);
            }
            this->m_cs.set();
        }

        void set_register (const uint8_t address, const uint8_t value) const {
            this->m_cs.clear();
            this->m_spi->shift_out(8, MCP_WRITE);
            this->m_spi->shift_out(8, address);
            this->m_spi->shift_out(8, value);
            this->m_cs.set();
        }

        void set_registers (const uint8_t address, const uint8_t *values, const uint8_t n) const {
            uint8_t i;
            this->m_cs.clear();
            this->m_spi->shift_out(8, MCP_WRITE);
            this->m_spi->shift_out(8, address);

            for (i = 0; i < n; i++) {
                this->m_spi->shift_out(8, values[i]);
            }
            this->m_cs.set();
        }

        void init_buffers () const {
            uint8_t i, a1, a2, a3;

            uint8_t  std    = 0;
            uint8_t  ext    = 1;
            uint32_t ulMask = 0x00, ulFilt = 0x00;


            this->write_id(MCP_RXM0SIDH, ext, ulMask);                        /*Set both masks to 0           */
            this->write_id(MCP_RXM1SIDH, ext, ulMask);                        /*Mask register ignores ext bit */

            /* Set all filters to 0         */
            this->write_id(MCP_RXF0SIDH, ext, ulFilt);                        /* RXB0: extended               */
            this->write_id(MCP_RXF1SIDH, std, ulFilt);                        /* RXB1: standard               */
            this->write_id(MCP_RXF2SIDH, ext, ulFilt);                        /* RXB2: extended               */
            this->write_id(MCP_RXF3SIDH, std, ulFilt);                        /* RXB3: standard               */
            this->write_id(MCP_RXF4SIDH, ext, ulFilt);
            this->write_id(MCP_RXF5SIDH, std, ulFilt);

            /* Clear, deactivate the three  */
            /* transmit buffers             */
            /* TXBnCTRL -> TXBnD7           */
            a1     = MCP_TXB0CTRL;
            a2     = MCP_TXB1CTRL;
            a3     = MCP_TXB2CTRL;
            for (i = 0; i < 14; i++) {                                          /* in-buffer loop               */
                this->set_register(a1, 0);
                this->set_register(a2, 0);
                this->set_register(a3, 0);
                a1++;
                a2++;
                a3++;
            }
            this->set_register(MCP_RXB0CTRL, 0);
            this->set_register(MCP_RXB1CTRL, 0);
        }

        void modify_register (const uint8_t address, const uint8_t mask, const uint8_t data) const {
            this->m_cs.clear();
            this->m_spi->shift_out(8, MCP_BITMOD);
            this->m_spi->shift_out(8, address);
            this->m_spi->shift_out(8, mask);
            this->m_spi->shift_out(8, data);
            this->m_cs.set();
        }

        uint8_t read_status () {
            uint8_t i;
            this->m_cs.clear();
            this->m_spi->shift_out(8, MCP_READ_STATUS);
            i = this->m_spi->shift_in(8);
            this->m_cs.set();

            return i;
        }

        PropWare::ErrorCode set_control_mode (const uint8_t mode) const {
            this->modify_register(MCP_CANCTRL, MODE_MASK, mode);

            const uint8_t i = static_cast<uint8_t>(this->read_register(MCP_CANCTRL) & MODE_MASK);

            if (i == mode)
                return NO_ERROR;
            else
                return MCP2515_FAIL;
        }

        uint8_t set_baud (const BaudRate baudRate) const {
            uint8_t cfg1, cfg2, cfg3;
            switch (baudRate) {
                case (BAUD_5KBPS):
                    cfg1 = MCP_16MHz_5kBPS_CFG1;
                    cfg2 = MCP_16MHz_5kBPS_CFG2;
                    cfg3 = MCP_16MHz_5kBPS_CFG3;
                    break;
                case (BAUD_10KBPS):
                    cfg1 = MCP_16MHz_10kBPS_CFG1;
                    cfg2 = MCP_16MHz_10kBPS_CFG2;
                    cfg3 = MCP_16MHz_10kBPS_CFG3;
                    break;
                case (BAUD_20KBPS):
                    cfg1 = MCP_16MHz_20kBPS_CFG1;
                    cfg2 = MCP_16MHz_20kBPS_CFG2;
                    cfg3 = MCP_16MHz_20kBPS_CFG3;
                    break;
                case (BAUD_31K25BPS):
                    cfg1 = MCP_16MHz_31k25BPS_CFG1;
                    cfg2 = MCP_16MHz_31k25BPS_CFG2;
                    cfg3 = MCP_16MHz_31k25BPS_CFG3;
                    break;
                case (BAUD_40KBPS):
                    cfg1 = MCP_16MHz_40kBPS_CFG1;
                    cfg2 = MCP_16MHz_40kBPS_CFG2;
                    cfg3 = MCP_16MHz_40kBPS_CFG3;
                    break;
                case (BAUD_50KBPS):
                    cfg1 = MCP_16MHz_50kBPS_CFG1;
                    cfg2 = MCP_16MHz_50kBPS_CFG2;
                    cfg3 = MCP_16MHz_50kBPS_CFG3;
                    break;
                case (BAUD_80KBPS):
                    cfg1 = MCP_16MHz_80kBPS_CFG1;
                    cfg2 = MCP_16MHz_80kBPS_CFG2;
                    cfg3 = MCP_16MHz_80kBPS_CFG3;
                    break;
                case (BAUD_100KBPS):
                    cfg1 = MCP_16MHz_100kBPS_CFG1;
                    cfg2 = MCP_16MHz_100kBPS_CFG2;
                    cfg3 = MCP_16MHz_100kBPS_CFG3;
                    break;
                case (BAUD_125KBPS):
                    cfg1 = MCP_16MHz_125kBPS_CFG1;
                    cfg2 = MCP_16MHz_125kBPS_CFG2;
                    cfg3 = MCP_16MHz_125kBPS_CFG3;
                    break;
                case (BAUD_200KBPS):
                    cfg1 = MCP_16MHz_200kBPS_CFG1;
                    cfg2 = MCP_16MHz_200kBPS_CFG2;
                    cfg3 = MCP_16MHz_200kBPS_CFG3;
                    break;
                case (BAUD_250KBPS):
                    cfg1 = MCP_16MHz_250kBPS_CFG1;
                    cfg2 = MCP_16MHz_250kBPS_CFG2;
                    cfg3 = MCP_16MHz_250kBPS_CFG3;
                    break;
                case (BAUD_500KBPS):
                    cfg1 = MCP_16MHz_500kBPS_CFG1;
                    cfg2 = MCP_16MHz_500kBPS_CFG2;
                    cfg3 = MCP_16MHz_500kBPS_CFG3;
                    break;
                case (BAUD_1000KBPS):
                    cfg1 = MCP_16MHz_1000kBPS_CFG1;
                    cfg2 = MCP_16MHz_1000kBPS_CFG2;
                    cfg3 = MCP_16MHz_1000kBPS_CFG3;
                    break;
                default:
                    return MCP2515_FAIL;
            }

            this->set_register(MCP_CNF1, cfg1);
            this->set_register(MCP_CNF2, cfg2);
            this->set_register(MCP_CNF3, cfg3);
            return NO_ERROR;
        }

        void write_id (const uint8_t mcp_addr, const uint8_t ext, const uint32_t id) const {
            uint16_t canid;
            uint8_t  tbufdata[4];

            canid = (uint16_t) (id & 0x0FFFF);

            if (ext == 1) {
                tbufdata[MCP_EID0] = (uint8_t) (canid & 0xFF);
                tbufdata[MCP_EID8] = (uint8_t) (canid >> 8);
                canid = (uint16_t) (id >> 16);
                tbufdata[MCP_SIDL] = (uint8_t) (canid & 0x03);
                tbufdata[MCP_SIDL] += (uint8_t) ((canid & 0x1C) << 3);
                tbufdata[MCP_SIDL] |= MCP_TXB_EXIDE_M;
                tbufdata[MCP_SIDH] = (uint8_t) (canid >> 5);
            }
            else {
                tbufdata[MCP_SIDH] = (uint8_t) (canid >> 3);
                tbufdata[MCP_SIDL] = (uint8_t) ((canid & 0x07) << 5);
                tbufdata[MCP_EID0] = 0;
                tbufdata[MCP_EID8] = 0;
            }
            this->set_registers(mcp_addr, tbufdata, 4);
        }

        void read_id (const uint8_t mcp_addr, uint8_t *ext, uint32_t *id) {
            uint8_t tbufdata[4];

            *ext = 0;
            *id  = 0;

            read_registers(mcp_addr, tbufdata, 4);

            *id = (tbufdata[MCP_SIDH] << 3) + (tbufdata[MCP_SIDL] >> 5);

            if ((tbufdata[MCP_SIDL] & MCP_TXB_EXIDE_M) == MCP_TXB_EXIDE_M) {
                /* extended id                  */
                *id  = (*id << 2) + (tbufdata[MCP_SIDL] & 0x03);
                *id  = (*id << 8) + tbufdata[MCP_EID8];
                *id  = (*id << 8) + tbufdata[MCP_EID0];
                *ext = 1;
            }
        }

        void write_canMsg (const uint8_t buffer_sidh_addr) {
            uint8_t mcp_addr;
            mcp_addr = buffer_sidh_addr;
            set_registers(mcp_addr + 5, this->m_data, this->m_dlc);                  /* write data bytes             */
            if (this->m_rtr == 1)                                                   /* if RTR set bit in byte       */
            {
                this->m_dlc |= MCP_RTR_MASK;
            }
            set_register((mcp_addr + 4), this->m_dlc);                        /* write the RTR and DLC        */
            this->write_id(mcp_addr, this->m_extFlg, this->m_id);                     /* write CAN id                 */

        }

        void read_canMsg (const uint8_t buffer_sidh_addr) {
            uint8_t mcp_addr, ctrl;

            mcp_addr = buffer_sidh_addr;

            read_id(mcp_addr, &this->m_extFlg, &this->m_id);

            ctrl = read_register(mcp_addr - 1);
            this->m_dlc = read_register(mcp_addr + 4);

            if ((ctrl & 0x08)) {
                this->m_rtr = 1;
            }
            else {
                this->m_rtr = 0;
            }

            this->m_dlc &= MCP_DLC_MASK;
            read_registers(mcp_addr + 5, &(this->m_data[0]), this->m_dlc);
        }

        void start_transmit (const uint8_t mcp_addr) {
            modify_register(mcp_addr - 1, MCP_TXB_TXREQ_M, MCP_TXB_TXREQ_M);
        }

        uint8_t get_next_free_tx_buffer (uint8_t *txbuf_n) {
            uint8_t res, i, ctrlval;
            uint8_t ctrlregs[MCP_N_TXBUFFERS] = {MCP_TXB0CTRL, MCP_TXB1CTRL, MCP_TXB2CTRL};

            res = MCP_ALLTXBUSY;
            *txbuf_n = 0x00;

            /* check all 3 TX-Buffers       */
            for (i = 0; i < MCP_N_TXBUFFERS; i++) {
                ctrlval = read_register(ctrlregs[i]);
                if ((ctrlval & MCP_TXB_TXREQ_M) == 0) {
                    *txbuf_n = ctrlregs[i] + 1;                                   /* return SIDH-address of Buffe */
                    /* r                            */
                    res = MCP2515_OK;
                    return res;                                                 /* ! function exit              */
                }
            }
            return res;
        }

        uint8_t set_message (uint32_t id, uint8_t ext, uint8_t len, uint8_t *pData) {
            int i = 0;
            this->m_extFlg = ext;
            this->m_id     = id;
            this->m_dlc    = len;
            for (i = 0; i < MAX_CHAR_IN_MESSAGE; i++)
                this->m_data[i] = *(pData + i);
            return MCP2515_OK;
        }

        uint8_t clear_message () {
            this->m_id     = 0;
            this->m_dlc    = 0;
            this->m_extFlg = 0;
            this->m_rtr    = 0;
            for (int i     = 0; i < this->m_dlc; i++)
                this->m_data[i] = 0x00;

            return MCP2515_OK;
        }

        uint8_t read_message () {
            const uint8_t stat = this->read_status();

            if (stat & MCP_STAT_RX0IF) {
                // Msg in Buffer 0
                this->read_canMsg(MCP_RXBUF_0);
                this->modify_register(MCP_CANINTF, MCP_RX0IF, 0);
                return NO_ERROR;
            } else if (stat & MCP_STAT_RX1IF) {
                // Msg in Buffer 1
                this->read_canMsg(MCP_RXBUF_1);
                this->modify_register(MCP_CANINTF, MCP_RX1IF, 0);
                return NO_ERROR;
            } else
                return NO_MESSAGE;
        }

        uint8_t send_message () {
            uint8_t  res, res1, txbuf_n;
            uint16_t uiTimeOut = 0;

            do {
                res = this->get_next_free_tx_buffer(&txbuf_n);                       /* info = addr.                 */
                uiTimeOut++;
            } while (res == MCP_ALLTXBUSY && (uiTimeOut < TIMEOUTVALUE));

            if (uiTimeOut == TIMEOUTVALUE) {
                return GET_TX_BF_TIMEOUT;                                      /* get tx buff time out         */
            }
            uiTimeOut = 0;
            this->write_canMsg(txbuf_n);
            this->start_transmit(txbuf_n);
            do {
                uiTimeOut++;
                res1 = this->read_register(txbuf_n); // read send buff ctrl reg
                res1 = res1 & 0x08;
            } while (res1 && (uiTimeOut < TIMEOUTVALUE));
            if (uiTimeOut == TIMEOUTVALUE)                                       /* send msg timeout             */
            {
                return SEND_MSG_TIMEOUT;
            }
            return NO_ERROR;

        }

    private:
        const SPI *m_spi;
        const Pin m_cs;

        /**
         * @brief   identifier xxxID
         *
         * either extended (the 29 LSB) or standard (the 11 LSB)
         */
        uint8_t  m_extFlg;
        /** can id */
        uint32_t m_id;
        /** data length */
        uint8_t  m_dlc;
        uint8_t  m_data[MAX_CHAR_IN_MESSAGE];
        uint8_t  m_rtr;

        /** Logger for debug messages */
        const Printer *m_logger;
};

};
