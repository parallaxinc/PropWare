/* File:    l3g.c
 *
 * Author:  David Zemon
 *          Collin Winans
 */

#include <l3g.h>

#define L3G_WHO_AM_I		0x0F

#define L3G_CTRL_REG1		0x20
#define L3G_CTRL_REG2		0x21
#define L3G_CTRL_REG3		0x22
#define L3G_CTRL_REG4		0x23
#define L3G_CTRL_REG5		0x24
#define L3G_REFERENCE		0x25
#define L3G_OUT_TEMP		0x26
#define L3G_STATUS_REG		0x27

#define L3G_OUT_X_L			0x28
#define L3G_OUT_X_H			0x29
#define L3G_OUT_Y_L			0x2A
#define L3G_OUT_Y_H			0x2B
#define L3G_OUT_Z_L			0x2C
#define L3G_OUT_Z_H			0x2D

#define L3G_FIFO_CTRL_REG	0x2E
#define L3G_FIFO_SRC_REG	0x2F

#define L3G_INT1_CFG		0x30
#define L3G_INT1_SRC		0x31
#define L3G_INT1_THS_XH		0x32
#define L3G_INT1_THS_XL		0x33
#define L3G_INT1_THS_YH		0x34
#define L3G_INT1_THS_YL		0x35
#define L3G_INT1_THS_ZH		0x36
#define L3G_INT1_THS_ZL		0x37
#define L3G_INT1_DURATION	0x38

uint32_t g_l3g_cs;
uint8_t g_l3g_alwaysSetMode = 0;

uint8_t L3GStart (const uint32_t mosi, const uint32_t miso, const uint32_t sclk,
		const uint32_t cs, const l3g_dps_mode_t dpsMode) {
	uint8_t err;

	// Ensure SPI module started
	if (!SPIIsRunning()) {
		checkErrors(
				SPIStart(mosi, miso, sclk, L3G_SPI_DEFAULT_FREQ, L3G_SPI_MODE, L3G_SPI_BITMODE));

	} else {
		checkErrors(SPISetMode(L3G_SPI_MODE));
		checkErrors(SPISetBitMode(L3G_SPI_BITMODE));
	}

	g_l3g_cs = cs;
	GPIODirModeSet(cs, GPIO_DIR_OUT);
	GPIOPinSet(cs);

	// NOTE: L3G has high- and low-pass filters. Should they be enabled? (Page
	// 31)
	checkErrors(L3GWrite8(L3G_CTRL_REG1, NIBBLE_0));
	checkErrors(L3GWrite8(L3G_CTRL_REG4, dpsMode | BIT_7));

	return 0;
}

void L3GAlwaysSetMode (const uint8_t alwaysSetMode) {
	g_l3g_alwaysSetMode = alwaysSetMode;
}

uint8_t L3GReadX (int16_t *val) {
	return L3GRead16(L3G_OUT_X_L, val);
}

uint8_t L3GReadY (int16_t *val) {
	return L3GRead16(L3G_OUT_Y_L, val);
}

uint8_t L3GReadZ (int16_t *val) {
	return L3GRead16(L3G_OUT_Z_L, val);
}

uint8_t L3GRead (const l3g_axis axis, int16_t *val) {
	return L3GRead16(L3G_OUT_X_L + (axis << 1), val);
}

uint8_t L3GReadAll (int16_t *val) {
	uint8_t err, i;

	uint8_t addr = L3G_OUT_X_L;
	addr |= BIT_7; // Set RW bit (
	addr |= BIT_6; // Enable address auto-increment

	if (g_l3g_alwaysSetMode) {
		checkErrors(SPISetMode(L3G_SPI_MODE));
		checkErrors(SPISetBitMode(L3G_SPI_BITMODE));
	}

	GPIOPinClear(g_l3g_cs);
	checkErrors(SPIShiftOut(8, addr));
	checkErrors(SPIShiftIn(16, &(val[L3G_X]), sizeof(val[L3G_X])));
	checkErrors(SPIShiftIn(16, &(val[L3G_Y]), sizeof(val[L3G_Y])));
	checkErrors(SPIShiftIn(16, &(val[L3G_Z]), sizeof(val[L3G_Z])));
	GPIOPinSet(g_l3g_cs);

	// err is useless at this point and will be used as a temporary 8-bit
	// variable
	for (i = 0; i < 3; ++i) {
		err = val[i] >> 8;
		val[i] <<= 8;
		val[i] |= err;
	}

	return 0;
}

uint8_t L3G_ioctl (const l3g_ioctl_function_t func, const uint8_t wrVal, uint8_t *rdVal) {
	uint8_t err, oldValue;

	if (g_l3g_alwaysSetMode) {
		checkErrors(SPISetMode(L3G_SPI_MODE));
		checkErrors(SPISetBitMode(L3G_SPI_BITMODE));
	}

	switch (func) {
		// All functions follow the read-modify-write routine
		case L3G_FUNC_MOD_DPS:
			checkErrors(L3GRead8(L3G_CTRL_REG4, (int8_t *) &oldValue));
			oldValue &= ~(BIT_6 | BIT_5);
			oldValue |= wrVal << 5;
			checkErrors(L3GWrite8(L3G_CTRL_REG4, oldValue));
			break;
		case L3G_FUNC_RD_REG:
			checkErrors(L3GRead8(wrVal, (int8_t * ) rdVal));
			break;
		default:
			return -1; // TODO: Create a real error code
	}

	return 0;
}

/*************************************
 *** Private Function Declarations ***
 *************************************/
uint8_t L3GWrite8 (uint8_t addr, const uint8_t dat) {
	uint8_t err;
	uint16_t outputValue;

	addr &= ~BIT_7; // Clear the RW bit (write mode)

	outputValue = ((uint16_t) addr) << 8;
	outputValue |= dat;

	if (g_l3g_alwaysSetMode) {
		checkErrors(SPISetMode(L3G_SPI_MODE));
		checkErrors(SPISetBitMode(L3G_SPI_BITMODE));
	}

	GPIOPinClear(g_l3g_cs);
	err = SPIShiftOut(16, outputValue);
//	checkErrors(SPIWait());
	GPIOPinSet(g_l3g_cs);

	return err;
}

uint8_t L3GWrite16 (uint8_t addr, const uint16_t dat) {
	uint8_t err;
	uint16_t outputValue;

	addr &= ~BIT_7; // Clear the RW bit (write mode)
	addr |= BIT_6; // Enable address auto-increment

	outputValue = ((uint16_t) addr) << 16;
	outputValue |= ((uint16_t) ((uint8_t) dat)) << 8;
	outputValue |= (uint8_t) (dat >> 8);

	if (g_l3g_alwaysSetMode) {
		checkErrors(SPISetMode(L3G_SPI_MODE));
		checkErrors(SPISetBitMode(L3G_SPI_BITMODE));
	}

	GPIOPinClear(g_l3g_cs);
	checkErrors(SPIShiftOut(24, outputValue));
	checkErrors(SPIWait());
	GPIOPinSet(g_l3g_cs);

	return 0;
}

uint8_t L3GRead8 (uint8_t addr, int8_t *dat) {
	uint8_t err;

	addr |= BIT_7; // Set RW bit (
	addr |= BIT_6; // Enable address auto-increment

	if (g_l3g_alwaysSetMode) {
		checkErrors(SPISetMode(L3G_SPI_MODE));
		checkErrors(SPISetBitMode(L3G_SPI_BITMODE));
	}

	GPIOPinClear(g_l3g_cs);
	checkErrors(SPIShiftOut(8, addr));
	checkErrors(SPIShiftIn(8, dat, sizeof(*dat)));
	GPIOPinSet(g_l3g_cs);

	return 0;
}

uint8_t L3GRead16 (uint8_t addr, int16_t *dat) {
	uint8_t err;

	addr |= BIT_7; // Set RW bit (
	addr |= BIT_6; // Enable address auto-increment

	if (g_l3g_alwaysSetMode) {
		checkErrors(SPISetMode(L3G_SPI_MODE));
		checkErrors(SPISetBitMode(L3G_SPI_BITMODE));
	}

	GPIOPinClear(g_l3g_cs);
	checkErrors(SPIShiftOut(8, addr));
	checkErrors(SPIShiftIn(16, dat, sizeof(*dat)));
	GPIOPinSet(g_l3g_cs);

	// err is useless at this point and will be used as a temporary 8-bit
	// variable
	err = *dat >> 8;
	*dat <<= 8;
	*dat |= err;

	return 0;
}
