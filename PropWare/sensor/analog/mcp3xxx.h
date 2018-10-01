/**
 * @file    PropWare/sensor/analog/mcp3xxx.h
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

#pragma once

#include <PropWare/PropWare.h>
#include <PropWare/serial/spi/spi.h>

namespace PropWare {

/**
 * @brief   MCP3xxx-series ADC driver using SPI communication for the Parallax Propeller. Compatible with the following
 *          devices:
 *              - MCP3002
 *              - MCP3004
 *              - MCP3008
 *              - MCP3202
 *              - MCP3204
 *              - MCP3208
 *              - MCP3302
 *              - MCP3304
 *              - MCP3308
 *
 * @note    MCP3xxx chips uses SPI mode 2 and shifts data MSB first
 */
class MCP3xxx {
    public:
        /** Single-ended channels */
        enum class Channel {
                /** Channel 0 */               CHANNEL_0,
                /** Channel 1 */               CHANNEL_1,
                /** Channel 2 */               CHANNEL_2,
                /** Channel 3 */               CHANNEL_3,
                /** Channel 4 (MCP3008 only) */CHANNEL_4,
                /** Channel 5 (MCP3008 only) */CHANNEL_5,
                /** Channel 6 (MCP3008 only) */CHANNEL_6,
                /** Channel 7 (MCP3008 only) */CHANNEL_7,
        };

        /** Pseudo-differential pair channels */
        enum class ChannelDiff {
                /** CH0+, CH1- */               DIFF_0_1,
                /** CH1+, CH0- */               DIFF_1_0,
                /** CH2+, CH3- */               DIFF_2_3,
                /** CH3+, CH2- */               DIFF_3_2,
                /** CH4+, CH5- (MCP3008 only) */DIFF_4_5,
                /** CH5+, CH4- (MCP3008 only) */DIFF_5_4,
                /** CH6+, CH7- (MCP3008 only) */DIFF_6_7,
                /** CH7+, CH6- (MCP3008 only) */DIFF_7_6
        };

        /** The part number determines the width of the data transfer */
        enum class PartNumber {
                /** 10-bit ADC, includes MCP3002, MCP3004 and MCP3008 */MCP300x = 11,
                /** 12-bit ADC, includes MCP3202, MCP3204 and MCP3208 */MCP320x = 13,
                /** 13-bit ADC, includes MCP3302, MCP3304 and MCP3308 */MCP330x = 14
        };

    public:
        /**
         * @brief       Construction requires an instance of the SPI module; the SPI module does not need to be started
         *
         * @param[in]   spi                 Constructed SPI module
         * @param[in]   cs                  Pin mask used for chip select
         * @param[in]   partNumber          Determine bit-width of the ADC channels
         * @param[in]   alwaysSetSPIMode    Should every invocation of `read` or `read_diff` set the SPI mode. Setting
         *                                  to true is only necessary when multiple devices are connected to the same
         *                                  SPI bus and use different SPI modes
         */
        MCP3xxx (SPI &spi, const PropWare::Pin::Mask cs, MCP3xxx::PartNumber partNumber,
                 const bool alwaysSetSPIMode = false)
                : m_spi(&spi),
                  m_alwaysSetMode(alwaysSetSPIMode),
                  m_dataWidth(static_cast<uint8_t>(partNumber)) {
            this->m_spi->set_mode(SPI_MODE);
            this->m_spi->set_bit_mode(SPI_BITMODE);

            this->m_cs.set_mask(cs);
            this->m_cs.set();
            this->m_cs.set_dir_out();
        }

        /**
         * @brief       Choose whether to always set the SPI mode and bitmode before reading or writing to the ADC;
         *              Useful when multiple devices are connected to the SPI bus
         *
         * @param[in]   alwaysSetMode   For any non-zero value, the SPI modes will always be set before a read or write
         *                              routine
         */
        void always_set_spi_mode (const bool alwaysSetMode) {
            this->m_alwaysSetMode = alwaysSetMode;
        }

        /**
         * @brief       Read a specific channel's data in single-ended mode
         *
         * @param[in]   channel     One of MCP_CHANNEL_`x`, where `x` is a number 0 through 3 (or 0 through 7 for the
         *                          MCP3008); Selects the channel to be read
         * @return      Returns 0 upon success, error code otherwise
         */
        uint16_t read (const MCP3xxx::Channel channel) {
            int8_t   options;
            uint16_t dat;

            options = START | static_cast<int8_t>(SINGLE_ENDED) | static_cast<int8_t>(channel);

            // Two dead bits between output and input - see page 19 of datasheet
            options <<= 2;

            if (this->m_alwaysSetMode) {
                this->m_spi->set_mode(MCP3xxx::SPI_MODE);
                this->m_spi->set_bit_mode(MCP3xxx::SPI_BITMODE);
            }

            this->m_cs.clear();
            this->m_spi->shift_out(OPTION_WIDTH, (uint32_t) options);
            dat = (uint16_t) this->m_spi->shift_in(this->m_dataWidth);
            this->m_cs.set();

            return dat;
        }

        /**
         * @brief       Read a specific axis's data in differential mode
         *
         * @param[in]   channels    One of DIFF_`x`_`y`, where `x` is a number 0 through 3 (or 0 through 7 for the
         *                          MCP3008) and `y` is `x` + (`x` + 1)%2 (See above defined enum or datasheet for
         *                          details)
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        uint16_t read_diff (const MCP3xxx::ChannelDiff channels) {
            int8_t   options;
            uint16_t dat;

            options = START | static_cast<int8_t>(DIFFERENTIAL) | static_cast<int8_t>(channels);

            // Two dead bits between output and input - see page 19 of datasheet
            options <<= 2;

            if (this->m_alwaysSetMode) {
                this->m_spi->set_mode(SPI_MODE);
                this->m_spi->set_bit_mode(SPI_BITMODE);
            }

            this->m_cs.clear();
            this->m_spi->shift_out(OPTION_WIDTH, (uint32_t) options);
            dat = (uint16_t) this->m_spi->shift_in(this->m_dataWidth);
            this->m_cs.set();

            return dat;
        }

    protected:
        static const SPI::Mode    SPI_MODE    = SPI::Mode::MODE_2;
        static const SPI::BitMode SPI_BITMODE = SPI::BitMode::MSB_FIRST;

        static const uint8_t START        = BIT_4;
        static const uint8_t SINGLE_ENDED = BIT_3;
        static const uint8_t DIFFERENTIAL = 0;
        static const uint8_t OPTION_WIDTH = 7;

    protected:
        SPI           *m_spi;
        PropWare::Pin m_cs;
        bool          m_alwaysSetMode;
        uint8_t       m_dataWidth;
};

}
