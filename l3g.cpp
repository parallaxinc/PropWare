/**
 * @file    l3g.c
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

#include <l3g.h>

namespace PropWare {

L3G::L3G (SPI *spi) {
    this->m_spi = spi;
    this->m_alwaysSetMode = false;
}

PropWare::ErrorCode L3G::start (const PropWare::GPIO::Pin mosi,
        const PropWare::GPIO::Pin miso, const PropWare::GPIO::Pin sclk,
        const PropWare::GPIO::Pin cs, const L3G::DPSMode dpsMode) {
    PropWare::ErrorCode err;
    // Ensure SPI module started
    if (!this->m_spi->is_running()) {
        check_errors(
                this->m_spi->start(mosi, miso, sclk, L3G::SPI_DEFAULT_FREQ,
                        L3G::SPI_MODE, L3G::SPI_BITMODE));

    } else {
        check_errors(this->m_spi->set_mode(L3G::SPI_MODE));
        check_errors(this->m_spi->set_bit_mode(L3G::SPI_BITMODE));
    }

    this->m_cs = cs;
    GPIO::set_dir(cs, GPIO::OUT);
    GPIO::pin_set(cs);

    // NOTE L3G has high- and low-pass filters. Should they be enabled? (Page
    // 31)
    check_errors(this->write8(L3G::CTRL_REG1, NIBBLE_0));
    check_errors(this->write8(L3G::CTRL_REG4, dpsMode | BIT_7));

    return 0;
}

void L3G::always_set_spi_mode (const bool alwaysSetMode) {
    this->m_alwaysSetMode = alwaysSetMode;
}

PropWare::ErrorCode L3G::read_x (int16_t *val) {
    return this->read16(L3G::OUT_X_L, val);
}

PropWare::ErrorCode L3G::read_y (int16_t *val) {
    return this->read16(L3G::OUT_Y_L, val);
}

PropWare::ErrorCode L3G::read_z (int16_t *val) {
    return this->read16(L3G::OUT_Z_L, val);
}

PropWare::ErrorCode L3G::read (const L3G::Axis axis, int16_t *val) {
    return this->read16(L3G::OUT_X_L + (axis << 1), val);
}

PropWare::ErrorCode L3G::read_all (int16_t *val) {
    uint8_t err, i;

    uint8_t addr = L3G::OUT_X_L;
    addr |= BIT_7;  // Set RW bit (
    addr |= BIT_6;  // Enable address auto-increment

    if (this->m_alwaysSetMode) {
        check_errors(this->m_spi->set_mode(L3G::SPI_MODE));
        check_errors(this->m_spi->set_bit_mode(L3G::SPI_BITMODE));
    }

    GPIO::pin_clear(this->m_cs);
    check_errors(this->m_spi->shift_out(8, addr));
    check_errors(
            this->m_spi->shift_in(16, &(val[L3G::X]), sizeof(val[L3G::X])));
    check_errors(
            this->m_spi->shift_in(16, &(val[L3G::Y]), sizeof(val[L3G::Y])));
    check_errors(
            this->m_spi->shift_in(16, &(val[L3G::Z]), sizeof(val[L3G::Z])));
    GPIO::pin_set(this->m_cs);

    // err is useless at this point and will be used as a temporary 8-bit
    // variable
    for (i = 0; i < 3; ++i) {
        err = val[i] >> 8;
        val[i] <<= 8;
        val[i] |= err;
    }

    return 0;
}

PropWare::ErrorCode L3G::ioctl (const L3G::IoctlFunction func, const uint8_t wrVal,
        uint8_t *rdVal) {
    uint8_t err, oldValue;

    if (this->m_alwaysSetMode) {
        check_errors(this->m_spi->set_mode(L3G::SPI_MODE));
        check_errors(this->m_spi->set_bit_mode(L3G::SPI_BITMODE));
    }

    switch (func) {
        // All functions follow the read-modify-write routine
        case L3G::FUNC_MOD_DPS:
            check_errors(this->read8(L3G::CTRL_REG4, (int8_t * ) &oldValue));
            oldValue &= ~(BIT_6 | BIT_5);
            oldValue |= wrVal << 5;
            check_errors(this->write8(L3G::CTRL_REG4, oldValue));
            break;
        case L3G::FUNC_RD_REG:
            check_errors(this->read8(wrVal, (int8_t * ) rdVal));
            break;
        default:
            return -1;  // TODO: Create a real error code
    }

    return 0;
}

/*************************************
 *** Private Function Declarations ***
 *************************************/
PropWare::ErrorCode L3G::write8 (uint8_t addr, const uint8_t dat) {
    PropWare::ErrorCode err;
    uint16_t outputValue;

    addr &= ~BIT_7;  // Clear the RW bit (write mode)

    outputValue = ((uint16_t) addr) << 8;
    outputValue |= dat;

    if (this->m_alwaysSetMode) {
        check_errors(this->m_spi->set_mode(L3G::SPI_MODE));
        check_errors(this->m_spi->set_bit_mode(L3G::SPI_BITMODE));
    }

    GPIO::pin_clear(this->m_cs);
    err = this->m_spi->shift_out(16, outputValue);
    check_errors(this->m_spi->wait());
    GPIO::pin_set(this->m_cs);

    return err;
}

PropWare::ErrorCode L3G::write16 (uint8_t addr, const uint16_t dat) {
    PropWare::ErrorCode err;
    uint16_t outputValue;

    addr &= ~BIT_7;  // Clear the RW bit (write mode)
    addr |= BIT_6;  // Enable address auto-increment

    outputValue = ((uint16_t) addr) << 16;
    outputValue |= ((uint16_t) ((uint8_t) dat)) << 8;
    outputValue |= (uint8_t) (dat >> 8);

    if (this->m_alwaysSetMode) {
        check_errors(this->m_spi->set_mode(L3G::SPI_MODE));
        check_errors(this->m_spi->set_bit_mode(L3G::SPI_BITMODE));
    }

    GPIO::pin_clear(this->m_cs);
    check_errors(this->m_spi->shift_out(24, outputValue));
    check_errors(this->m_spi->wait());
    GPIO::pin_set(this->m_cs);

    return 0;
}

PropWare::ErrorCode L3G::read8 (uint8_t addr, int8_t *dat) {
    PropWare::ErrorCode err;

    addr |= BIT_7;  // Set RW bit (
    addr |= BIT_6;  // Enable address auto-increment

    if (this->m_alwaysSetMode) {
        check_errors(this->m_spi->set_mode(L3G::SPI_MODE));
        check_errors(this->m_spi->set_bit_mode(L3G::SPI_BITMODE));
    }

    GPIO::pin_clear(this->m_cs);
    check_errors(this->m_spi->shift_out(8, addr));
    check_errors(this->m_spi->shift_in(8, dat, sizeof(*dat)));
    GPIO::pin_set(this->m_cs);

    return 0;
}

PropWare::ErrorCode L3G::read16 (uint8_t addr, int16_t *dat) {
    PropWare::ErrorCode err;

    addr |= BIT_7;  // Set RW bit (
    addr |= BIT_6;  // Enable address auto-increment

    if (this->m_alwaysSetMode) {
        check_errors(this->m_spi->set_mode(L3G::SPI_MODE));
        check_errors(this->m_spi->set_bit_mode(L3G::SPI_BITMODE));
    }

    GPIO::pin_clear(this->m_cs);
    check_errors(this->m_spi->shift_out(8, addr));
    check_errors(this->m_spi->shift_in(16, dat, sizeof(*dat)));
    GPIO::pin_set(this->m_cs);

    // err is useless at this point and will be used as a temporary 8-bit
    // variable
    err = *dat >> 8;
    *dat <<= 8;
    *dat |= err;

    return 0;
}

}
