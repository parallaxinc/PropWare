/**
 * @file        hd44780.h
 */
/**
 * @author      David Zemon
 * @author      Collin Winans
 *
 * @brief       Support for the common "character LCD" modules making use of the
 *              HD44780 controller
 *
 * @note        Does not natively support 40x4 or 24x4 character displays
 *
 * @detailed
 */

/**
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
 * @defgroup _propware_hd44780_public	Public members
 * @{
 */

#include <stdarg.h>
#include <stdlib.h>
#include <propeller.h>
#include <PropWare.h>

#define HD44780_DEBUG

/** Number of spaces inserted for '\\t' */
#define HD44780_TAB_WIDTH               4

/**
 * @brief   LCD databus width
 */
typedef enum {
    HD44780_4BIT,
    HD44780_8BIT,
    HD44780_BITMODES
} hd44780_bitmode_t;

/**
 * @brief   Supported LCD dimensions; Used for determining cursor placement
 *
 * @note    There are two variations of 16x1 character LCDs; if you're unsure
 *          which version you have, try 16x1_1 first, it is more common. 16x1_1
 *          uses both DDRAM lines of the controller, 8-characters on each line;
 *          16x1_2 places all 16 characters are a single line of DDRAM.
 */
typedef enum {
    HD44780_8x1,
    HD44780_8x2,
    HD44780_8x4,
    HD44780_16x1_1,
    HD44780_16x1_2,
    HD44780_16x2,
    HD44780_16x4,
    HD44780_20x1,
    HD44780_20x2,
    HD44780_20x4,
    HD44780_24x1,
    HD44780_24x2,
    HD44780_40x1,
    HD44780_40x2,
    HD44780_DIMENSIONS
} hd44780_dimensions_t;

/**
 * @name    Error codes
 * @{
 */
#define HD44780_ERRORS_BASE             48
#define HD44780_ERRORS_LIMIT            16
#define HD44780_INVALID_CTRL_SGNL       HD44780_ERRORS_BASE + 0
#define HD44780_INVALID_DATA_MASK       HD44780_ERRORS_BASE + 1
#define HD44780_INVALID_DIMENSIONS      HD44780_ERRORS_BASE + 2
/**@}*/

/**
 * @name    Commands
 * @note    Must be combined with arguments below to create a parameter for the
 *          HD44780
 */
#define HD44780_CLEAR                   BIT_0
#define HD44780_RET_HOME                BIT_1
#define HD44780_ENTRY_MODE_SET          BIT_2
#define HD44780_DISPLAY_CTRL            BIT_3
#define HD44780_SHIFT                   BIT_4
#define HD44780_FUNCTION_SET            BIT_5
#define HD44780_SET_CGRAM_ADDR          BIT_6
#define HD44780_SET_DDRAM_ADDR          BIT_7
/**@}*/

/**
 * @name    Entry mode arguments
 * @{
 */
#define HD44780_SHIFT_INC               BIT_1
#define HD44780_SHIFT_EN                BIT_0
/**@}*/

/**
 * @name    Display control arguments
 * @{
 */
#define HD44780_DISPLAY_PWR             BIT_2
#define HD44780_CURSOR                  BIT_1
#define HD44780_BLINK                   BIT_0
/**@}*/

/**
 * @name    Cursor/display shift arguments
 * @{
 */
#define HD44780_SHIFT_DISPLAY           BIT_3 // 0 = shift cursor
#define HD44780_SHIFT_RIGHT             BIT_2 // 0 = shift left
 /**@}*/

/**
 * @name Function set arguments
 * @{
 */
#define HD44780_8BIT_MODE               BIT_4 // 0 = 4-bit mode
#define HD44780_2LINE_MODE              BIT_3 // 0 = 1-line mode
#define HD44780_5x10_CHAR               BIT_2 // 0 = 5x8 dot mode
/**@}*/

/************************
 *** Public Functions ***
 ************************/
/**
 * @brief   Initialize an HD44780 LCD display
 *
 * @param   dataPinsMask    Pin mask for all 4 or all 8 data wires; NOTE: all
 *                          pins must be consecutive and the LSB on the LCD must
 *                          be the LSB in your data mask (i.e., if you are using
 *                          pins 16-23 on the Propeller, pin 16 must be
 *                          connected to D0 on the LCD, NOT D7)
 * @param   rs, rw, en      Pin masks for each of the RS, RW, and EN signals
 * @param   bitmode         Select between HD44780_4BIT and HD44780_8BIT modes
 *                          to determine whether you will need 4 data wires
 *                          or 8 between the Propeller and your LCD device
 * @param   dimensions        Dimensions of your LCD device. Most common is
 *                          HD44780_16x2
 */
int8_t HD44780Start (const uint32_t dataPinsMask, const uint32_t rs,
        const uint32_t rw, const uint32_t en, const hd44780_bitmode_t bitmode,
        const hd44780_dimensions_t dimensions);

/**
 * @brief   Clear the LCD display and return cursor to home
 */
inline void HD44780Clear (void);

/**
 * @brief   Move the cursor to a specified column and row
 *
 * @param   row     Zero-indexed row to place the cursor
 * @param   col     Zero indexed column to place the cursor
 */
void HD44780Move (const uint8_t row, const uint8_t col);

/**
 * @brief       Print formatted text to the LCD
 *
 * @detailed    Please visit http://www.cplusplus.com/reference/cstdio/printf/
 *              for full documentation
 *
 * @param       *fmt    C string that contains the text to be written to the LCD
 * @param       ...     (optional) Additional arguments
 *
 * Supported formats: i, d, u, X, s, c ("%%" will print '%')
 */
void HD44780_printf (char *fmt, ...);

/**
 * @brief       Print a string to the LCD
 *
 * @detailed    Via a series of calls to HD44780_putchar, prints each character
 *              individually
 *
 * @param       *s  Address where c-string can be found (must be
 *                  null-terminated)
 */
void HD44780_puts (char *s);

/**
 * @brief   Print a single char to the LCD and increment the pointer (automatic)
 *
 * @param   c   Individual char to be printed
 */
void HD44780_putchar (const char c);

/**
 * @brief   Format and print a signed number to the LCD
 *
 * @param   x   The signed number to be printed
 */
void HD44780_int (int32_t x);

/**
 * @brief   Format and print a unsigned number to the LCD
 *
 * @param   x   The unsigned number to be printed
 */
void HD44780_uint (uint32_t x);

/**
 * @brief   Format and print an unsigned hexadecimal number to the LCD
 *
 * @param   x   The number to be printed
 */
void HD44780_hex (uint32_t x);

/**@}*/

/*************************
 *** Private Functions ***
 *************************/
/**
 * @defgroup _propware_hd44780_private  Private members
 * @{
 */
typedef struct {
        uint8_t charRows;
        uint8_t charColumns;
        uint8_t ddramCharRowBreak;
        uint8_t ddramLineEnd;
} hd44780_mem_map_t;

/**
 * @brief   Send a control command to the LCD module
 *
 * @param   c   8-bit command to send to the LCD
 */
inline static void HD44780Cmd (const uint8_t c);

static void HD44780Write (const uint8_t val);

/**
 * @brief   Toggle the enable pin, inducing a write to the LCD's register
 */
static void HD44780ClockPulse (void);

static void HD44780GenerateMemMap (const hd44780_dimensions_t dimensions);

/**@}*/

/**@}*/

#endif /* HD44780_H_ */
