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

PropWare::MCP3000::MCP3000 (SPI *spi, PropWare::MCP3000::PartNumber partNumber) {
    this->m_spi = spi;
    this->m_alwaysSetMode = 0;
    this->m_dataWidth = partNumber;
}

PropWare::ErrorCode PropWare::MCP3000::start (const PropWare::Pin::Mask mosi,
        const PropWare::Pin::Mask miso, const PropWare::Pin::Mask sclk,
        const PropWare::Pin::Mask cs) {
    PropWare::ErrorCode err;

    this->m_cs.set_mask(cs);
    this->m_cs.set_dir(PropWare::Pin::OUT);
    this->m_cs.set();

    if (!this->m_spi->is_running()) {
        check_errors(
                this->m_spi->start(mosi, miso, sclk,
                        PropWare::MCP3000::SPI_DEFAULT_FREQ,
                        PropWare::MCP3000::SPI_MODE,
                        PropWare::MCP3000::SPI_BITMODE));
    } else {
        check_errors(this->m_spi->set_mode(PropWare::MCP3000::SPI_MODE));
        check_errors(this->m_spi->set_bit_mode(PropWare::MCP3000::SPI_BITMODE));
    }

    return 0;
}

void PropWare::MCP3000::always_set_spi_mode (const bool alwaysSetMode) {
    this->m_alwaysSetMode = alwaysSetMode;
}

PropWare::ErrorCode PropWare::MCP3000::read (
        const PropWare::MCP3000::Channel channel, uint16_t *dat) {
    PropWare::ErrorCode err;
    int8_t options;

    options = PropWare::MCP3000::START | PropWare::MCP3000::SINGLE_ENDED
            | channel;
    options <<= 2; // One dead bit between output and input - see page 19 of datasheet

    if (this->m_alwaysSetMode) {
        check_errors(this->m_spi->set_mode(MCP3000::SPI_MODE));
        check_errors(this->m_spi->set_bit_mode(MCP3000::SPI_BITMODE));
    }

    this->m_cs.clear();
    check_errors(this->m_spi->shift_out(
            PropWare::MCP3000::OPTN_WIDTH, options));
    check_errors(this->m_spi->shift_in(this->m_dataWidth, dat, sizeof(*dat)));
    this->m_cs.set();

    return 0;
}

PropWare::ErrorCode PropWare::MCP3000::read_diff (
        const PropWare::MCP3000::ChannelDiff channels, uint16_t *dat) {
    int8_t err, options;

    options = PropWare::MCP3000::START | PropWare::MCP3000::DIFFERENTIAL
            | channels;
    options <<= 2; // One dead bit between output and input - see page 19 of datasheet

    if (this->m_alwaysSetMode) {
        check_errors(this->m_spi->set_mode(PropWare::MCP3000::SPI_MODE));
        check_errors(this->m_spi->set_bit_mode(PropWare::MCP3000::SPI_BITMODE));
    }

    this->m_cs.clear();
    check_errors(this->m_spi->shift_out(
            PropWare::MCP3000::OPTN_WIDTH, options));
    check_errors(this->m_spi->shift_in(this->m_dataWidth, dat, sizeof(*dat)));
    this->m_cs.set();

    return 0;
}
