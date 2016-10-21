/**
 * @file    PropWare/sensor/gyroscope/l3g.h
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
         * Sensitivity measured in degrees per second
         */
        typedef enum {
            /** 250 degrees per second */ DPS_250  = 0x00,
            /** 500 degrees per second */ DPS_500  = 0x10,
            /** 2000 degrees per second */DPS_2000 = 0x20
        } DPSMode;

    public:
        static const DPSMode DEFAULT_DPS = DPS_250;

        static const uint8_t WHO_AM_I = 0x0F;

        static const uint8_t CTRL_REG1  = 0x20;
        static const uint8_t CTRL_REG2  = 0x21;
        static const uint8_t CTRL_REG3  = 0x22;
        static const uint8_t CTRL_REG4  = 0x23;
        static const uint8_t CTRL_REG5  = 0x24;
        static const uint8_t REFERENCE  = 0x25;
        static const uint8_t OUT_TEMP   = 0x26;
        static const uint8_t STATUS_REG = 0x27;
        static const uint8_t OUT_X_L    = 0x28;
        static const uint8_t OUT_X_H    = 0x29;
        static const uint8_t OUT_Y_L    = 0x2A;
        static const uint8_t OUT_Y_H    = 0x2B;
        static const uint8_t OUT_Z_L    = 0x2C;
        static const uint8_t OUT_Z_H    = 0x2D;

        static const uint8_t FIFO_CTRL_REG = 0x2E;
        static const uint8_t FIFO_SRC_REG  = 0x2F;

        static const uint8_t INT1_CFG      = 0x30;
        static const uint8_t INT1_SRC      = 0x31;
        static const uint8_t INT1_THS_XH   = 0x32;
        static const uint8_t INT1_THS_XL   = 0x33;
        static const uint8_t INT1_THS_YH   = 0x34;
        static const uint8_t INT1_THS_YL   = 0x35;
        static const uint8_t INT1_THS_ZH   = 0x36;
        static const uint8_t INT1_THS_ZL   = 0x37;
        static const uint8_t INT1_DURATION = 0x38;

    public:
        /**
         * @param[in]   spi             SPI bus used for communication with the L3G device
         * @param[in]   cs              Chip select pin mask
         * @param[in]   dpsMode         Precision to be used, measured in degrees per second
         * @param[in]   alwaysSetMode   When set, the SPI object will always have its mode reset, before every read
         *                              or write operation
         */
        L3G(SPI &spi, const PropWare::Port::Mask cs, const PropWare::L3G::DPSMode dpsMode = DEFAULT_DPS,
            const bool alwaysSetMode = false)
            : m_spi(&spi),
              m_cs(cs, PropWare::Pin::Dir::OUT),
              m_dpsMode(dpsMode),
              m_alwaysSetMode(alwaysSetMode) {
            this->m_cs.set();
        }

        /**
         * @brief       Initialize an L3G module
         */
        void start() const {
            this->m_spi->set_mode(PropWare::L3G::SPI_MODE);
            this->m_spi->set_bit_mode(PropWare::L3G::SPI_BITMODE);

            // NOTE L3G has high- and low-pass filters. Should they be enabled?
            // (Page 31)
            this->write8(PropWare::L3G::CTRL_REG1, NIBBLE_0);
            this->write8(PropWare::L3G::CTRL_REG4, BIT_7 | this->m_dpsMode);
        }

        /**
         * @brief       Choose whether to always set the SPI mode and bitmode before reading or writing to the L3G
         *              module; Useful when multiple devices are connected to the SPI bus
         *
         * @param[in]   alwaysSetMode   For any non-zero value, the SPI modes will always be set before a read or write
         *                              routine
         */
        void always_set_spi_mode(const bool alwaysSetMode) {
            this->m_alwaysSetMode = alwaysSetMode;
        }

        /**
         * @brief       Read a specific axis's data
         *
         * @param[in]   axis    Selects the axis to be read
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        int16_t read(const PropWare::L3G::Axis axis) const {
            return this->read16(PropWare::L3G::OUT_X_L + (static_cast<uint8_t>(axis) << 1));
        }

        /**
         * @brief       Read data from the X axis
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        int16_t read_x() const {
            return this->read16(PropWare::L3G::OUT_X_L);
        }

        /**
         * @brief       Read data from the Y axis
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        int16_t read_y() const {
            return this->read16(PropWare::L3G::OUT_Y_L);
        }

        /**
         * @brief       Read data from the Z axis
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        int16_t read_z() const {
            return this->read16(PropWare::L3G::OUT_Z_L);
        }

        /**
         * @brief       Read data from all three axes
         *
         * @param[out]  val     Starting address for data to be placed; 6 contiguous bytes of space are required for the
         *                      read routine
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        void read_all(int16_t *val) const {
            uint8_t i;

            uint8_t addr = PropWare::L3G::OUT_X_L;
            addr |= BIT_7;  // Set RW bit (
            addr |= BIT_6;  // Enable address auto-increment

            this->maybe_set_spi_mode();

            this->m_cs.clear();
            this->m_spi->shift_out(8, addr);
            val[Axis::X] = (int16_t) this->m_spi->shift_in(16);
            val[Axis::Y] = (int16_t) this->m_spi->shift_in(16);
            val[Axis::Z] = (int16_t) this->m_spi->shift_in(16);
            this->m_cs.set();

            // err is useless at this point and will be used as a temporary
            // 8-bit variable
            uint32_t temp;
            for (i = 0; i < 3; ++i) {
                temp = (uint32_t) (val[i] >> 8);
                val[i] <<= 8;
                val[i] |= temp;
            }
        }

        /**
         * @brief       Modify the scale of L3G in units of degrees per second
         *
         * @param[in]   dpsMode     Desired full-scale mode
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        void set_dps(const PropWare::L3G::DPSMode dpsMode) {
            uint8_t oldValue;

            this->m_dpsMode = dpsMode;
            this->maybe_set_spi_mode();

            oldValue = this->read8(PropWare::L3G::CTRL_REG4);
            oldValue &= ~(BIT_5 | BIT_4);
            oldValue |= static_cast<uint8_t>(dpsMode);
            this->write8(PropWare::L3G::CTRL_REG4, oldValue);
        }

        /**
         * @brief   Retrieve the current DPS setting
         *
         * @return  Returns what the L3G module is using for DPS mode
         */
        PropWare::L3G::DPSMode get_dps() const {
            return this->m_dpsMode;
        }

        /**
         * @brief       Convert the raw, integer value from the gyro into units of degrees-per-second
         *
         *
         * @pre         Input value must have been read in when the DPS setting was set to the same value as it is
         *              during this function execution. If the input value was read with a different DPS setting, use
         *              `PropWare::L3G::convert_to_dps(const int16_t rawValue, const PropWare::L3G::DPSMode dpsMode)`
         *              to specify the correct DPS setting
         *
         * @param[in]   rawValue    Value from the gyroscope
         *
         * @return      Returns the rotational value in degrees-per-second
         */
        float convert_to_dps(const int16_t rawValue) const {
            return PropWare::L3G::convert_to_dps(rawValue, this->m_dpsMode);
        }

        /**
         * @brief       Convert the raw, integer value from the gyro into units of degrees-per-second
         *
         * @param[in]   rawValue    Value from the gyroscope
         * @param[in]   dpsMode     The DPS setting used at the time of reading rawValue
         *
         * @return      Returns the rotational value in degrees-per-second
         */
        static float convert_to_dps(const int16_t rawValue, const PropWare::L3G::DPSMode dpsMode) {
            switch (dpsMode) {
                case DPSMode::DPS_250:
                    return (float) (rawValue * 0.00875);
                case DPSMode::DPS_500:
                    return (float) (rawValue * 0.01750);
                case DPSMode::DPS_2000:
                    return (float) (rawValue * 0.07000);
            }
            return 0;
        }

    private:
        static const SPI::Mode    SPI_MODE    = SPI::Mode::MODE_3;
        static const SPI::BitMode SPI_BITMODE = SPI::BitMode::MSB_FIRST;

    protected:
        /***********************************
         *** Private Method Declarations ***
         ***********************************/
        /**
         * @brief       Write one byte to the L3G module
         *
         * @param[in]   registerAddress    Destination register address
         * @param[in]   registerValue     Data to be written to the destination register
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        void write8(uint8_t registerAddress, const uint8_t registerValue) const {
            uint16_t combinedWord;

            registerAddress &= ~BIT_7;  // Clear the RW bit (write mode)

            combinedWord = ((uint16_t) registerAddress) << 8;
            combinedWord |= registerValue;

            this->maybe_set_spi_mode();

            this->m_cs.clear();
            this->m_spi->shift_out(16, combinedWord);
            this->m_cs.set();
        }

        /**
         * @brief       Write one byte to the L3G module
         *
         * @param[in]   registerAddress    Destination register address
         * @param[in]   registerValue     Data to be written to the destination register
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        void write16(uint8_t registerAddress, const uint16_t registerValue) const {
            uint16_t outputValue;

            registerAddress &= ~BIT_7;  // Clear the RW bit (write mode)
            registerAddress |= BIT_6;  // Enable address auto-increment

            outputValue = ((uint16_t) registerAddress) << 16;
            outputValue |= ((uint16_t) ((uint8_t) registerValue)) << 8;
            outputValue |= (uint8_t) (registerValue >> 8);

            this->maybe_set_spi_mode();

            this->m_cs.clear();
            this->m_spi->shift_out(24, outputValue);
            this->m_cs.set();
        }

        /**
         * @brief       Read one byte from the L3G module
         *
         * @param[in]   registerAddress    Origin register address
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        uint8_t read8(uint8_t registerAddress) const {
            uint8_t registerValue;

            registerAddress |= BIT_7;  // Set RW bit (
            registerAddress |= BIT_6;  // Enable address auto-increment

            this->maybe_set_spi_mode();

            this->m_cs.clear();
            this->m_spi->shift_out(8, registerAddress);
            registerValue = (int8_t) this->m_spi->shift_in(8);
            this->m_cs.set();

            return registerValue;
        }

        /**
         * @brief       Read two bytes from the L3G module
         *
         * @param[in]   registerAddress    Origin register address
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        int16_t read16(uint8_t registerAddress) const {
            uint32_t registerValue;


            registerAddress |= BIT_7;  // Set RW bit (
            registerAddress |= BIT_6;  // Enable address auto-increment

            this->maybe_set_spi_mode();

            this->m_cs.clear();
            this->m_spi->shift_out(8, registerAddress);
            registerValue = (int16_t) this->m_spi->shift_in(16);
            this->m_cs.set();

            // err is useless at this point and will be used as a temporary
            // 8-bit variable
            int temp;
            temp = registerValue >> 8;
            registerValue <<= 8;
            return (int16_t) (registerValue | temp);
        }

        /**
         * @brief   Set the SPI mode iff PropWare::L3G::m_alwaysSetMode is true
         *
         * @return  Returns 0 upon success, error code otherwise
         */
        void maybe_set_spi_mode() const {
            if (this->m_alwaysSetMode) {
                this->m_spi->set_mode(L3G::SPI_MODE);
                this->m_spi->set_bit_mode(L3G::SPI_BITMODE);
            }
        }

    private:
        SPI                    *m_spi;
        const PropWare::Pin    m_cs;
        PropWare::L3G::DPSMode m_dpsMode;
        bool                   m_alwaysSetMode;
};

}
