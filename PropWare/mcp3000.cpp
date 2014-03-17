/**
 * @file    mcp300x.cpp
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
#include <PropWare/mcp3000.h>

namespace PropWare {

MCP3000::MCP3000 (SPI *spi, PropWare::MCP3000::PartNumber partNumber) {
    this->m_spi = spi;
    this->m_alwaysSetMode = 0;
    this->m_dataWidth = partNumber;
}

PropWare::ErrorCode MCP3000::start (const PropWare::GPIO::Pin mosi,
        const PropWare::GPIO::Pin miso, const PropWare::GPIO::Pin sclk,
        const PropWare::GPIO::Pin cs) {
    PropWare::ErrorCode err;

    this->m_cs = cs;
    GPIO::set_dir(cs, GPIO::OUT);
    GPIO::pin_set(cs);

    if (!this->m_spi->is_running()) {
        check_errors(
                this->m_spi->start(mosi, miso, sclk, MCP3000::SPI_DEFAULT_FREQ, MCP3000::SPI_MODE, MCP3000::SPI_BITMODE));
    } else {
        check_errors(this->m_spi->set_mode(MCP3000::SPI_MODE));
        check_errors(this->m_spi->set_bit_mode(MCP3000::SPI_BITMODE));
    }

    return 0;
}

void MCP3000::always_set_spi_mode (const bool alwaysSetMode) {
    this->m_alwaysSetMode = alwaysSetMode;
}

PropWare::ErrorCode MCP3000::read (const MCP3000::Channel channel, uint16_t *dat) {
    PropWare::ErrorCode err;
    int8_t options;

    options = MCP3000::START | MCP3000::SINGLE_ENDED | channel;
    options <<= 2; // One dead bit between output and input - see page 19 of datasheet

    if (this->m_alwaysSetMode) {
        check_errors(this->m_spi->set_mode(MCP3000::SPI_MODE));
        check_errors(this->m_spi->set_bit_mode(MCP3000::SPI_BITMODE));
    }

    GPIO::pin_clear(this->m_cs);
    check_errors(this->m_spi->shift_out(MCP3000::OPTN_WIDTH, options));
    check_errors(this->m_spi->shift_in(this->m_dataWidth, dat, sizeof(*dat)));
    GPIO::pin_set(this->m_cs);

    return 0;
}

PropWare::ErrorCode MCP3000::read_diff (const MCP3000::ChannelDiff channels, uint16_t *dat) {
    int8_t err, options;

    options = MCP3000::START | MCP3000::DIFFERENTIAL | channels;
    options <<= 2; // One dead bit between output and input - see page 19 of datasheet

    if (this->m_alwaysSetMode) {
        check_errors(this->m_spi->set_mode(MCP3000::SPI_MODE));
        check_errors(this->m_spi->set_bit_mode(MCP3000::SPI_BITMODE));
    }

    GPIO::pin_clear(this->m_cs);
    check_errors(this->m_spi->shift_out(MCP3000::OPTN_WIDTH, options));
    check_errors(this->m_spi->shift_in(this->m_dataWidth, dat, sizeof(*dat)));
    GPIO::pin_set(this->m_cs);

    return 0;
}

}
