/**
 * @file    l3g.c
 *
 * @author  David Zemon, Collin Winans
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

#include <l3g.h>

#define L3G_SPI_MODE            SPI_MODE_3
#define L3G_SPI_BITMODE         SPI_MSB_FIRST
#define L3G_SPI_DEFAULT_FREQ    100000

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

    // NOTE L3G has high- and low-pass filters. Should they be enabled? (Page
    // 31)
    checkErrors(L3GWrite8(L3G_CTRL_REG1, NIBBLE_0));
    checkErrors(L3GWrite8(L3G_CTRL_REG4, dpsMode | BIT_7));

    return 0;
}

void L3GAlwaysSetSPIMode (const uint8_t alwaysSetMode) {
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
    addr |= BIT_7;  // Set RW bit (
    addr |= BIT_6;  // Enable address auto-increment

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

uint8_t L3G_ioctl (const l3g_ioctl_function_t func, const uint8_t wrVal,
        uint8_t *rdVal) {
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
            return -1;  // TODO: Create a real error code
    }

    return 0;
}

/*************************************
 *** Private Function Declarations ***
 *************************************/
uint8_t L3GWrite8 (uint8_t addr, const uint8_t dat) {
    uint8_t err;
    uint16_t outputValue;

    addr &= ~BIT_7;  // Clear the RW bit (write mode)

    outputValue = ((uint16_t) addr) << 8;
    outputValue |= dat;

    if (g_l3g_alwaysSetMode) {
        checkErrors(SPISetMode(L3G_SPI_MODE));
        checkErrors(SPISetBitMode(L3G_SPI_BITMODE));
    }

    GPIOPinClear(g_l3g_cs);
    err = SPIShiftOut(16, outputValue);
    checkErrors(SPIWait());
    GPIOPinSet(g_l3g_cs);

    return err;
}

uint8_t L3GWrite16 (uint8_t addr, const uint16_t dat) {
    uint8_t err;
    uint16_t outputValue;

    addr &= ~BIT_7;  // Clear the RW bit (write mode)
    addr |= BIT_6;  // Enable address auto-increment

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

    addr |= BIT_7;  // Set RW bit (
    addr |= BIT_6;  // Enable address auto-increment

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

    addr |= BIT_7;  // Set RW bit (
    addr |= BIT_6;  // Enable address auto-increment

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
