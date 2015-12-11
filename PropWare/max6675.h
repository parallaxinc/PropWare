/**
 * @file    PropWare/max6675.h
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

#pragma once

#include <PropWare/PropWare.h>
#include <PropWare/spi.h>

namespace PropWare {

/**
 * @brief   K-type thermocouple amplifier driver using SPI communication for the Parallax Propeller
 */
class MAX6675 {
    public:
        /**
         * @brief       Construction requires an instance of the SPI module; the SPI module does not need to be started
         *
         * @param[in]   *spi    Constructed SPI module
         */
        MAX6675 (SPI *spi) {
            this->m_spi           = spi;
            this->m_alwaysSetMode = 0;
        }

        /**
         * @brief       Initialize communication with an MAX6675 device
         *
         * @param[in]   mosi    PinNum mask for MOSI
         * @param[in]   miso    PinNum mask for MISO
         * @param[in]   sclk    PinNum mask for SCLK
         * @param[in]   cs      PinNum mask for CS
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        void start (const Port::Mask mosi, const Port::Mask miso, const Port::Mask sclk, const Port::Mask cs) {
            this->m_spi->set_mosi(mosi);
            this->m_spi->set_miso(miso);
            this->m_spi->set_sclk(sclk);
            this->m_spi->set_clock(SPI_DEFAULT_FREQ);
            this->m_spi->set_mode(SPI_MODE);
            this->m_spi->set_bit_mode(SPI_BITMODE);

            this->m_cs.set_mask(cs);
            this->m_cs.set_dir(PropWare::Pin::OUT);
            this->m_cs.set();
        }

        /**
         * @brief       Choose whether to always set the SPI mode and bitmode before reading or writing to the chip;
         *              Useful when multiple devices are connected to the SPI bus
         *
         * @param[in]   alwaysSetMode   For any non-zero value, the SPI modes will always be set before a read or write
         *                              routine
         */
        void always_set_spi_mode (const bool alwaysSetMode) {
            this->m_alwaysSetMode = alwaysSetMode;
        }

        /**
         * @brief       Read data in fixed-point form
         *
         * 12-bit data is stored where lower 2 bits are fractional and upper10 bits are the whole number. Value
         * presented in degrees Celsius
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        uint16_t read () {
            uint16_t dat;

            if (this->m_alwaysSetMode) {
                this->m_spi->set_mode(SPI_MODE);
                this->m_spi->set_bit_mode(SPI_BITMODE);
            }

            this->m_cs.clear();
            dat = (uint16_t) this->m_spi->shift_in(MAX6675::BIT_WIDTH);
            this->m_cs.set();

            return dat;
        }

        /**
         * @brief       Read data and return integer value
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        uint16_t read_whole () {
            uint16_t dat;

            dat = this->read();
            return dat >> 2;
        }

        /**
         * @brief       Read data in floating point form
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        float read_float () {
            uint16_t            temp;

            temp = this->read();

            float dat;
            dat = temp >> 2;
            return dat + ((float) (temp & (BIT_1 | BIT_0))) / 4;
        }

    private:
        static const uint32_t     SPI_DEFAULT_FREQ = 1000000;
        static const SPI::Mode    SPI_MODE         = SPI::MODE_1;
        static const SPI::BitMode SPI_BITMODE      = SPI::MSB_FIRST;
        static const uint8_t      BIT_WIDTH        = 12;

    private:
        SPI  *m_spi;
        Pin  m_cs;
        bool m_alwaysSetMode;
};

}
