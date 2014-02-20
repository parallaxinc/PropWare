/**
 * @file    max6675.c
 *
 * @project PropWare
 *
 * @author  David Zemon
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

#include <max6675.h>

uint32_t g_max6675_cs = -1;
uint8_t g_max6675_alwaysSetMode = 0;

int8_t max6675_start (const uint32_t mosi, const uint32_t miso,
        const uint32_t sclk, const uint32_t cs) {
    int8_t err;

    if (!spi_is_running()) {
        check_errors(
                spi_start(mosi, miso, sclk, MAX6675_SPI_DEFAULT_FREQ, MAX6675_SPI_MODE, MAX6675_SPI_BITMODE));
    } else {
        check_errors(spi_set_mode(MAX6675_SPI_MODE));
        check_errors(spi_set_bit_mode(MAX6675_SPI_BITMODE));
    }

    g_max6675_cs = cs;
    gpio_set_dir(cs, GPIO_DIR_OUT);

    return 0;
}

void max6675_always_set_spi_mode (const uint8_t alwaysSetMode) {
    g_max6675_alwaysSetMode = alwaysSetMode;
}

int8_t max6675_read (uint16_t *dat) {
    int8_t err;

    if (g_max6675_alwaysSetMode) {
        check_errors(spi_set_mode(MAX6675_SPI_MODE));
        check_errors(spi_set_bit_mode(MAX6675_SPI_BITMODE));
    }

    *dat = 0;
    gpio_pin_clear(g_max6675_cs);
    check_errors(spi_shift_in(MAX6675_BIT_WIDTH, dat, sizeof(*dat)));
    gpio_pin_set(g_max6675_cs);

    return 0;
}

int8_t max6675_read_whole (uint16_t *dat) {
    int8_t err;

    check_errors(max6675_read(dat));
    *dat >>= 2;

    return 0;
}

int8_t max6675_read_float (float *dat) {
    int8_t err;
    uint16_t temp;

    check_errors(max6675_read(&temp));

    *dat = temp >> 2;
    *dat += ((float) (temp & (BIT_1 | BIT_0))) / 4;

    return 0;
}
