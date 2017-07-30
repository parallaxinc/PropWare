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
 * @brief   L3G gyroscope driver using SPI communication for the Parallax Propeller
 */
class L3G {
    public:
        /**
         * Axes of the L3G device
         */
        typedef enum {
            /** X axis */         X,
            /** Y axis */         Y,
            /** Z axis */         Z,
            /** Number of axes */ AXES
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
        static const SPI::Mode    SPI_MODE    = SPI::Mode::MODE_3;
        static const SPI::BitMode SPI_BITMODE = SPI::BitMode::MSB_FIRST;

        enum class Register {
                WHO_AM_I      = 0x0F,
                CTRL_REG1     = 0x20,
                CTRL_REG2     = 0x21,
                CTRL_REG3     = 0x22,
                CTRL_REG4     = 0x23,
                CTRL_REG5     = 0x24,
                REFERENCE     = 0x25,
                OUT_TEMP      = 0x26,
                STATUS_REG    = 0x27,
                OUT_X_L       = 0x28,
                OUT_X_H       = 0x29,
                OUT_Y_L       = 0x2A,
                OUT_Y_H       = 0x2B,
                OUT_Z_L       = 0x2C,
                OUT_Z_H       = 0x2D,
                FIFO_CTRL_REG = 0x2E,
                FIFO_SRC_REG  = 0x2F,
                INT1_CFG      = 0x30,
                INT1_SRC      = 0x31,
                INT1_THS_XH   = 0x32,
                INT1_THS_XL   = 0x33,
                INT1_THS_YH   = 0x34,
                INT1_THS_YL   = 0x35,
                INT1_THS_ZH   = 0x36,
                INT1_THS_ZL   = 0x37,
                INT1_DURATION = 0x38
        };

    public:
        /**
         * @brief       Convert the raw, integer value from the gyro into units of degrees-per-second
         *
         * @param[in]   rawValue    Value from the gyroscope
         * @param[in]   dpsMode     The DPS setting used at the time of reading rawValue
         *
         * @return      Returns the rotational value in degrees-per-second
         */
        static float to_dps (const int rawValue, const DPSMode dpsMode) {
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

    public:
        /**
         * @param[in]   spi             SPI bus used for communication with the L3G device
         * @param[in]   cs              Chip select pin mask
         * @param[in]   dpsMode         Precision to be used, measured in degrees per second
         * @param[in]   alwaysSetMode   When set, the SPI object will always have its mode reset, before every read
         *                              or write operation
         */
        L3G (SPI &spi, const Port::Mask cs, const bool alwaysSetMode = false)
            : m_spi(&spi),
              m_cs(cs, Pin::Dir::OUT),
              m_alwaysSetMode(alwaysSetMode) {
            this->m_cs.set();
        }

        /**
         * @brief       Choose whether to always set the SPI mode and bitmode before reading or writing to the L3G
         *              module; Useful when multiple devices are connected to the SPI bus
         *
         * @param[in]   alwaysSetMode   For any non-zero value, the SPI modes will always be set before a read or write
         *                              routine
         */
        void always_set_spi_mode (const bool alwaysSetMode) {
            this->m_alwaysSetMode = alwaysSetMode;
        }

        /**
         * @brief       Read data from all three axes
         *
         * @param[out]  values  Starting address for data to be placed; 6 contiguous bytes of space are required for the
         *                      read routine
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        void read (int16_t *values) const {
            const auto addr = static_cast<uint8_t>(Register::OUT_X_L) | BIT_7 | BIT_6;

            this->maybe_set_spi_mode();

            this->m_cs.clear();
            this->m_spi->shift_out(8, addr);
            values[Axis::X]        = static_cast<int16_t>(this->m_spi->shift_in(16));
            values[Axis::Y]        = static_cast<int16_t>(this->m_spi->shift_in(16));
            values[Axis::Z]        = static_cast<int16_t>(this->m_spi->shift_in(16));
            this->m_cs.set();

            // err is useless at this point and will be used as a temporary
            // 8-bit variable
            uint32_t          temp;
            for (unsigned int axis = 0; axis < AXES; ++axis) {
                temp = (uint32_t) (values[axis] >> 8);
                values[axis] <<= 8;
                values[axis] |= temp;
            }
        }

        /**
         * @brief       Read a specific axis's data
         *
         * @param[in]   axis    Selects the axis to be read
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        int16_t read (const Axis axis) const {
            return this->read16(static_cast<Register>(static_cast<unsigned int>(Register::OUT_X_L) + 2 * axis));
        }

        /**
         * @brief       Read data from the X axis
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        int16_t read_x () const {
            return this->read16(Register::OUT_X_L);
        }

        /**
         * @brief       Read data from the Y axis
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        int16_t read_y () const {
            return this->read16(Register::OUT_Y_L);
        }

        /**
         * @brief       Read data from the Z axis
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        int16_t read_z () const {
            return this->read16(Register::OUT_Z_L);
        }

        /**
         * @brief       Modify the scale of L3G in units of degrees per second
         *
         * @param[in]   dpsMode     Desired full-scale mode
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        void set_dps (const DPSMode dpsMode) const {
            this->maybe_set_spi_mode();

            uint8_t registerValue = this->read(Register::CTRL_REG4);
            registerValue &= ~(BIT_5 | BIT_4);
            registerValue |= dpsMode;
            this->write(Register::CTRL_REG4, registerValue);
        }

        /**
         * @brief       Write one byte to the L3G module
         *
         * @param[in]   address     Destination register address
         * @param[in]   value       Data to be written to the destination register
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        void write (const Register address, const uint8_t registerValue) const {
            const uint8_t  commandByte  = static_cast<uint8_t>(address) & ~BIT_7;  // Clear the RW bit (write mode)
            const uint16_t combinedWord = commandByte << 8 | registerValue;

            this->maybe_set_spi_mode();

            this->m_cs.clear();
            this->m_spi->shift_out(16, combinedWord);
            this->m_cs.set();
        }

        /**
         * @brief       Read one byte from the L3G module
         *
         * @param[in]   address     Origin register address
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        uint8_t read (const Register address) const {
            // Set RW bit and enable auto-increment
            const uint8_t commandByte = static_cast<uint8_t>(address) | BIT_7 | BIT_6;

            this->maybe_set_spi_mode();

            this->m_cs.clear();
            this->m_spi->shift_out(8, commandByte);
            const auto registerValue = static_cast<uint8_t>(this->m_spi->shift_in(8));
            this->m_cs.set();

            return registerValue;
        }

        /**
         * @brief       Set a single bit in a register on the device
         *
         * This method is highly inefficient (runtime performance) for multi-bit modifications. Use this only when
         * code size or code legibility is more important than runtime performance.
         *
         * @param[in]   address     Address of the register to modify
         * @param[in]   bit         Bit in the register that should be set high
         */
        void set_bit (const Register address, const Bit bit) const {
            const auto startingValue = this->read(address);
            this->write(address, bit | startingValue);
        }

        /**
         * @brief       Clear a single bit in a register on the device
         *
         * This method is highly inefficient (runtime performance) for multi-bit modifications. Use this only when
         * code size or code legibility is more important than runtime performance.
         *
         * @param[in]   address     Address of the register to modify
         * @param[in]   bit         Bit in the register that should be cleared low
         */
        void clear_bit (const Register address, const Bit bit) const {
            const auto startingValue = this->read(address);
            this->write(address, ~bit & startingValue);
        }

    protected:
        /**
         * @brief       Read two bytes from the L3G module
         *
         * @param[in]   address    Origin register address
         *
         * @return      Returns 0 upon success, error code otherwise
         */
        int16_t read16 (Register address) const {
            // Set RW bit and enable auto-increment
            const uint8_t commandByte = static_cast<uint8_t>(address) | BIT_7 | BIT_6;

            this->maybe_set_spi_mode();

            this->m_cs.clear();
            this->m_spi->shift_out(8, commandByte);
            uint16_t registerValue = static_cast<uint16_t>(this->m_spi->shift_in(16));
            this->m_cs.set();

            uint_fast8_t temp = registerValue >> 8;
            registerValue <<= 8;
            return static_cast<int16_t>(registerValue | temp);
        }

        /**
         * @brief   Set the SPI mode iff m_alwaysSetMode is true
         *
         * @return  Returns 0 upon success, error code otherwise
         */
        void maybe_set_spi_mode () const {
            if (this->m_alwaysSetMode) {
                this->m_spi->set_mode(SPI_MODE);
                this->m_spi->set_bit_mode(SPI_BITMODE);
            }
        }

    private:
        SPI       *m_spi;
        const Pin m_cs;
        bool      m_alwaysSetMode;
};

}
