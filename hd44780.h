/**
 * @file        hd44780.h
 */
/**
 * @brief       Support for the common "character LCD" modules using the HD44780
 *              controller for the Parallax Propeller
 *
 * @project     PropWare
 *
 * @author      David Zemon
 * @author      Collin Winans
 *
 * @note        Does not natively support 40x4 or 24x4 character displays
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

#ifndef HD44780_H_
#define HD44780_H_

/**
 * @defgroup _propware_hd44780	HD44780 Character LCD
 * @{
 */

/**
 * @publicsection @{
 */

#include <stdarg.h>
#include <stdlib.h>
#include <propeller.h>
#include <PropWare.h>

/** @name   HD44780 Extra Code Options
 * @{ */
/**
 * Enable or disable error checking on parameters
 * <p>
 * DEFAULT: On
 */
#define HD44780_OPTION_DEBUG
/** @} */

class HD44780 {
    public:
        /**
         * @brief   LCD databus width
         */
        typedef enum {
            BM_4,
            BM_8,
            BITMODES
        } Bitmode;

        /**
         * @brief   Supported LCD dimensions; Used for determining cursor placement
         *
         * @note    There are two variations of 16x1 character LCDs; if you're unsure
         *          which version you have, try 16x1_1 first, it is more common. 16x1_1
         *          uses both DDRAM lines of the controller, 8-characters on each line;
         *          16x1_2 places all 16 characters are a single line of DDRAM.
         */
        typedef enum {
            /** 8x1 */DIM_8x1,
            /** 8x2 */DIM_8x2,
            /** 8x4 */DIM_8x4,
            /** 16x1 mode 1 */DIM_16x1_1,
            /** 16x1 mode 2 */DIM_16x1_2,
            /** 16x2 */DIM_16x2,
            /** 16x4 */DIM_16x4,
            /** 20x1 */DIM_20x1,
            /** 20x2 */DIM_20x2,
            /** 20x4 */DIM_20x4,
            /** 24x1 */DIM_24x1,
            /** 24x2 */DIM_24x2,
            /** 40x1 */DIM_40x1,
            /** 40x2 */DIM_40x2,
            /** Number of different dimensions supported */DIMENSIONS
        } Dimensions;

        /** Number of allocated error codes for HD44780 */
#define HD44780_ERRORS_LIMIT            16
        /** First HD44780 error code */
#define HD44780_ERRORS_BASE             48

        /**
         * Error codes - Proceeded by SD, SPI
         */
        typedef enum {
            /** HD44780 Error 0 */INVALID_CTRL_SGNL = HD44780_ERRORS_BASE,
            /** HD44780 Error 1 */INVALID_DATA_MASK,
            /** HD44780 Error 2 */INVALID_DIMENSIONS
        } ErrorCode;

    public:
        /** Number of spaces inserted for '\\t' */
        static const uint8_t TAB_WIDTH;

        /**
         * @name    Commands
         * @note    Must be combined with arguments below to create a parameter for the
         *          HD44780
         */
        static const uint8_t CLEAR;
        static const uint8_t RET_HOME;
        static const uint8_t ENTRY_MODE_SET;
        static const uint8_t DISPLAY_CTRL;
        static const uint8_t SHIFT;
        static const uint8_t FUNCTION_SET;
        static const uint8_t SET_CGRAM_ADDR;
        static const uint8_t SET_DDRAM_ADDR;
        /**@}*/

        /**
         * @name    Entry mode arguments
         * @{
         */
        static const uint8_t SHIFT_INC;
        static const uint8_t SHIFT_EN;
        /**@}*/

        /**
         * @name    Display control arguments
         * @{
         */
        static const uint8_t DISPLAY_PWR;
        static const uint8_t CURSOR;
        static const uint8_t BLINK;
        /**@}*/

        /**
         * @name    Cursor/display shift arguments
         * @{
         */
        static const uint8_t SHIFT_DISPLAY;  // 0 = shift cursor
        static const uint8_t SHIFT_RIGHT;  // 0 = shift left
        /**@}*/

        /**
         * @name Function set arguments
         * @{
         */
        static const uint8_t FUNC_8BIT_MODE;  // 0 = 4-bit mode
        static const uint8_t FUNC_2LINE_MODE;  // 0 = 1-line mode
        static const uint8_t FUNC_5x10_CHAR;  // 0 = 5x8 dot mode
        /**@}*/

        /************************
         *** Public Functions ***
         ************************/
    public:
        HD44780 ();

        /**
         * @brief       Initialize an HD44780 LCD display
         *
         * @param[in]   dataPinsMask    Pin mask for all 4 or all 8 data wires; NOTE:
         *                              all pins must be consecutive and the LSB on the
         *                              LCD must be the LSB in your data mask (i.e., if
         *                              you are using pins 16-23 on the Propeller, pin
         *                              16 must be connected to D0 on the LCD, NOT D7)
         * @param[in]   rs, rw, en      Pin masks for each of the RS, RW, and EN signals
         * @param[in]   bitmode         Select between HD44780_4BIT and HD44780_8BIT
         *                              modes to determine whether you will need 4 data
         *                              wires or 8 between the Propeller and your LCD
         *                              device
         * @param[in]   dimensions      Dimensions of your LCD device. Most common is
         *                              HD44780_16x2
         *
         * @return      Returns 0 upon success, otherwise error code
         */
        int8_t start (const uint32_t dataPinsMask, const uint32_t rs,
                const uint32_t rw, const uint32_t en,
                const HD44780::Bitmode bitmode,
                const HD44780::Dimensions dimensions);

        /**
         * @brief   Clear the LCD display and return cursor to home
         */
        void clear (void);

        /**
         * @brief       Move the cursor to a specified column and row
         *
         * @param[in]   row     Zero-indexed row to place the cursor
         * @param[in]   col     Zero indexed column to place the cursor
         */
        void move (const uint8_t row, const uint8_t col);

        /**
         * @brief       Print a string to the LCD
         *
         * @detailed    Via a series of calls to HD44780_putchar, prints each character
         *              individually
         *
         * @param[in]   *s  Address where c-string can be found (must be
         *                  null-terminated)
         */
        void putStr (const char str[]);

        /**
         * @brief       Print a single char to the LCD and increment the pointer
         *              (automatic)
         *
         * @param[in]   c   Individual char to be printed
         */
        void putChar (const char c);

    private:
        /**
         * Store metadata on the LCD device to determine when line-wraps should and
         * shouldn't occur
         */
        typedef struct {
                /** How many characters can be displayed on a single row */
                uint8_t charRows;
                /** How many characters can be dispayed in a single column */
                uint8_t charColumns;
                /** How many contiguous bytes of memory per visible character row */
                uint8_t ddramCharRowBreak;
                /** Last byte of memory used in each DDRAM line */
                uint8_t ddramLineEnd;
        } MemMap;

        /*************************
         *** Private Functions ***
         *************************/
    private:
        /**
         * @brief      Send a control command to the LCD module
         *
         * @param[in]  c   8-bit command to send to the LCD
         */
        void cmd (const uint8_t c);

        /**
         * @brief       Write a single byte to the LCD - instruction or data
         *
         * @param[in]   val     Value to be written
         */
        void write (const uint8_t val);

        /**
         * @brief   Toggle the enable pin, inducing a write to the LCD's register
         */
        void clock_pulse (void);

        /**
         * @brief   The memory map is used to determine where line wraps should and
         *          shouldn't occur
         */
        void generate_mem_map (const HD44780::Dimensions dimensions);

    private:
        uint32_t m_rs, m_rw, m_en;
        uint32_t m_dataMask;
        HD44780::Dimensions m_dim;
        HD44780::Bitmode m_bitmode;
        uint8_t m_dataLSBNum;
        uint8_t m_curRow;
        uint8_t m_curCol;
        HD44780::MemMap m_memMap;
};

#endif /* HD44780_H_ */
