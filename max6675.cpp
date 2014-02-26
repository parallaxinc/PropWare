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

namespace PropWare {

MAX6675::MAX6675 () {
    this->m_cs = -1;
    this->m_alwaysSetMode = 0;
}

int8_t MAX6675::start (const PropWare::GPIO::Pin mosi,
        const PropWare::GPIO::Pin miso, const PropWare::GPIO::Pin clk,
        const PropWare::GPIO::Pin cs) {
    int8_t err;

    this->m_spi = SPI::getSPI();

    if (!this->m_spi->is_running()) {
        check_errors(
                this->m_spi->start(mosi, miso, sclk, MAX6675::SPI_DEFAULT_FREQ, MAX6675::SPI_MODE, MAX6675::SPI_BITMODE));
    } else {
        check_errors(this->m_spi->set_mode(MAX6675::SPI_MODE));
        check_errors(this->m_spi->set_bit_mode(MAX6675::SPI_BITMODE));
    }

    this->m_cs = cs;
    GPIO::set_dir(cs, GPIO::OUT);

    return 0;
}

void MAX6675::always_set_spi_mode (const bool alwaysSetMode) {
    this->m_alwaysSetMode = alwaysSetMode;
}

int8_t MAX6675::read (uint16_t *dat) {
    int8_t err;

    if (this->m_alwaysSetMode) {
        check_errors(this->m_spi->set_mode(MAX6675::SPI_MODE));
        check_errors(this->m_spi->set_bit_mode(MAX6675::SPI_BITMODE));
    }

    *dat = 0;
    GPIO::pin_clear(this->m_cs);
    check_errors(this->m_spi->shift_in(MAX6675::BIT_WIDTH, dat, sizeof(*dat)));
    GPIO::pin_set(this->m_cs);

    return 0;
}

int8_t MAX6675::read_whole (uint16_t *dat) {
    int8_t err;

    check_errors(this->read(dat));
    *dat >>= 2;

    return 0;
}

int8_t MAX6675::read_float (float *dat) {
    int8_t err;
    uint16_t temp;

    check_errors(this->read(&temp));

    *dat = temp >> 2;
    *dat += ((float) (temp & (BIT_1 | BIT_0))) / 4;

    return 0;
}

}
