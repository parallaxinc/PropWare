/**
 * @file    l3g.h
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

#ifndef L3G_H_
#define L3G_H_

#include <propeller.h>
#include <PropWare/PropWare.h>
#include <PropWare/spi.h>

namespace PropWare {

/**
 * @brief   L3G gyroscope driver using SPI communication for the Parallax
 *          Propeller
 */
class L3G {
    public:
        /**
         * Axes of the L3G device
         */
        typedef enum {
            /** X axis */X,
            /** Y axis */Y,
            /** Z axis */Z
        } Axis;

        /**
         * Extra functions available on the L3G device; Callable by passing one as the
         * first parameter to @ref L3G::ioctl
         */
        typedef enum {
            /** Set the sensitivity of input values; must be one of L3G_DPSMode */
            FUNC_MOD_DPS,
            /** Read the value on any internal register */
            FUNC_RD_REG,
            /** Total number of advanced functions */
            FUNCS
        } IoctlFunction;

        /**
         * Sensitivity measured in degrees per second
         */
        typedef enum {
            /** 250 degrees per second */
            DPS_250 = 0x00,
            /** 500 degrees per second */
            DPS_500 = 0x10,
            /** 2000 degrees per second */
            DPS_2000 = 0x20
        } DPSMode;

    public:
        static const uint8_t WHO_AM_I = 0x0F;

        static const uint8_t CTRL_REG1 = 0x20;
        static const uint8_t CTRL_REG2 = 0x21;
        static const uint8_t CTRL_REG3 = 0x22;
        static const uint8_t CTRL_REG4 = 0x23;
        static const uint8_t CTRL_REG5 = 0x24;
        static const uint8_t REFERENCE = 0x25;
        static const uint8_t OUT_TEMP = 0x26;
        static const uint8_t STATUS_REG = 0x27;
        static const uint8_t OUT_X_L = 0x28;
        static const uint8_t OUT_X_H = 0x29;
        static const uint8_t OUT_Y_L = 0x2A;
        static const uint8_t OUT_Y_H = 0x2B;
        static const uint8_t OUT_Z_L = 0x2C;
        static const uint8_t OUT_Z_H = 0x2D;

        static const uint8_t FIFO_CTRL_REG = 0x2E;
        static const uint8_t FIFO_SRC_REG = 0x2F;

        static const uint8_t INT1_CFG = 0x30;
        static const uint8_t INT1_SRC = 0x31;
        static const uint8_t INT1_THS_XH = 0x32;
        static const uint8_t INT1_THS_XL = 0x33;
        static const uint8_t INT1_THS_YH = 0x34;
        static const uint8_t INT1_THS_YL = 0x35;
        static const uint8_t INT1_THS_ZH = 0x36;
        static const uint8_t INT1_THS_ZL = 0x37;
        static const uint8_t INT1_DURATION = 0x38;

    public:
        L3G (SPI *spi);

        /**
         * @brief       Initialize an L3G module
         *
         * @param[in]   mosi        Pin mask for MOSI
         * @param[in]   miso        Pin mask for MISO
         * @param[in]   sclk        Pin mask for SCLK
         * @param[in]   cs          Pin mask for CS
         * @param[in]   dpsMode     One of L3G_250_DPS, L3G_500_DPS, L3G_2000_DPS;
         *                          Determines the resolution of the L3G device in terms
         *                          of degrees per second
         *
         * @return       Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode start (const PropWare::GPIO::Pin mosi,
                const PropWare::GPIO::Pin miso, const PropWare::GPIO::Pin sclk,
                const PropWare::GPIO::Pin cs, const L3G::DPSMode dpsMode);

        /**
         * @brief       Choose whether to always set the SPI mode and bitmode before
         *              reading or writing to the L3G module; Useful when multiple
         *              devices are connected to the SPI bus
         *
         * @param[in]   alwaysSetMode   For any non-zero value, the SPI modes will
         *                              always be set before a read or write routine
         */
        void always_set_spi_mode (const bool alwaysSetMode);

        /**
         * @brief       Read a specific axis's data
         *
         * @param[in]   axis    One of L3G_X, L3G_Y, L3G_Z; Selects the axis to be read
         * @param[out]  *val    Address that data should be placed into
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode read (const L3G::Axis axis, int16_t *val);

        /**
         * @brief       Read data from the X axis
         *
         * @param[out]  *val    Address that data should be placed into
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode read_x (int16_t *val);

        /**
         * @brief       Read data from the Y axis
         *
         * @param[out]  *val    Address that data should be placed into
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode read_y (int16_t *val);

        /**
         * @brief       Read data from the Z axis
         *
         * @param[out]  *val    Address that data should be placed into
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode read_z (int16_t *val);

        /**
         * @brief       Read data from all three axes
         *
         * @param[out]  *val    Starting address for data to be placed; 6 contiguous
         *                      bytes of space are required for the read routine
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode read_all (int16_t *val);

        /**
         * @brief       Allow numerous advanced functions to be performed on the L3G,
         *              depending on the value of the first parameter
         *
         *
         * @detailed    <strong>L3G_FUNC_MOD_DPS:</strong> Modify the precision of L3G
         *              in terms of degrees per second
         * @param[in]   func    Descriptor for which function should be performed
         * @param[in]   wrVal   One of L3G_250_DPS, L3G_500_DPS, L3G_2000_DPS
         * @param[out]  *rdVal  Unused
         *
         * @detailed    <strong>L3G_FUNC_RD_REG:</strong> Read any register from the L3G
         * @param[in]   func    Descriptor for which function should be performed
         * @param[in]   wrVal   Address of the desired register
         * @param[out]  *rdVal  Resulting value will be stored in rdVal
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode ioctl (const L3G::IoctlFunction func, const uint8_t wrVal,
                uint8_t *rdVal);

    private:
        static const uint32_t SPI_DEFAULT_FREQ = 100000;
        static const SPI::Mode SPI_MODE = SPI::MODE_3;
        static const SPI::BitMode SPI_BITMODE = SPI::MSB_FIRST;

        /***********************************
         *** Private Method Declarations ***
         ***********************************/
    private:
        /**
         * @brief       Write one byte to the L3G module
         *
         * @param[in]   address     Destination register address
         * @param[in]   dat         Data to be written to the destination register
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode write8 (uint8_t addr, const uint8_t dat);

        /**
         * @brief       Write one byte to the L3G module
         *
         * @param[in]   address     Destination register address
         * @param[in]   dat         Data to be written to the destination register
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode write16 (uint8_t addr, const uint16_t dat);

        /**
         * @brief       Read one byte from the L3G module
         *
         * @param[in]   address     Origin register address
         * @param[out]  *dat        Address where incoming data should be stored
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode read8 (uint8_t addr, int8_t *dat);

        /**
         * @brief       Read two bytes from the L3G module
         *
         * @param[in]   address     Origin register address
         * @param[out]  *dat        Address where incoming data should be stored
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        PropWare::ErrorCode read16 (uint8_t addr, int16_t *dat);

    private:
        SPI *m_spi;
        PropWare::GPIO::Pin m_cs;
        bool m_alwaysSetMode;
};

}

#endif /* L3G_H_ */
