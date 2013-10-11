/* File:    hd44780.c
 *
 * Author:  David Zemon
 *          Collin Winans
 */

#include <hd44780.h>
#ifdef HD44780_DEBUG
#include <stdio.h>
#endif

static uint32_t g_hd44780_rs, g_hd44780_rw, g_hd44780_en;
static uint32_t g_hd44780_dataMask;
static hd44780_dimensions_t g_hd44780_dim;
static hd44780_bitmode_t g_hd44780_bitmode;
static uint8_t g_hd44780_dataLSBNum;
static uint8_t g_hd44780_pos = 0;

int8_t HD44780Start (const uint32_t dataPinsMask, const uint32_t rs,
		const uint32_t rw, const uint32_t en, const hd44780_bitmode_t bitmode,
		const hd44780_dimensions_t dimensions) {
	uint8_t i, bits;
	uint32_t tempMask;
	uint8_t arg;

#ifdef HD44780_DEBUG
	if (1 != PropWareCountBits(rw) || 1 != PropWareCountBits(rs)
			|| 1 != PropWareCountBits(en))
		return HD44780_INVALID_CTRL_SGNL;

	// Ensure either 4 or 8 bits were sent in for the data mask
	switch (bitmode) {
		case HD44780_8BIT:
			bits = 8;
			if (8 != PropWareCountBits(dataPinsMask))
				return HD44780_INVALID_DATA_MASK;
			break;
		case HD44780_4BIT:
			bits = 4;
			if (4 != PropWareCountBits(dataPinsMask))
				return HD44780_INVALID_DATA_MASK;
			break;
		default:
			return HD44780_INVALID_DATA_MASK;
	}

	// Ensure that all 4 or 8 bits are consecutive
	tempMask = dataPinsMask;
	while (0 == (BIT_0 & tempMask))
		tempMask >>= 1;
	for (i = 0; i < bits; ++i)
		if (0 == (BIT_0 & tempMask))
			return HD44780_INVALID_DATA_MASK;
		else
			tempMask >>= 1;

	if (HD44780_DIMENSIONS <= dimensions)
		return HD44780_INVALID_DIMENSIONS;
#endif

	// Save all control signal pin masks
	GPIODirModeSet(rs | rw | en, GPIO_DIR_OUT);
	g_hd44780_rs = rs;
	g_hd44780_rw = rw;
	g_hd44780_en = en;

	// Save data pin masks
	GPIODirModeSet(dataPinsMask, GPIO_DIR_OUT);
	g_hd44780_dataMask = dataPinsMask;

	// Determine the data LSB
	while (!(BIT_0 & (dataPinsMask >> g_hd44780_dataLSBNum)))
		g_hd44780_dataLSBNum++;

	// Save mode
	g_hd44780_dim = dimensions;
	g_hd44780_bitmode = bitmode;

	// Begin init routine:
	waitcnt(500 * MILLISECOND + CNT);
	HD44780Cmd(0x30);
	waitcnt(100 * MILLISECOND + CNT);
	HD44780ClockPulse();
	waitcnt(10 * MILLISECOND + CNT);
	HD44780ClockPulse();
	waitcnt(10 * MILLISECOND + CNT);

	// Default functions during initialization
	arg = HD44780_FUNCTION_SET;
	if (HD44780_8BIT == bitmode)
		arg |= HD44780_8BIT_MODE;
	if (0 != dimensions % 3)
		arg |= HD44780_2LINE_MODE;
	HD44780Cmd(arg | HD44780_5x10_CHAR); // Most LCDs are 5x10
	printf("1) 0x%02X\n", arg | HD44780_5x10_CHAR);

	// Turn off display shift (set cursor shift) and leave default of shift-left
	HD44780Cmd(HD44780_SHIFT);
	printf("2) 0x%02X\n", HD44780_SHIFT);

	// Turn the display on; Leave cursor off and not blinking
//	HD44780Cmd(HD44780_DISPLAY_CTRL | HD44780_DISPLAY_PWR);
	HD44780Cmd(HD44780_DISPLAY_CTRL | HD44780_DISPLAY_PWR | HD44780_CURSOR | HD44780_BLINK);
	printf("3) 0x%02X\n", HD44780_DISPLAY_CTRL | HD44780_DISPLAY_PWR | HD44780_CURSOR | HD44780_BLINK);

	// Set cursor to auto-increment upon writing a character
	HD44780Cmd(HD44780_ENTRY_MODE_SET | HD44780_SHIFT_INC);
	printf("4) 0x%02X\n", HD44780_ENTRY_MODE_SET | HD44780_SHIFT_INC);

	HD44780Clear();

	return 0;
}

void HD44780Clear (void) {
	HD44780Cmd(HD44780_CLEAR);
	g_hd44780_pos = 0;
}

void HD44780Move (const uint8_t row, const uint8_t col) {

}

void HD44780_putchar (const char c) {
	if ('\n' == c) {
		// TODO: Move the cursor down one line
	} else {

		//set RS to data and RW to write
		GPIOPinSet(g_hd44780_rs);
		HD44780Write(c);

		// Determine if line-wrap is necessary
	}
}

void HD44780_puts (char* s) {
	while (*(s++))
		HD44780_putchar(*s);
}

void HD44780Cmd (const uint8_t c) {
	//set RS to command mode and RW to write
	GPIOPinClear(g_hd44780_rs);
	HD44780Write(c);
}

void HD44780Write (const uint8_t val) {
	uint32_t shiftedVal = val;

	// Clear RW to signal write value
	GPIOPinClear(g_hd44780_rw);

	if (HD44780_4BIT == g_hd44780_bitmode) {
		// shift out the high nibble
		shiftedVal <<= g_hd44780_dataLSBNum - 4;
		GPIOPinWrite(g_hd44780_dataMask, shiftedVal);
		HD44780ClockPulse();
		shiftedVal <<= 4;
		GPIOPinWrite(g_hd44780_dataMask, shiftedVal);
		// Shift remaining four bits out
	} else /* Implied: if (HD44780_8BIT == g_hd44780_bitmode) */{
		shiftedVal <<= g_hd44780_dataLSBNum;
		GPIOPinWrite(g_hd44780_dataMask, shiftedVal);
	}
	HD44780ClockPulse();
}

inline void HD44780ClockPulse (void) {
	GPIOPinSet(g_hd44780_en);
	waitcnt(MILLISECOND + CNT);
	GPIOPinClear(g_hd44780_en);
}
