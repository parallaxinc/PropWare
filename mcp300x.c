/**
 * @file    mcp300x.c
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

// Includes
#include <mcp300x.h>

#define MCP300X_START           BIT_4
#define MCP300X_SINGLE_ENDED    BIT_3
#define MCP300X_DIFFERENTIAL    0

#define MCP300X_OPTN_WIDTH      7
#define MCP300X_DATA_WIDTH      11

uint8_t g_mcp300x_cs;
uint8_t g_mcp300x_alwaysSetMode = 0;

int8_t mcp300x_start (const uint32_t mosi, const uint32_t miso,
        const uint32_t sclk, const uint32_t cs) {
    int8_t err;

    g_mcp300x_cs = cs;
    gpio_set_dir(cs, GPIO_DIR_OUT);
    gpio_pin_set(cs);

    if (!spi_is_running()) {
        check_errors(
                spi_start(mosi, miso, sclk, MCP300X_SPI_DEFAULT_FREQ, MCP300X_SPI_MODE, MCP300X_SPI_BITMODE));
    } else {
        check_errors(spi_set_mode(MCP300X_SPI_MODE));
        check_errors(spi_set_bit_mode(MCP300X_SPI_BITMODE));
    }

    return 0;
}

void mcp300x_always_set_spi_mode (const uint8_t alwaysSetMode) {
    g_mcp300x_alwaysSetMode = alwaysSetMode;
}

int8_t mcp300x_read (const MCP_Channel channel, uint16_t *dat) {
    int8_t err, options;

    options = MCP300X_START | MCP300X_SINGLE_ENDED | channel;
    options <<= 2; // One dead bit between output and input - see page 19 of datasheet

    if (g_mcp300x_alwaysSetMode) {
        check_errors(spi_set_mode(MCP300X_SPI_MODE));
        check_errors(spi_set_bit_mode(MCP300X_SPI_BITMODE));
    }

    gpio_pin_clear(g_mcp300x_cs);
    check_errors(spi_shift_out(MCP300X_OPTN_WIDTH, options));
    check_errors(spi_shift_in(MCP300X_DATA_WIDTH, dat, sizeof(*dat)));
    gpio_pin_set(g_mcp300x_cs);

    return 0;
}

int8_t mcp300x_read_diff (const MCP_ChannelDiff channels, uint16_t *dat) {
    int8_t err, options;

    options = MCP300X_START | MCP300X_DIFFERENTIAL | channels;
    options <<= 2; // One dead bit between output and input - see page 19 of datasheet

    if (g_mcp300x_alwaysSetMode) {
        check_errors(spi_set_mode(MCP300X_SPI_MODE));
        check_errors(spi_set_bit_mode(MCP300X_SPI_BITMODE));
    }

    gpio_pin_clear(g_mcp300x_cs);
    check_errors(spi_shift_out(MCP300X_OPTN_WIDTH, options));
    check_errors(spi_shift_in(MCP300X_DATA_WIDTH, dat, sizeof(*dat)));
    gpio_pin_set(g_mcp300x_cs);

    return 0;
}
