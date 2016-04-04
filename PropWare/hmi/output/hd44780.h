/**
 * @file        PropWare/hd44780.h
 *
 * @author      David Zemon
 * @author      Collin Winans
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
#include <PropWare/gpio/pin.h>
#include <PropWare/gpio/port.h>
#include <PropWare/string/printer/printcapable.h>
#include <PropWare/string/printer/printer.h>

namespace PropWare {

/**
 * @brief       Support for the common "character LCD" modules using the HD44780
 *              controller for the Parallax Propeller
 *
 * @note        Does not natively support 40x4 or 24x4 character displays
 */
class HD44780 : public PrintCapable {
    public:
        /**
         * @brief   LCD databus width
         */
        typedef enum {
            /** 4-bit mode */BM_4 = 4,
            /** 8-bit mode */BM_8 = 8,
        } Bitmode;

        /**
         * @brief   Supported LCD dimensions; Used for determining cursor placement
         *
         * @note    There are two variations of 16x1 character LCDs; if you're
         *          unsure which version you have, try 16x1_1 first, it is more
         *          common. 16x1_1 uses both DDRAM lines of the controller,
         *          8-characters on each line; 16x1_2 places all 16 characters
         *          are a single line of DDRAM.
         */
        typedef enum {
            /** 8x1 */        DIM_8x1,
            /** 8x2 */        DIM_8x2,
            /** 8x2 */        DIM_8x4,
            /** 16x1 mode 1 */DIM_16x1_1,
            /** 16x1 mode 2 */DIM_16x1_2,
            /** 16x2 */       DIM_16x2,
            /** 16x2 */       DIM_16x4,
            /** 20x1 */       DIM_20x1,
            /** 20x2 */       DIM_20x2,
            /** 20x2 */       DIM_20x4,
            /** 24x1 */       DIM_24x1,
            /** 24x2 */       DIM_24x2,
            /** 40x1 */       DIM_40x1,
            /** 40x2 */       DIM_40x2,
        } Dimensions;

        /** Number of allocated error codes for HD44780 */
#define HD44780_ERRORS_LIMIT            16
        /** First HD44780 error code */
#define HD44780_ERRORS_BASE             48

        /**
         * Error codes - Proceeded by SD, SPI
         */
        typedef enum {
            /** No error */          NO_ERROR          = 0,
            /** First HD44780 error */BEG_ERROR = HD44780_ERRORS_BASE,
            /** HD44780 Error 0 */   INVALID_CTRL_SGNL = HD44780::BEG_ERROR,
            /** HD44780 Error 1 */   INVALID_DIMENSIONS,
            /** Last HD44780 error */END_ERROR         = HD44780::INVALID_DIMENSIONS
        } ErrorCode;

    protected:
        /**
         * Store metadata on the LCD device to determine when line-wraps should
         * and shouldn't occur
         */
        typedef struct {
            /** How many characters can be displayed on a single row */
            uint8_t charRows;
            /** How many characters can be displayed in a single column */
            uint8_t charColumns;
            /**
             * How many contiguous bytes of memory per visible character row
             */
            uint8_t ddramCharRowBreak;
            /** Last byte of memory used in each DDRAM line */
            uint8_t ddramLineEnd;
        } MemMap;

    public:
        /** Number of spaces inserted for '\\t' */
        static const uint8_t TAB_WIDTH = 4;

        /**
         * @name    Commands
         * @note    Must be combined with arguments below to create a parameter
         *          for the HD44780
         */
        static const uint8_t CLEAR          = BIT_0;
        static const uint8_t RET_HOME       = BIT_1;
        static const uint8_t ENTRY_MODE_SET = BIT_2;
        static const uint8_t DISPLAY_CTRL   = BIT_3;
        static const uint8_t SHIFT          = BIT_4;
        static const uint8_t FUNCTION_SET   = BIT_5;
        static const uint8_t SET_CGRAM_ADDR = BIT_6;
        static const uint8_t SET_DDRAM_ADDR = BIT_7;
        /**@}*/

        /**
         * @name    Entry mode arguments
         * @{
         */
        static const uint8_t SHIFT_INC = BIT_1;
        static const uint8_t SHIFT_EN  = BIT_0;
        /**@}*/

        /**
         * @name    Display control arguments
         * @{
         */
        static const uint8_t DISPLAY_PWR = BIT_2;
        static const uint8_t CURSOR      = BIT_1;
        static const uint8_t BLINK       = BIT_0;
        /**@}*/

        /**
         * @name    Cursor/display shift arguments
         * @{
         */
        static const uint8_t SHIFT_DISPLAY = BIT_3;  // 0 = shift cursor
        static const uint8_t SHIFT_RIGHT   = BIT_2;  // 0 = shift left
        /**@}*/

        /**
         * @name Function set arguments
         * @{
         */
        static const uint8_t FUNC_8BIT_MODE  = BIT_4;  // 0 = 4-bit mode
        static const uint8_t FUNC_2LINE_MODE = BIT_3;  // 0 = "1-line" mode - use 2-line mode for 2- and 4-line displays
        static const uint8_t FUNC_5x10_CHAR  = BIT_2;  // 0 = 5x8 dot mode
        /**@}*/

    public:
        /************************
         *** Public Functions ***
         ************************/
        HD44780 () {
            this->m_curPos      = &(this->m_bogus);
            this->m_curPos->row = 0;
            this->m_curPos->col = 0;
        }

        /**
         * @brief       Initialize an HD44780 LCD display
         *
         * @note        A 250 ms delay is called while the LCD does internal
         *              initialization
         *
         * @param[in]   lsbDataPin  Pin mask for the least significant pin of the data port
         * @param[in]   rs, rw, en  PropWare::Pin::Mask instances for each of the RS, RW, and EN signals
         * @param[in]   bitmode     Select between HD44780::BM_4 and HD44780::BM_8 modes to determine whether you
         *                          will need 4 data wires or 8 between the Propeller and your LCD device
         * @param[in]   dimensions  Dimensions of your LCD device. Most common is HD44780::DIM_16x2
         *
         * @return      Returns 0 upon success, otherwise error code
         */
        void start (const PropWare::Pin::Mask lsbDataPin, const Pin rs, const Pin rw, const Pin en,
                    const HD44780::Bitmode bitmode, const HD44780::Dimensions dimensions) {
            uint8_t arg;

            // Wait for a couple years until the LCD has done internal initialization
            waitcnt(250 * MILLISECOND + CNT);

            // Save all control signal pin masks
            this->m_rs = rs;
            this->m_rw = rw;
            this->m_en = en;
            this->m_rs.set_dir(PropWare::Pin::OUT);
            this->m_rw.set_dir(PropWare::Pin::OUT);
            this->m_en.set_dir(PropWare::Pin::OUT);
            this->m_rs.clear();
            this->m_rw.clear();
            this->m_en.clear();

            // Save data port
            this->m_dataPort.set_mask(lsbDataPin, bitmode);
            this->m_dataPort.set_dir(PropWare::Pin::OUT);

            // Save the modes
            this->generate_mem_map(dimensions);
            this->m_bitmode = bitmode;

            // Begin init routine:
            if (HD44780::BM_8 == bitmode)
                arg = 0x30;
            else
                /* Implied: "if (HD44780::BM_4 == bitmode)" */
                arg = 0x3;

            this->m_dataPort.write(arg);
            this->clock_pulse();
            waitcnt(100 * MILLISECOND + CNT);

            this->clock_pulse();
            waitcnt(100 * MILLISECOND + CNT);

            this->clock_pulse();
            waitcnt(10 * MILLISECOND + CNT);

            if (PropWare::HD44780::BM_4 == bitmode) {
                this->m_dataPort.write(0x2);
                this->clock_pulse();
            }

            // Default functions during initialization
            arg = PropWare::HD44780::FUNCTION_SET;
            if (PropWare::HD44780::BM_8 == bitmode)
                arg |= PropWare::HD44780::FUNC_8BIT_MODE;
            arg |= PropWare::HD44780::FUNC_2LINE_MODE;
            this->cmd(arg);

            // Turn off display shift (set cursor shift) and leave default of
            // shift-left
            arg = PropWare::HD44780::SHIFT;
            this->cmd(arg);

            // Turn the display on; Leave cursor off and not blinking
            arg = PropWare::HD44780::DISPLAY_CTRL
                    | PropWare::HD44780::DISPLAY_PWR;
            this->cmd(arg);

            // Set cursor to auto-increment upon writing a character
            arg = PropWare::HD44780::ENTRY_MODE_SET
                    | PropWare::HD44780::SHIFT_INC;
            this->cmd(arg);

            this->clear();
        }

        /**
         * @brief   Clear the LCD display and return cursor to home
         */
        void clear (void) {
            this->cmd(PropWare::HD44780::CLEAR);
            this->m_curPos->row = 0;
            this->m_curPos->col = 0;
            waitcnt(1530 * MICROSECOND + CNT);
        }

        /**
         * @brief       Move the cursor to a specified column and row
         *
         * @param[in]   row     Zero-indexed row to place the cursor
         * @param[in]   col     Zero indexed column to place the cursor
         */
        void move (const uint8_t row, const uint8_t col) const {
            uint8_t ddramLine, addr = 0;

            // Handle weird special case where a single row LCD is split across
            // multiple DDRAM lines (i.e., 16x1 type 1)
            if (this->m_memMap.ddramCharRowBreak > this->m_memMap.ddramLineEnd) {
                ddramLine = col / this->m_memMap.ddramLineEnd;
                if (ddramLine)
                    addr  = 0x40;
                addr |= col % this->m_memMap.ddramLineEnd;
            } else if (4 == this->m_memMap.charRows) {
                // Determine DDRAM line
                if (row % 2)
                    addr = 0x40;
                if (row / 2)
                    addr += this->m_memMap.ddramCharRowBreak;
                addr += col % this->m_memMap.ddramCharRowBreak;

            } else /* implied: "if (2 == memMap.charRows)" */{
                if (row)
                    addr = 0x40;
                addr |= col;
            }

            this->cmd(addr | PropWare::HD44780::SET_DDRAM_ADDR);
            this->m_curPos->row = row;
            this->m_curPos->col = col;
        }

        void puts (const char string[]) {
            const char *s = (char *) string;

            while (*s) {
                this->put_char(*s);
                ++s;
            }
        }

        void put_char (const char c) {
            // For manual new-line characters...
            if ('\n' == c) {
                this->m_curPos->row++;
                if (this->m_curPos->row == this->m_memMap.charRows)
                    this->m_curPos->row = 0;
                this->m_curPos->col     = 0;
                this->move(this->m_curPos->row, this->m_curPos->col);
            } else if ('\t' == c) {
                do {
                    this->put_char(' ');
                } while (this->m_curPos->col % PropWare::HD44780::TAB_WIDTH);
            } else if ('\r' == c)
                this->move(this->m_curPos->row, 0);
                // And for everything else...
            else {
                //set RS to data and RW to write
                this->m_rs.set();
                this->write((const uint8_t) c);

                // Insert a line wrap if necessary
                ++this->m_curPos->col;
                if (this->m_memMap.charColumns == this->m_curPos->col)
                    this->put_char('\n');

                // Handle weird special case where a single row LCD is split
                // across multiple DDRAM lines (i.e., 16x1 type 1)
                if (this->m_memMap.ddramCharRowBreak
                        > this->m_memMap.ddramLineEnd)
                    this->move(this->m_curPos->row, this->m_curPos->col);
            }
        }

        /**
         * @brief      Send a control command to the LCD module
         *
         * @param[in]  command  8-bit command to send to the LCD
         */
        void cmd (const uint8_t command) const {
            //set RS to command mode and RW to write
            this->m_rs.clear();
            this->write(command);
        }

        static void print_error_str (const Printer *printer, const HD44780::ErrorCode err) {
            char str[] = "HD44780 Error %u: %s\n";

            switch (err) {
                case PropWare::HD44780::INVALID_CTRL_SGNL:
                    printer->printf(str, err - PropWare::HD44780::BEG_ERROR, "invalid control signal");
                    break;
                case PropWare::HD44780::INVALID_DIMENSIONS:
                    printer->printf(str, err - PropWare::HD44780::BEG_ERROR,
                                    "invalid LCD dimension; please choose from the HD44780::Dimensions type");
                    break;
                default:
                    break;
            }
        }

    protected:
        /***************************
         *** Protected Functions ***
         ***************************/
        /**
         * @brief       Write a single byte to the LCD - instruction or data
         *
         * @param[in]   val     Value to be written
         */
        void write (const uint8_t val) const {
            // Clear RW to signal write value
            this->m_rw.clear();

            if (PropWare::HD44780::BM_4 == this->m_bitmode) {
                // shift out the high nibble
                this->m_dataPort.write(val >> 4);
                this->clock_pulse();

                // Shift out low nibble
                this->m_dataPort.write(val);
            }
                // Shift remaining four bits out
            else /* Implied: if (HD44780::8BIT == this->m_bitmode) */{
                this->m_dataPort.write(val);
            }
            this->clock_pulse();
        }

        /**
         * @brief   Toggle the enable pin, inducing a write to the LCD's
         *          register
         */
        void clock_pulse (void) const {
            this->m_en.set();
            waitcnt(MILLISECOND + CNT);
            this->m_en.clear();
        }

        /**
         * @brief   The memory map is used to determine where line wraps should
         *          and shouldn't occur
         */
        void generate_mem_map (const HD44780::Dimensions dimensions) {
            // TODO: Make this a look-up table instead of a switch-case
            switch (dimensions) {
                case PropWare::HD44780::DIM_8x1:
                    this->m_memMap.charRows          = 1;
                    this->m_memMap.charColumns       = 8;
                    this->m_memMap.ddramCharRowBreak = 8;
                    this->m_memMap.ddramLineEnd      = 8;
                    break;
                case PropWare::HD44780::DIM_8x2:
                    this->m_memMap.charRows          = 2;
                    this->m_memMap.charColumns       = 8;
                    this->m_memMap.ddramCharRowBreak = 8;
                    this->m_memMap.ddramLineEnd      = 8;
                    break;
                case PropWare::HD44780::DIM_8x4:
                    this->m_memMap.charRows          = 4;
                    this->m_memMap.charColumns       = 8;
                    this->m_memMap.ddramCharRowBreak = 8;
                    this->m_memMap.ddramLineEnd      = 16;
                    break;
                case PropWare::HD44780::DIM_16x1_1:
                    this->m_memMap.charRows          = 1;
                    this->m_memMap.charColumns       = 16;
                    this->m_memMap.ddramCharRowBreak = 8;
                    this->m_memMap.ddramLineEnd      = 8;
                    break;
                case PropWare::HD44780::DIM_16x1_2:
                    this->m_memMap.charRows          = 1;
                    this->m_memMap.charColumns       = 16;
                    this->m_memMap.ddramCharRowBreak = 16;
                    this->m_memMap.ddramLineEnd      = 16;
                    break;
                case PropWare::HD44780::DIM_16x2:
                    this->m_memMap.charRows          = 2;
                    this->m_memMap.charColumns       = 16;
                    this->m_memMap.ddramCharRowBreak = 16;
                    this->m_memMap.ddramLineEnd      = 16;
                    break;
                case PropWare::HD44780::DIM_16x4:
                    this->m_memMap.charRows          = 4;
                    this->m_memMap.charColumns       = 16;
                    this->m_memMap.ddramCharRowBreak = 16;
                    this->m_memMap.ddramLineEnd      = 32;
                    break;
                case PropWare::HD44780::DIM_20x1:
                    this->m_memMap.charRows          = 1;
                    this->m_memMap.charColumns       = 20;
                    this->m_memMap.ddramCharRowBreak = 20;
                    this->m_memMap.ddramLineEnd      = 20;
                    break;
                case PropWare::HD44780::DIM_20x2:
                    this->m_memMap.charRows          = 2;
                    this->m_memMap.charColumns       = 20;
                    this->m_memMap.ddramCharRowBreak = 20;
                    this->m_memMap.ddramLineEnd      = 20;
                    break;
                case PropWare::HD44780::DIM_20x4:
                    this->m_memMap.charRows          = 4;
                    this->m_memMap.charColumns       = 20;
                    this->m_memMap.ddramCharRowBreak = 20;
                    this->m_memMap.ddramLineEnd      = 40;
                    break;
                case PropWare::HD44780::DIM_24x1:
                    this->m_memMap.charRows          = 1;
                    this->m_memMap.charColumns       = 24;
                    this->m_memMap.ddramCharRowBreak = 24;
                    this->m_memMap.ddramLineEnd      = 24;
                    break;
                case PropWare::HD44780::DIM_24x2:
                    this->m_memMap.charRows          = 2;
                    this->m_memMap.charColumns       = 24;
                    this->m_memMap.ddramCharRowBreak = 24;
                    this->m_memMap.ddramLineEnd      = 24;
                    break;
                case PropWare::HD44780::DIM_40x1:
                    this->m_memMap.charRows          = 1;
                    this->m_memMap.charColumns       = 40;
                    this->m_memMap.ddramCharRowBreak = 40;
                    this->m_memMap.ddramLineEnd      = 40;
                    break;
                case PropWare::HD44780::DIM_40x2:
                    this->m_memMap.charRows          = 2;
                    this->m_memMap.charColumns       = 40;
                    this->m_memMap.ddramCharRowBreak = 40;
                    this->m_memMap.ddramLineEnd      = 40;
                    break;
            }
        }

    private:
        typedef struct {
            uint8_t row;
            uint8_t col;
        }                    Position;

    protected:
        HD44780::MemMap m_memMap;

    private:
        // Horrible bad hack so that methods can be const
        Position         m_bogus;
        Position         *m_curPos;
        Pin              m_rs, m_rw, m_en;
        SimplePort       m_dataPort;
        HD44780::Bitmode m_bitmode;
};

}
