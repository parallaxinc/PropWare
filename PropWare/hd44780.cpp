/**
 * @file    hd44780.cpp
 *
 * @project PropWare
 *
 * @author  David Zemon
 * @author  Collin Winans
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

#include <tinyio.h>
#include <PropWare/hd44780.h>

const uint8_t PropWare::HD44780::TAB_WIDTH = 4;

const uint8_t PropWare::HD44780::CLEAR = BIT_0;
const uint8_t PropWare::HD44780::RET_HOME = BIT_1;
const uint8_t PropWare::HD44780::ENTRY_MODE_SET = BIT_2;
const uint8_t PropWare::HD44780::DISPLAY_CTRL = BIT_3;
const uint8_t PropWare::HD44780::SHIFT = BIT_4;
const uint8_t PropWare::HD44780::FUNCTION_SET = BIT_5;
const uint8_t PropWare::HD44780::SET_CGRAM_ADDR = BIT_6;
const uint8_t PropWare::HD44780::SET_DDRAM_ADDR = BIT_7;

const uint8_t PropWare::HD44780::SHIFT_INC = BIT_1;
const uint8_t PropWare::HD44780::SHIFT_EN = BIT_0;

const uint8_t PropWare::HD44780::DISPLAY_PWR = BIT_2;
const uint8_t PropWare::HD44780::CURSOR = BIT_1;
const uint8_t PropWare::HD44780::BLINK = BIT_0;

const uint8_t PropWare::HD44780::SHIFT_DISPLAY = BIT_3;
const uint8_t PropWare::HD44780::SHIFT_RIGHT = BIT_2;

const uint8_t PropWare::HD44780::FUNC_8BIT_MODE = BIT_4;
const uint8_t PropWare::HD44780::FUNC_2LINE_MODE = BIT_3;
const uint8_t PropWare::HD44780::FUNC_5x10_CHAR = BIT_2;

PropWare::HD44780::HD44780 () {
    this->m_curRow = 0;
    this->m_curCol = 0;
}

PropWare::ErrorCode PropWare::HD44780::start (const PropWare::Pin::Mask lsbDataPin,
        const PropWare::Pin rs, const PropWare::Pin rw,
        const PropWare::Pin en, const PropWare::HD44780::Bitmode bitmode,
        const PropWare::HD44780::Dimensions dimensions) {
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
    this->m_dim = dimensions;
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
    if (0 != dimensions % 3)
        arg |= PropWare::HD44780::FUNC_2LINE_MODE;
    this->cmd(arg);

    // Turn off display shift (set cursor shift) and leave default of shift-left
    arg = PropWare::HD44780::SHIFT;
    this->cmd(arg);

    // Turn the display on; Leave cursor off and not blinking
    arg = PropWare::HD44780::DISPLAY_CTRL | PropWare::HD44780::DISPLAY_PWR;
    this->cmd(arg);

    // Set cursor to auto-increment upon writing a character
    arg = PropWare::HD44780::ENTRY_MODE_SET | PropWare::HD44780::SHIFT_INC;
    this->cmd(arg);

    this->clear();

    return 0;
}

void PropWare::HD44780::clear (void) {
    this->cmd(PropWare::HD44780::CLEAR);
    this->m_curRow = 0;
    this->m_curCol = 0;
    waitcnt(1530*MICROSECOND + CNT);
}

void PropWare::HD44780::move (const uint8_t row, const uint8_t col) {
    uint8_t ddramLine, addr = 0;

    // Handle weird special case where a single row LCD is split across
    // multiple DDRAM lines (i.e., 16x1 type 1)
    if (this->m_memMap.ddramCharRowBreak > this->m_memMap.ddramLineEnd) {
        ddramLine = col / this->m_memMap.ddramLineEnd;
        if (ddramLine)
            addr = 0x40;
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
    this->m_curRow = row;
    this->m_curCol = col;
}

void PropWare::HD44780::putStr (const char str[]) {
    const char *s = str;

    while (*s) {
        this->putChar(*s);
        ++s;
    }
}

void PropWare::HD44780::putChar (const char c) {
    // For manual new-line characters...
    if ('\n' == c) {
        if (++this->m_curRow == this->m_memMap.charRows)
            this->m_curRow = 0;
        this->m_curCol = 0;
        this->move(this->m_curRow, this->m_curCol);
    } else if ('\t' == c) {
        do {
            this->putChar(' ');
        } while (this->m_curCol % PropWare::HD44780::TAB_WIDTH);
    }
    // And for everything else...
    else {
        //set RS to data and RW to write
        this->m_rs.set();
        this->write(c);

        // Insert a line wrap if necessary
        ++this->m_curCol;
        if (this->m_memMap.charColumns == this->m_curCol)
            this->putChar('\n');

        // Handle weird special case where a single row LCD is split across
        // multiple DDRAM lines (i.e., 16x1 type 1)
        if (this->m_memMap.ddramCharRowBreak > this->m_memMap.ddramLineEnd)
            this->move(this->m_curRow, this->m_curCol);
    }
}

void PropWare::HD44780::cmd (const uint8_t c) {
    //set RS to command mode and RW to write
    this->m_rs.clear();
    this->write(c);
}

void PropWare::HD44780::write (const uint8_t val) {
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

void PropWare::HD44780::clock_pulse (void) {
    this->m_en.set();
    waitcnt(MILLISECOND + CNT);
    this->m_en.clear();
}

void PropWare::HD44780::generate_mem_map (
        const PropWare::HD44780::Dimensions dimensions) {
    switch (dimensions) {
        case PropWare::HD44780::DIM_8x1:
            this->m_memMap.charRows = 1;
            this->m_memMap.charColumns = 8;
            this->m_memMap.ddramCharRowBreak = 8;
            this->m_memMap.ddramLineEnd = 8;
            break;
        case PropWare::HD44780::DIM_8x2:
            this->m_memMap.charRows = 2;
            this->m_memMap.charColumns = 8;
            this->m_memMap.ddramCharRowBreak = 8;
            this->m_memMap.ddramLineEnd = 8;
            break;
        case PropWare::HD44780::DIM_8x4:
            this->m_memMap.charRows = 4;
            this->m_memMap.charColumns = 8;
            this->m_memMap.ddramCharRowBreak = 8;
            this->m_memMap.ddramLineEnd = 16;
            break;
        case PropWare::HD44780::DIM_16x1_1:
            this->m_memMap.charRows = 1;
            this->m_memMap.charColumns = 16;
            this->m_memMap.ddramCharRowBreak = 8;
            this->m_memMap.ddramLineEnd = 8;
            break;
        case PropWare::HD44780::DIM_16x1_2:
            this->m_memMap.charRows = 1;
            this->m_memMap.charColumns = 16;
            this->m_memMap.ddramCharRowBreak = 16;
            this->m_memMap.ddramLineEnd = 16;
            break;
        case PropWare::HD44780::DIM_16x2:
            this->m_memMap.charRows = 2;
            this->m_memMap.charColumns = 16;
            this->m_memMap.ddramCharRowBreak = 16;
            this->m_memMap.ddramLineEnd = 16;
            break;
        case PropWare::HD44780::DIM_16x4:
            this->m_memMap.charRows = 4;
            this->m_memMap.charColumns = 16;
            this->m_memMap.ddramCharRowBreak = 16;
            this->m_memMap.ddramLineEnd = 32;
            break;
        case PropWare::HD44780::DIM_20x1:
            this->m_memMap.charRows = 1;
            this->m_memMap.charColumns = 20;
            this->m_memMap.ddramCharRowBreak = 20;
            this->m_memMap.ddramLineEnd = 20;
            break;
        case PropWare::HD44780::DIM_20x2:
            this->m_memMap.charRows = 2;
            this->m_memMap.charColumns = 20;
            this->m_memMap.ddramCharRowBreak = 20;
            this->m_memMap.ddramLineEnd = 20;
            break;
        case PropWare::HD44780::DIM_20x4:
            this->m_memMap.charRows = 4;
            this->m_memMap.charColumns = 8;
            this->m_memMap.ddramCharRowBreak = 8;
            this->m_memMap.ddramLineEnd = 20;
            break;
        case PropWare::HD44780::DIM_24x1:
            this->m_memMap.charRows = 1;
            this->m_memMap.charColumns = 24;
            this->m_memMap.ddramCharRowBreak = 24;
            this->m_memMap.ddramLineEnd = 24;
            break;
        case PropWare::HD44780::DIM_24x2:
            this->m_memMap.charRows = 2;
            this->m_memMap.charColumns = 24;
            this->m_memMap.ddramCharRowBreak = 24;
            this->m_memMap.ddramLineEnd = 24;
            break;
        case PropWare::HD44780::DIM_40x1:
            this->m_memMap.charRows = 1;
            this->m_memMap.charColumns = 40;
            this->m_memMap.ddramCharRowBreak = 40;
            this->m_memMap.ddramLineEnd = 40;
            break;
        case PropWare::HD44780::DIM_40x2:
            this->m_memMap.charRows = 2;
            this->m_memMap.charColumns = 40;
            this->m_memMap.ddramCharRowBreak = 40;
            this->m_memMap.ddramLineEnd = 40;
            break;
        default:
            break;
    }
}

void PropWare::HD44780::print_error_str (
        const PropWare::HD44780::ErrorCode err) {
    char str[] = "HD44780 Error %u: %s\n";

    switch (err) {
        case PropWare::HD44780::INVALID_CTRL_SGNL:
            printf(str, err - PropWare::HD44780::BEG_ERROR, "invalid control signal");
            break;
        case PropWare::HD44780::INVALID_DATA_MASK:
            printf(str, err - PropWare::HD44780::BEG_ERROR, "invalid data-pins mask");
            break;
        case PropWare::HD44780::INVALID_DIMENSIONS:
            printf(str, err - PropWare::HD44780::BEG_ERROR, "invalid LCD dimension; please choose from the "
                    "HD44780::Dimensions type");
            break;
    }
}
