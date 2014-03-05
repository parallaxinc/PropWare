/**
 * @file    max6675.h
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

#ifndef MAX6675_H_
#define MAX6675_H_

#include <PropWare.h>
#include <spi.h>

namespace PropWare {

/**
 * @brief   K-type thermocouple amplifier driver using SPI communication for the
 *          Parallax Propeller
 */
class MAX6675 {
    public:
        MAX6675 ();

        /**
         * @brief       Initialize communication with an MAX6675 device
         *
         * @param[in]   mosi        Pin mask for MOSI
         * @param[in]   miso        Pin mask for MISO
         * @param[in]   sclk        Pin mask for SCLK
         * @param[in]   cs          Pin mask for CS
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode start (const PropWare::GPIO::Pin mosi,
                const PropWare::GPIO::Pin miso, const PropWare::GPIO::Pin clk,
                const PropWare::GPIO::Pin cs);

        /**
         * @brief       Choose whether to always set the SPI mode and bitmode before
         *              reading or writing to the chip; Useful when multiple devices are
         *              connected to the SPI bus
         *
         * @param[in]   alwaysSetMode   For any non-zero value, the SPI modes will
         *                              always be set before a read or write routine
         */
        void always_set_spi_mode (const bool alwaysSetMode);

        /**
         * @brief       Read data in fixed-point form
         *
         * @detailed    12-bit data is stored where lower 2 bits are fractional and
         *              upper 10 bits are the whole number. Value presented in degrees
         *              Celsius
         *
         * @param[out]  *dat    Address where data should be stored
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode read (uint16_t *dat);

        /**
         * @brief       Read data and return integer value
         *
         * @param[out]  *dat    Address where data should be stored
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode read_whole (uint16_t *dat);

        /**
         * @brief       Read data in floating point form
         *
         * @param[out]  *dat    Address where data should be stored
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode read_float (float *dat);

    private:
        static const uint32_t SPI_DEFAULT_FREQ = 1000000;
        static const SPI::Mode SPI_MODE = SPI::MODE_1;
        static const SPI::BitMode SPI_BITMODE = SPI::MSB_FIRST;
        static const uint8_t BIT_WIDTH = 12;

    private:
        SPI *m_spi;
        PropWare::GPIO::Pin m_cs;
        bool m_alwaysSetMode;
};

}

#endif /* MAX6675_H_ */
