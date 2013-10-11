/* File:    hd44780.h
 *
 * Author:  David Zemon
 *          Collin Winans
 *
 * Description: TODO: Do me
 */

#ifndef HD44780_H_
#define HD44780_H_

#include <propeller.h>
#include <PropWare.h>

#define HD44780_DEBUG

// LCD parameters
typedef enum {
	HD44780_4BIT,
	HD44780_8BIT,
	HD44780_BITMODES
} hd44780_bitmode_t;

// Possible LCD dimensions
typedef enum {
	HD44780_8x1,
	HD44780_8x2,
	HD44780_8x4,
	HD44780_16x1,
	HD44780_16x2,
	HD44780_16x4,
	HD44780_20x1,
	HD44780_20x2,
	HD44780_20x4,
	HD44780_24x1,
	HD44780_24x2,
	HD44780_24x4,
	HD44780_40x1,
	HD44780_40x2,
	HD44780_40x4,
	HD44780_DIMENSIONS
} hd44780_dimensions_t;

// Errors
#define HD44780_ERRORS_BASE				48
#define HD44780_ERRORS_LIMIT			16
#define HD44780_INVALID_CTRL_SGNL		HD44780_ERRORS_BASE + 0
#define HD44780_INVALID_DATA_MASK		HD44780_ERRORS_BASE + 1
#define HD44780_INVALID_DIMENSIONS		HD44780_ERRORS_BASE + 2

// Commands; NOTE: must be OR-ed with arguments below
#define HD44780_CLEAR					BIT_0
#define HD44780_RET_HOME				BIT_1
#define HD44780_ENTRY_MODE_SET			BIT_2
#define HD44780_DISPLAY_CTRL			BIT_3
#define HD44780_SHIFT					BIT_4
#define HD44780_FUNCTION_SET			BIT_5
#define HD44780_SET_CGRAM_ADDR			BIT_6
#define HD44780_SET_DDRAM_ADDR			BIT_7

// Entry mode arguments
#define HD44780_SHIFT_INC				BIT_1
#define HD44780_SHIFT_EN				BIT_0

// Display control arguments
#define HD44780_DISPLAY_PWR				BIT_2
#define HD44780_CURSOR					BIT_1
#define HD44780_BLINK					BIT_0

// Cursor/display shift arguments
#define HD44780_SHIFT_DISPLAY			BIT_3 // 0 = shift cursor
#define HD44780_SHIFT_RIGHT				BIT_2 // 0 = shift left
// Function set arguments
#define HD44780_8BIT_MODE				BIT_4 // 0 = 4-bit mode
#define HD44780_2LINE_MODE				BIT_3 // 0 = 1-line mode
#define HD44780_5x10_CHAR				BIT_2 // 0 = 5x8 dot mode
/************************
 *** Public Functions ***
 ************************/
/**
 * \brief   Initialize an HD44780 LCD display
 *
 * \param   dataPinsMask    Pin mask for all 4 or all 8 data wires; NOTE: all
 *                          pins must be consecutive and the LSB on the LCD must
 *                          be the LSB in your data mask (i.e., if you are using
 *                          pins 16-23 on the Propeller, pin 16 must be
 *                          connected to D0 on the LCD, NOT D7)
 * \param   rs, rw, en      Pin masks for each of the RS, RW, and EN signals
 * \param   bitmode         Select between HD44780_4BIT and HD44780_8BIT modes
 *                          to determine whether you will need 4 data wires
 *                          or 8 between the Propeller and your LCD device
 * \param   dimensions		Dimensions of your LCD device. Most common is
 *                          HD44780_16x2
 */
int8_t HD44780Start (const uint32_t dataPinsMask, const uint32_t rs,
		const uint32_t rw, const uint32_t en, const hd44780_bitmode_t bitmode,
		const hd44780_dimensions_t dimensions);

void HD44780Clear (void);

void HD44780Move (const uint8_t row, const uint8_t col);

void HD44780_puts (char *s);

void HD44780_putchar (const char c);

static void HD44780Cmd (const uint8_t c);

static void HD44780Write (const uint8_t val);

static inline void HD44780ClockPulse (void);
#endif /* HD44780_H_ */
