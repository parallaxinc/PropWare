/**
 * @file    mcp300x.h
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

#ifndef MCP300X_H_
#define MCP300X_H_

#include <propeller.h>
#include <PropWare.h>
#include <spi.h>

namespace PropWare {

/**
 * @brief   MCP3004/MCP3008 ADC driver using SPI communication for the Parallax
 *          Propeller
 *
 * @note    MCP300x chips uses SPI mode 2 and shifts data MSB first
 */
class MCP300x {
    public:
        /** Single-ended channels */
        typedef enum {
            /** Channel 0 */CHANNEL_0,
            /** Channel 1 */CHANNEL_1,
            /** Channel 2 */CHANNEL_2,
            /** Channel 3 */CHANNEL_3,
            /** Channel 4 (MCP3008 only) */CHANNEL_4,
            /** Channel 5 (MCP3008 only) */CHANNEL_5,
            /** Channel 6 (MCP3008 only) */CHANNEL_6,
            /** Channel 7 (MCP3008 only) */CHANNEL_7,
        } Channel;

        /** Pseudo-differential pair channels */
        typedef enum {
            /** CH0+, CH1- */DIFF_0_1,
            /** CH1+, CH0- */DIFF_1_0,
            /** CH2+, CH3- */DIFF_2_3,
            /** CH3+, CH2- */DIFF_3_2,
            /** CH4+, CH5- (MCP3008 only) */DIFF_4_5,
            /** CH5+, CH4- (MCP3008 only) */DIFF_5_4,
            /** CH6+, CH7- (MCP3008 only) */DIFF_6_7,
            /** CH7+, CH6- (MCP3008 only) */DIFF_7_6
        } ChannelDiff;

    public:
        MCP300x ();

        /**
         * @brief       Initialize communication with an MCP300x device
         *
         * @param[in]   mosi        Pin mask for MOSI
         * @param[in]   miso        Pin mask for MISO
         * @param[in]   sclk        Pin mask for SCLK
         * @param[in]   cs          Pin mask for CS
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        int8_t start (const uint32_t mosi, const uint32_t miso,
                const uint32_t sclk, const uint32_t cs);

        /**
         * @brief       Choose whether to always set the SPI mode and bitmode before
         *              reading or writing to the ADC; Useful when multiple devices are
         *              connected to the SPI bus
         *
         * @param[in]   alwaysSetMode   For any non-zero value, the SPI modes will
         *                              always be set before a read or write routine
         */
        void always_set_spi_mode (const uint8_t alwaysSetMode);

        /**
         * @brief       Read a specific channel's data in single-ended mode
         *
         * @param[in]   axis    One of MCP_CHANNEL_<x>, where <x> is a number 0 through
         *                      3 (or 0 through 7 for the MCP3008); Selects the channel
         *                      to be read
         * @param[out]  *val    Address that data should be placed into
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        int8_t read (const MCP300x::Channel channel, uint16_t *dat);

        /**
         * @brief       Read a specific axis's data in differential mode
         *
         * @param[in]   axis    One of DIFF_<x>_<y>, where <x> is a number 0 through 3
         *                      (or 0 through 7 for the MCP3008) and <y> is
         *                      <x> + (<x> + 1)%2 (See above defined enum or datasheet
         *                      for details)
         * @param[out]  *val    Address that data should be placed into
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        int8_t read_diff (const MCP300x::ChannelDiff channels, uint16_t *dat);

    private:
        static const uint32_t SPI_DEFAULT_FREQ = 100000;
        static const SPI::Mode SPI_MODE = SPI::MODE_2;
        static const SPI::BitMode SPI_BITMODE = SPI::MSB_FIRST;

        static const uint8_t START = BIT_4;
        static const uint8_t SINGLE_ENDED = BIT_3;
        static const uint8_t DIFFERENTIAL = 0;
        static const uint8_t OPTN_WIDTH = 7;
        static const uint8_t DATA_WIDTH = 11;

    private:
        SPI *m_spi;
        uint8_t m_cs;
        uint8_t m_alwaysSetMode;
};

}

#endif /* MCP300X_H_ */
