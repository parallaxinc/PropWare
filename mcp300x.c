/**
 * @file    mcp300x.c
 *
 * @author  David Zemon, Collin Winans
 */

/**
 * @copyright
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013 David Zemon
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

// Includes
#include <mcp300x.h>

#define MCP300X_START           BIT_4
#define MCP300X_SINGLE_ENDED    BIT_3
#define MCP300X_DIFFERENTIAL    0

#define MCP300X_OPTN_WIDTH      7
#define MCP300X_DATA_WIDTH      11

uint8_t g_mcp300x_cs;
uint8_t g_mcp300x_alwaysSetMode = 0;

int8_t MCP300xStart (const uint32_t mosi, const uint32_t miso,
        const uint32_t sclk, const uint32_t cs) {
    int8_t err;

    g_mcp300x_cs = cs;
    GPIODirModeSet(cs, GPIO_DIR_OUT);
    GPIOPinSet(cs);

    if (!SPIIsRunning()) {
        checkErrors(
                SPIStart(mosi, miso, sclk, MCP300X_SPI_DEFAULT_FREQ, MCP300X_SPI_MODE, MCP300X_SPI_BITMODE));
    } else {
        checkErrors(SPISetMode(MCP300X_SPI_MODE));
        checkErrors(SPISetBitMode(MCP300X_SPI_BITMODE));
    }

    return 0;
}

void MCP300xAlwaysSetMode (const uint8_t alwaysSetMode) {
    g_mcp300x_alwaysSetMode = alwaysSetMode;
}

int8_t MCP300xRead (const mcp_channel_t channel, uint16_t *dat) {
    int8_t err, options;

    options = MCP300X_START | MCP300X_SINGLE_ENDED | channel;
    options <<= 2; // One dead bit between output and input - see page 19 of datasheet

    if (g_mcp300x_alwaysSetMode) {
        checkErrors(SPISetMode(MCP300X_SPI_MODE));
        checkErrors(SPISetBitMode(MCP300X_SPI_BITMODE));
    }

    GPIOPinClear(g_mcp300x_cs);
    checkErrors(SPIShiftOut(MCP300X_OPTN_WIDTH, options));
    checkErrors(SPIShiftIn(MCP300X_DATA_WIDTH, dat, sizeof(*dat)));
    GPIOPinSet(g_mcp300x_cs);

    return 0;
}

int8_t MCP300xReadDif (const mcp_channel_diff_t channels, uint16_t *dat) {
    int8_t err, options;

    options = MCP300X_START | MCP300X_DIFFERENTIAL | channels;
    options <<= 2; // One dead bit between output and input - see page 19 of datasheet

    if (g_mcp300x_alwaysSetMode) {
        checkErrors(SPISetMode(MCP300X_SPI_MODE));
        checkErrors(SPISetBitMode(MCP300X_SPI_BITMODE));
    }

    GPIOPinClear(g_mcp300x_cs);
    checkErrors(SPIShiftOut(MCP300X_OPTN_WIDTH, options));
    checkErrors(SPIShiftIn(MCP300X_DATA_WIDTH, dat, sizeof(*dat)));
    GPIOPinSet(g_mcp300x_cs);

    return 0;
}
