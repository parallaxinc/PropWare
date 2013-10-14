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
static uint8_t g_hd44780_curRow = 0;
static uint8_t g_hd44780_curCol = 0;
static hd44780_mem_map_t g_hd44780_memMap;

int8_t HD44780Start (const uint32_t dataPinsMask, const uint32_t rs,
		const uint32_t rw, const uint32_t en, const hd44780_bitmode_t bitmode,
		const hd44780_dimensions_t dimensions) {
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

	// Wait for a couple years until the LCD has done internal initialization
	waitcnt(250 * MILLISECOND + CNT);

	// Save all control signal pin masks
	g_hd44780_rs = rs;
	g_hd44780_rw = rw;
	g_hd44780_en = en;
	GPIODirModeSet(rs | rw | en, GPIO_DIR_OUT);
	GPIOPinClear(rs | rw | en);

	// Save data pin masks
	GPIODirModeSet(dataPinsMask, GPIO_DIR_OUT);
	g_hd44780_dataMask = dataPinsMask;

	// Determine the data LSB
	while (!(BIT_0 & (dataPinsMask >> g_hd44780_dataLSBNum)))
		g_hd44780_dataLSBNum++;

	// Save the modes
	g_hd44780_dim = dimensions;
	HD44780GenerateMemMap(dimensions);
	g_hd44780_bitmode = bitmode;

	// Begin init routine:
	if (HD44780_8BIT == bitmode)
		arg = 0x30;
	else
		/* Implied: "if (HD44780_4BIT == bitmode)" */
		arg = 0x3;
	arg <<= g_hd44780_dataLSBNum;

	GPIOPinWrite(g_hd44780_dataMask, arg);
	HD44780ClockPulse();
	waitcnt(100 * MILLISECOND + CNT);

	HD44780ClockPulse();
	waitcnt(100 * MILLISECOND + CNT);

	HD44780ClockPulse();
	waitcnt(10 * MILLISECOND + CNT);


	if (HD44780_4BIT == bitmode) {
		GPIOPinWrite(g_hd44780_dataMask, 0x2 << g_hd44780_dataLSBNum);
		HD44780ClockPulse();
	}

	// Default functions during initialization
	arg = HD44780_FUNCTION_SET;
	if (HD44780_8BIT == bitmode)
		arg |= HD44780_8BIT_MODE;
	if (0 != dimensions % 3)
		arg |= HD44780_2LINE_MODE;
	HD44780Cmd(arg);

	// Turn off display shift (set cursor shift) and leave default of shift-left
	HD44780Cmd(HD44780_SHIFT);

	// Turn the display on; Leave cursor off and not blinking
	//	HD44780Cmd(HD44780_DISPLAY_CTRL | HD44780_DISPLAY_PWR);
	arg = HD44780_DISPLAY_CTRL | HD44780_DISPLAY_PWR | HD44780_CURSOR
			| HD44780_BLINK;
	HD44780Cmd(arg);

	// Set cursor to auto-increment upon writing a character
	arg = HD44780_ENTRY_MODE_SET | HD44780_SHIFT_INC;
	HD44780Cmd(arg);

	HD44780Clear();

	return 0;
}

inline void HD44780Clear (void) {
	HD44780Cmd(HD44780_CLEAR);
	g_hd44780_curRow = 0;
	g_hd44780_curCol = 0;
	waitcnt(1530*MICROSECOND + CNT);
}

void HD44780Move (const uint8_t row, const uint8_t col) {
	uint8_t ddramLine, addr = 0;

	// Handle weird special case where a single row LCD is split across
	// multiple DDRAM lines (i.e., 16x1 type 1)
	if (g_hd44780_memMap.ddramCharRowBreak > g_hd44780_memMap.ddramLineEnd) {
		ddramLine = col / g_hd44780_memMap.ddramLineEnd;
		if (ddramLine)
			addr = 0x40;
		addr |= col % g_hd44780_memMap.ddramLineEnd;
	} else if (4 == g_hd44780_memMap.charRows) {
		// Determine DDRAM line
		if (row % 2)
			addr = 0x40;
		if (row / 2)
			addr += g_hd44780_memMap.ddramCharRowBreak;
		addr += col % g_hd44780_memMap.ddramCharRowBreak;

	} else /* implied: "if (2 == memMap.charRows)" */{
		if (row)
			addr = 0x40;
		addr |= col;
	}

	HD44780Cmd(addr | HD44780_SET_DDRAM_ADDR);
	g_hd44780_curRow = row;
	g_hd44780_curCol = col;
}

void HD44780_putchar (const char c) {
	// For manual new-line characters...
	if ('\n' == c) {
		if (++g_hd44780_curRow == g_hd44780_memMap.charRows)
			g_hd44780_curRow = 0;
		g_hd44780_curCol = 0;
		HD44780Move(g_hd44780_curRow, g_hd44780_curCol);
	}
	// And for everything else...
	else {
		//set RS to data and RW to write
		GPIOPinSet(g_hd44780_rs);
		HD44780Write(c);

		// Insert a line wrap if necessary
		++g_hd44780_curCol;
		if (g_hd44780_memMap.charColumns == g_hd44780_curCol)
			HD44780_putchar('\n');

		// Handle weird special case where a single row LCD is split across
		// multiple DDRAM lines (i.e., 16x1 type 1)
		if (g_hd44780_memMap.ddramCharRowBreak > g_hd44780_memMap.ddramLineEnd)
			HD44780Move(g_hd44780_curRow, g_hd44780_curCol);
	}
}

void HD44780_puts (char* s) {
	while (*s) {
		HD44780_putchar(*s);
		++s;
	}
}

inline void HD44780Cmd (const uint8_t c) {
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

		// Shift out low nibble
		shiftedVal <<= 4;
		GPIOPinWrite(g_hd44780_dataMask, shiftedVal);
	}
	// Shift remaining four bits out
	else /* Implied: if (HD44780_8BIT == g_hd44780_bitmode) */{
		shiftedVal <<= g_hd44780_dataLSBNum;
		GPIOPinWrite(g_hd44780_dataMask, shiftedVal);
	}
	HD44780ClockPulse();
}

void HD44780ClockPulse (void) {
	GPIOPinSet(g_hd44780_en);
	waitcnt(MILLISECOND + CNT);
	GPIOPinClear(g_hd44780_en);
}

void HD44780GenerateMemMap (const hd44780_dimensions_t dimensions) {
	switch (dimensions) {
		case HD44780_8x1:
			g_hd44780_memMap.charRows = 1;
			g_hd44780_memMap.charColumns = 8;
			g_hd44780_memMap.ddramCharRowBreak = 8;
			g_hd44780_memMap.ddramLineEnd = 8;
			break;
		case HD44780_8x2:
			g_hd44780_memMap.charRows = 2;
			g_hd44780_memMap.charColumns = 8;
			g_hd44780_memMap.ddramCharRowBreak = 8;
			g_hd44780_memMap.ddramLineEnd = 8;
			break;
		case HD44780_8x4:
			g_hd44780_memMap.charRows = 4;
			g_hd44780_memMap.charColumns = 8;
			g_hd44780_memMap.ddramCharRowBreak = 8;
			g_hd44780_memMap.ddramLineEnd = 16;
			break;
		case HD44780_16x1_1:
			g_hd44780_memMap.charRows = 1;
			g_hd44780_memMap.charColumns = 16;
			g_hd44780_memMap.ddramCharRowBreak = 8;
			g_hd44780_memMap.ddramLineEnd = 8;
			break;
		case HD44780_16x1_2:
			g_hd44780_memMap.charRows = 1;
			g_hd44780_memMap.charColumns = 16;
			g_hd44780_memMap.ddramCharRowBreak = 16;
			g_hd44780_memMap.ddramLineEnd = 16;
			break;
		case HD44780_16x2:
			g_hd44780_memMap.charRows = 2;
			g_hd44780_memMap.charColumns = 16;
			g_hd44780_memMap.ddramCharRowBreak = 16;
			g_hd44780_memMap.ddramLineEnd = 16;
			break;
		case HD44780_16x4:
			g_hd44780_memMap.charRows = 4;
			g_hd44780_memMap.charColumns = 16;
			g_hd44780_memMap.ddramCharRowBreak = 16;
			g_hd44780_memMap.ddramLineEnd = 32;
			break;
		case HD44780_20x1:
			g_hd44780_memMap.charRows = 1;
			g_hd44780_memMap.charColumns = 20;
			g_hd44780_memMap.ddramCharRowBreak = 20;
			g_hd44780_memMap.ddramLineEnd = 20;
			break;
		case HD44780_20x2:
			g_hd44780_memMap.charRows = 2;
			g_hd44780_memMap.charColumns = 20;
			g_hd44780_memMap.ddramCharRowBreak = 20;
			g_hd44780_memMap.ddramLineEnd = 20;
			break;
		case HD44780_20x4:
			g_hd44780_memMap.charRows = 4;
			g_hd44780_memMap.charColumns = 8;
			g_hd44780_memMap.ddramCharRowBreak = 8;
			g_hd44780_memMap.ddramLineEnd = 20;
			break;
		case HD44780_24x1:
			g_hd44780_memMap.charRows = 1;
			g_hd44780_memMap.charColumns = 24;
			g_hd44780_memMap.ddramCharRowBreak = 24;
			g_hd44780_memMap.ddramLineEnd = 24;
			break;
		case HD44780_24x2:
			g_hd44780_memMap.charRows = 2;
			g_hd44780_memMap.charColumns = 24;
			g_hd44780_memMap.ddramCharRowBreak = 24;
			g_hd44780_memMap.ddramLineEnd = 24;
			break;
		case HD44780_40x1:
			g_hd44780_memMap.charRows = 1;
			g_hd44780_memMap.charColumns = 40;
			g_hd44780_memMap.ddramCharRowBreak = 40;
			g_hd44780_memMap.ddramLineEnd = 40;
			break;
		case HD44780_40x2:
			g_hd44780_memMap.charRows = 2;
			g_hd44780_memMap.charColumns = 40;
			g_hd44780_memMap.ddramCharRowBreak = 40;
			g_hd44780_memMap.ddramLineEnd = 40;
			break;
		default:
			break;
	}
}
