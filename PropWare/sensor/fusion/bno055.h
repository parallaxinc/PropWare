/**
 * @file    PropWare/sensor/fusion/bno055.h
 *
 * @author  David Zemon (Only modified for use in PropWare)
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

#include <PropWare/serial/i2c/i2cmaster.h>
#include <PropWare/utility/ahrs/quaternion.h>
#include <PropWare/utility/ahrs/vector.h>

namespace PropWare {

/**
 * @brief   This is a library for the BNO055 orientation sensor
 *
 * Designed specifically to work with the Adafruit BNO055 Breakout.
 *
 * Pick one up today in the adafruit shop!
 * ------> http://www.adafruit.com/products
 *
 * These sensors use I2C to communicate, 2 pins are required to interface.
 *
 * Adafruit invests time and resources providing this open source code, please support Adafruit andopen-source
 * hardware by purchasing products from Adafruit!
 *
 * Written by KTOWN for Adafruit Industries.
 *
 * MIT license, all text above must be included in any redistribution
 */
class BNO055 {
    public:
        static const uint8_t DEVICE_ADDRESS_A = 0x28 << 1;
        static const uint8_t DEVICE_ADDRESS_B = 0x29 << 1;
        static const uint8_t ID        = 0xA0;

        enum class Address {
                /* Page id register definition */
                    PAGE_ID = 0X07,

                /* PAGE0 REGISTER DEFINITION START*/
                    CHIP_ID       = 0x00,
                    ACCEL_REV_ID  = 0x01,
                    MAG_REV_ID    = 0x02,
                    GYRO_REV_ID   = 0x03,
                    SW_REV_ID_LSB = 0x04,
                    SW_REV_ID_MSB = 0x05,
                    BL_REV_ID     = 0X06,

                /* Accel data register */
                    ACCEL_DATA_X_LSB = 0X08,
                    ACCEL_DATA_X_MSB = 0X09,
                    ACCEL_DATA_Y_LSB = 0X0A,
                    ACCEL_DATA_Y_MSB = 0X0B,
                    ACCEL_DATA_Z_LSB = 0X0C,
                    ACCEL_DATA_Z_MSB = 0X0D,

                /* Mag data register */
                    MAG_DATA_X_LSB = 0X0E,
                    MAG_DATA_X_MSB = 0X0F,
                    MAG_DATA_Y_LSB = 0X10,
                    MAG_DATA_Y_MSB = 0X11,
                    MAG_DATA_Z_LSB = 0X12,
                    MAG_DATA_Z_MSB = 0X13,

                /* Gyro data registers */
                    GYRO_DATA_X_LSB = 0X14,
                    GYRO_DATA_X_MSB = 0X15,
                    GYRO_DATA_Y_LSB = 0X16,
                    GYRO_DATA_Y_MSB = 0X17,
                    GYRO_DATA_Z_LSB = 0X18,
                    GYRO_DATA_Z_MSB = 0X19,

                /* Euler data registers */
                    EULER_H_LSB = 0X1A,
                    EULER_H_MSB = 0X1B,
                    EULER_R_LSB = 0X1C,
                    EULER_R_MSB = 0X1D,
                    EULER_P_LSB = 0X1E,
                    EULER_P_MSB = 0X1F,

                /* Quaternion data registers */
                    QUATERNION_DATA_W_LSB = 0X20,
                    QUATERNION_DATA_W_MSB = 0X21,
                    QUATERNION_DATA_X_LSB = 0X22,
                    QUATERNION_DATA_X_MSB = 0X23,
                    QUATERNION_DATA_Y_LSB = 0X24,
                    QUATERNION_DATA_Y_MSB = 0X25,
                    QUATERNION_DATA_Z_LSB = 0X26,
                    QUATERNION_DATA_Z_MSB = 0X27,

                /* Linear acceleration data registers */
                    LINEAR_ACCEL_DATA_X_LSB = 0X28,
                    LINEAR_ACCEL_DATA_X_MSB = 0X29,
                    LINEAR_ACCEL_DATA_Y_LSB = 0X2A,
                    LINEAR_ACCEL_DATA_Y_MSB = 0X2B,
                    LINEAR_ACCEL_DATA_Z_LSB = 0X2C,
                    LINEAR_ACCEL_DATA_Z_MSB = 0X2D,

                /* Gravity data registers */
                    GRAVITY_DATA_X_LSB = 0X2E,
                    GRAVITY_DATA_X_MSB = 0X2F,
                    GRAVITY_DATA_Y_LSB = 0X30,
                    GRAVITY_DATA_Y_MSB = 0X31,
                    GRAVITY_DATA_Z_LSB = 0X32,
                    GRAVITY_DATA_Z_MSB = 0X33,

                /* Temperature data register */
                    TEMP = 0X34,

                /* Status registers */
                    CALIB_STAT      = 0X35,
                    SELFTEST_RESULT = 0X36,
                    INTR_STAT       = 0X37,

                SYS_CLK_STAT = 0X38,
                SYS_STAT     = 0X39,
                SYS_ERR      = 0X3A,

                /* Unit selection register */
                    UNIT_SEL    = 0X3B,
                    DATA_SELECT = 0X3C,

                /* Mode registers */
                    OPR_MODE = 0X3D,
                    PWR_MODE = 0X3E,

                SYS_TRIGGER = 0X3F,
                TEMP_SOURCE = 0X40,

                /* Axis remap registers */
                    AXIS_MAP_CONFIG = 0X41,
                    AXIS_MAP_SIGN   = 0X42,

                /* SIC registers */
                    SIC_MATRIX_0_LSB = 0X43,
                    SIC_MATRIX_0_MSB = 0X44,
                    SIC_MATRIX_1_LSB = 0X45,
                    SIC_MATRIX_1_MSB = 0X46,
                    SIC_MATRIX_2_LSB = 0X47,
                    SIC_MATRIX_2_MSB = 0X48,
                    SIC_MATRIX_3_LSB = 0X49,
                    SIC_MATRIX_3_MSB = 0X4A,
                    SIC_MATRIX_4_LSB = 0X4B,
                    SIC_MATRIX_4_MSB = 0X4C,
                    SIC_MATRIX_5_LSB = 0X4D,
                    SIC_MATRIX_5_MSB = 0X4E,
                    SIC_MATRIX_6_LSB = 0X4F,
                    SIC_MATRIX_6_MSB = 0X50,
                    SIC_MATRIX_7_LSB = 0X51,
                    SIC_MATRIX_7_MSB = 0X52,
                    SIC_MATRIX_8_LSB = 0X53,
                    SIC_MATRIX_8_MSB = 0X54,

                /* Accelerometer Offset registers */
                    ACCEL_OFFSET_X_LSB = 0X55,
                    ACCEL_OFFSET_X_MSB = 0X56,
                    ACCEL_OFFSET_Y_LSB = 0X57,
                    ACCEL_OFFSET_Y_MSB = 0X58,
                    ACCEL_OFFSET_Z_LSB = 0X59,
                    ACCEL_OFFSET_Z_MSB = 0X5A,

                /* Magnetometer Offset registers */
                    MAG_OFFSET_X_LSB = 0X5B,
                    MAG_OFFSET_X_MSB = 0X5C,
                    MAG_OFFSET_Y_LSB = 0X5D,
                    MAG_OFFSET_Y_MSB = 0X5E,
                    MAG_OFFSET_Z_LSB = 0X5F,
                    MAG_OFFSET_Z_MSB = 0X60,

                /* Gyroscope Offset register s*/
                    GYRO_OFFSET_X_LSB = 0X61,
                    GYRO_OFFSET_X_MSB = 0X62,
                    GYRO_OFFSET_Y_LSB = 0X63,
                    GYRO_OFFSET_Y_MSB = 0X64,
                    GYRO_OFFSET_Z_LSB = 0X65,
                    GYRO_OFFSET_Z_MSB = 0X66,

                /* Radius registers */
                    ACCEL_RADIUS_LSB = 0X67,
                    ACCEL_RADIUS_MSB = 0X68,
                    MAG_RADIUS_LSB   = 0X69,
                    MAG_RADIUS_MSB   = 0X6A
        };

        typedef enum {
            POWER_MODE_NORMAL   = 0X00,
            POWER_MODE_LOWPOWER = 0X01,
            POWER_MODE_SUSPEND  = 0X02
        }                    PowerMode;

        enum class OperationMode {
                /* Operation mode settings*/
                    CONFIG       = 0X00,
                    ACCONLY      = 0X01,
                    MAGONLY      = 0X02,
                    GYRONLY      = 0X03,
                    ACCMAG       = 0X04,
                    ACCGYRO      = 0X05,
                    MAGGYRO      = 0X06,
                    AMG          = 0X07,
                    IMUPLUS      = 0X08,
                    COMPASS      = 0X09,
                    M4G          = 0X0A,
                    NDOF_FMC_OFF = 0X0B,
                    NDOF         = 0X0C
        };

        typedef enum {
            REMAP_CONFIG_P0 = 0x21,
            REMAP_CONFIG_P1 = 0x24, // default
            REMAP_CONFIG_P2 = 0x24,
            REMAP_CONFIG_P3 = 0x21,
            REMAP_CONFIG_P4 = 0x24,
            REMAP_CONFIG_P5 = 0x21,
            REMAP_CONFIG_P6 = 0x21,
            REMAP_CONFIG_P7 = 0x24
        }                    AxisRemapConfiguration;

        typedef enum {
            P0 = 0x04,
            P1 = 0x00, // default
            P2 = 0x06,
            P3 = 0x02,
            P4 = 0x03,
            P5 = 0x01,
            P6 = 0x07,
            P7 = 0x05
        }                    AxisRemapSign;

        typedef enum {
            ACCELEROMETER = static_cast<uint8_t>(Address::ACCEL_DATA_X_LSB),
            MAGNETOMETER  = static_cast<uint8_t>(Address::MAG_DATA_X_LSB),
            GYROSCOPE     = static_cast<uint8_t>(Address::GYRO_DATA_X_LSB),
            EULER         = static_cast<uint8_t>(Address::EULER_H_LSB),
            LINEARACCEL   = static_cast<uint8_t>(Address::LINEAR_ACCEL_DATA_X_LSB),
            GRAVITY       = static_cast<uint8_t>(Address::GRAVITY_DATA_X_LSB)
        }                    VectorType;

        typedef enum {
            NO_ERROR       = 0,
            BEG_ERROR      = 256,
            FAILED_TO_BOOT = BEG_ERROR,
            END_ERROR      = FAILED_TO_BOOT
        }                    ErrorCode;

        struct Offsets {
            uint16_t accel_x;
            uint16_t accel_y;
            uint16_t accel_z;
            uint16_t gyro_x;
            uint16_t gyro_y;
            uint16_t gyro_z;
            uint16_t mag_x;
            uint16_t mag_y;
            uint16_t mag_z;

            uint16_t accel_radius;
            uint16_t mag_radius;
        };

        struct RevisionInfo {
            uint8_t  accel;
            uint8_t  mag;
            uint8_t  gyro;
            uint16_t sw;
            uint8_t  bl;
        };

    public:
        static uint8_t convert (const Address address) {
            return static_cast<uint8_t>(address);
        }

        static uint8_t convert (const OperationMode operationMode) {
            return static_cast<uint8_t>(operationMode);
        }

    public:
        BNO055 (const uint8_t address = DEVICE_ADDRESS_A, const I2CMaster &bus = pwI2c)
            : m_bus(&bus),
              m_address(address) {
        }

        uint8_t get_byte(const Address address) const {
            const uint8_t x = convert(address);
            return this->m_bus->get(this->m_address, x);
        }

        PropWare::ErrorCode begin (const OperationMode mode = OperationMode::NDOF) {
            /* Make sure we have the right device */
            auto id = this->get_byte(Address::CHIP_ID);
            if (id != ID) {
                waitcnt(SECOND + CNT); // hold on for boot
                id = this->get_byte(Address::CHIP_ID);
                if (id != ID) {
                    pwOut << "fail 0\n";
                    return FAILED_TO_BOOT;  // still not? ok bail
                }
            }

            /* Switch to config mode (just in case since this is the default) */
            if (!this->set_mode(OperationMode::CONFIG)) {
                pwOut << "fail 1\n";
                return FAILED_TO_BOOT;
            }

            /* Reset */
            if (!this->m_bus->put(this->m_address, convert(Address::SYS_TRIGGER), 0x20)) {
                pwOut << "fail 2\n";
                return FAILED_TO_BOOT;
            }
            while (this->m_bus->get(this->m_address, convert(Address::CHIP_ID)) != ID)
                waitcnt(10 * MILLISECOND + CNT);
            waitcnt(50 * MILLISECOND + CNT);

            /* Set to normal power mode */
            if (!this->m_bus->put(this->m_address, convert(Address::PWR_MODE), POWER_MODE_NORMAL)) {
                pwOut << "fail 3\n";
                return FAILED_TO_BOOT;
            }
            waitcnt(10 * MILLISECOND + CNT);

            if (!this->m_bus->put(this->m_address, convert(Address::PAGE_ID), 0)) {
                pwOut << "fail 4\n";
                return FAILED_TO_BOOT;
            }

            /* Set the output units */
            /*
            uint8_t unitsel = (0 << 7) | // Orientation = Android
                              (0 << 4) | // Temperature = Celsius
                              (0 << 2) | // Euler = Degrees
                              (1 << 1) | // Gyro = Rads
                              (0 << 0);  // Accelerometer = m/s^2
            write8(Address::UNIT_SEL, unitsel);
            */

            /* Configure axis mapping (see section 3.4) */
            /*
            write8(Address::AXIS_MAP_CONFIG, REMAP_CONFIG_P2); // P0-P7, Default is P1
            delay(10);
            write8(Address::AXIS_MAP_SIGN, REMAP_SIGN_P2); // P0-P7, Default is P1
            delay(10);
            */

            if (!this->m_bus->put(this->m_address, convert(Address::SYS_TRIGGER), 0x0)) {
                pwOut << "fail 5\n";
                return FAILED_TO_BOOT;
            }
            waitcnt(10 * MILLISECOND + CNT);

            /* Set the requested operating mode (see section 3.3) */
            if (!this->set_mode(mode)) {
                pwOut << "fail 6\n";
                return FAILED_TO_BOOT;
            }
            waitcnt(20 * MILLISECOND + CNT);

            return NO_ERROR;
        }

        /**
         * @brief       Puts the chip in the specified operating mode
         *
         * @param[out]  mode    Desired mode of operation
         *
         * @return      True if the write was ACKed, false otherwise
         */
        bool set_mode (const OperationMode mode) {
            this->_mode = mode;
            const auto ack = this->m_bus->put(this->m_address, convert(Address::OPR_MODE), convert(mode));
            waitcnt(30 * MILLISECOND + CNT);
            return ack;
        }

        /**
         * @brief       Use the external 32.768KHz crystal
         *
         * @param[in]   useExternalCrystal  When set, the external crystal will be used.
         */
        void set_external_crystal_use (const bool useExternalCrystal) {
            const OperationMode startingMode = _mode;

            /* Switch to config mode (just in case since this is the default) */
            this->set_mode(OperationMode::CONFIG);
            waitcnt(25 * MILLISECOND + CNT);
            this->m_bus->put(this->m_address, convert(Address::PAGE_ID), 0);
            if (useExternalCrystal)
                this->m_bus->put(this->m_address, convert(Address::SYS_TRIGGER), 0x80);
            else
                this->m_bus->put(this->m_address, convert(Address::SYS_TRIGGER), 0x00);
            waitcnt(10 * MILLISECOND + CNT);

            /* Set the requested operating mode (see section 3.3) */
            this->set_mode(startingMode);
            waitcnt(20 * MILLISECOND + CNT);
        }

        /**
         * @brief       Gets the chip's revision numbers
         *
         * @param[out]  info
         */
        void get_revision_info (RevisionInfo *info) const {

            memset(info, 0, sizeof(RevisionInfo));

            /* Check the accelerometer revision */
            info->accel = this->m_bus->get(this->m_address, convert(Address::ACCEL_REV_ID));

            /* Check the magnetometer revision */
            info->mag = this->m_bus->get(this->m_address, convert(Address::MAG_REV_ID));

            /* Check the gyroscope revision */
            info->gyro = this->m_bus->get(this->m_address, convert(Address::GYRO_REV_ID));

            /* Check the SW revision */
            info->bl = this->m_bus->get(this->m_address, convert(Address::BL_REV_ID));

            const auto a = this->m_bus->get(this->m_address, convert(Address::SW_REV_ID_LSB));
            const auto b = this->m_bus->get(this->m_address, convert(Address::SW_REV_ID_MSB));
            info->sw = (static_cast<uint16_t>(b) << 8) | a;
        }

        /**
         * @brief       Gets the latest system status information
         *
         * @param[out]  systemStatus
         * @param[out]  selfTestResult
         * @param[out]  systemError
         */
        void get_system_status (uint8_t *systemStatus, uint8_t *selfTestResult, uint8_t *systemError) const {
            this->m_bus->put(this->m_address, convert(Address::PAGE_ID), 0);

            /* System Status (see section 4.3.58)
               ---------------------------------
               0 = Idle
               1 = System Error
               2 = Initializing Peripherals
               3 = System Iniitalization
               4 = Executing Self-Test
               5 = Sensor fusio algorithm running
               6 = System running without fusion algorithms */

            if (0 != systemStatus)
                *systemStatus = this->m_bus->get(this->m_address, convert(Address::SYS_STAT));

            /* Self Test Results (see section )
               --------------------------------
               1 = test passed, 0 = test failed

               Bit 0 = Accelerometer self test
               Bit 1 = Magnetometer self test
               Bit 2 = Gyroscope self test
               Bit 3 = MCU self test

               0x0F = all good! */

            if (0 != selfTestResult)
                *selfTestResult = this->m_bus->get(this->m_address, convert(Address::SELFTEST_RESULT));

            /* System Error (see section 4.3.59)
               ---------------------------------
               0 = No error
               1 = Peripheral initialization error
               2 = System initialization error
               3 = Self test result failed
               4 = Register map value out of range
               5 = Register map address out of range
               6 = Register map write error
               7 = BNO low power mode not available for selected operat ion mode
               8 = Accelerometer power mode not available
               9 = Fusion algorithm configuration error
               A = Sensor configuration error */

            if (systemError != 0)
                *systemError = this->m_bus->get(this->m_address, convert(Address::SYS_ERR));

            waitcnt(200 * MILLISECOND + CNT);
        }

        /**
         * @brief  Gets current calibration state.
         *
         * Each value will be set to 0 if not calibrated and 3 if fully calibrated.
         *
         * @param[out]  system
         * @param[out]  gyro
         * @param[out]  accel
         * @param[out]  magnitude
         */
        void get_calibration (uint8_t *system, uint8_t *gyro, uint8_t *accel, uint8_t *magnitude) const {
            const auto calData = this->m_bus->get(this->m_address, convert(Address::CALIB_STAT));
            if (NULL != system)
                *system    = (calData >> 6) & static_cast<uint8_t>(0x03);
            if (NULL != gyro)
                *gyro      = (calData >> 4) & static_cast<uint8_t>(0x03);
            if (NULL != accel)
                *accel     = (calData >> 2) & static_cast<uint8_t>(0x03);
            if (NULL != magnitude)
                *magnitude = calData & static_cast<uint8_t>(0x03);
        }

        /**
         * @brief  Gets a vector reading from the specified source
         *
         * @param[in]   type
         * @param[out]  result
         */
        imu::Vector<3> get_vector (const VectorType type) const {
            uint8_t buffer[6];
            this->m_bus->get(this->m_address, static_cast<uint8_t>(type), buffer, 6);

            const auto x = ((int16_t) buffer[0]) | (((int16_t) buffer[1]) << 8);
            const auto y = ((int16_t) buffer[2]) | (((int16_t) buffer[3]) << 8);
            const auto z = ((int16_t) buffer[4]) | (((int16_t) buffer[5]) << 8);

            /* Convert the value to an appropriate range (section 3.6.4) */
            /* and assign the value to the Vector type */
            double scalar;
            switch (type) {
                case MAGNETOMETER:
                case GYROSCOPE:
                case EULER:
                    scalar = 16;
                    break;
                case ACCELEROMETER:
                case LINEARACCEL:
                case GRAVITY:
                    scalar = 100;
                    break;
                default:
                    scalar = 1;
            }

            return imu::Vector<3>(x / scalar, y / scalar, z / scalar);
        }

        imu::Quaternion get_quaternion () const {
            uint8_t buffer[8];
            this->m_bus->get(this->m_address, convert(Address::QUATERNION_DATA_W_LSB), buffer, 8);
            const auto w = (static_cast<uint16_t>(buffer[1]) << 8) | buffer[0];
            const auto x = (static_cast<uint16_t>(buffer[3]) << 8) | buffer[2];
            const auto y = (static_cast<uint16_t>(buffer[5]) << 8) | buffer[4];
            const auto z = (static_cast<uint16_t>(buffer[7]) << 8) | buffer[6];

            /* Assign to Quaternion */
            /* See http://ae-bst.resource.bosch.com/media/products/dokumente/bno055/BST_Address::DS000_12~1.pdf
               3.6.5.5 Orientation (Quaternion)  */
            const double scale = 1.0 / (1 << 14);
            return imu::Quaternion(scale * w, scale * x, scale * y, scale * z);
        }

        int8_t get_temperature () const {
            return this->m_bus->get(this->m_address, convert(Address::TEMP));
        }

        /* Functions to deal with raw calibration data */
        bool get_sensor_offsets (uint8_t *calibrationData) {
            if (is_fully_calibrated()) {
                const OperationMode lastMode = this->_mode;
                this->set_mode(OperationMode::CONFIG);

                const auto bytes = static_cast<size_t>(
                    convert(Address::MAG_RADIUS_MSB)
                        - convert(Address::ACCEL_OFFSET_X_LSB)
                        + 1
                );
                this->m_bus->get(this->m_address, convert(Address::ACCEL_OFFSET_X_LSB), calibrationData, bytes);

                this->set_mode(lastMode);
                return true;
            } else
                return false;
        }

        bool get_sensor_offsets (Offsets &offsets) {
            if (is_fully_calibrated()) {
                const OperationMode lastMode = this->_mode;
                this->set_mode(OperationMode::CONFIG);
                waitcnt(25 * MILLISECOND + CNT);

                const auto bus = this->m_bus;
                offsets.accel_x = (bus->get(this->m_address, convert(Address::ACCEL_OFFSET_X_MSB)) << 8)
                    | bus->get(this->m_address, convert(Address::ACCEL_OFFSET_X_LSB));
                offsets.accel_y = (bus->get(this->m_address, convert(Address::ACCEL_OFFSET_Y_MSB)) << 8)
                    | bus->get(this->m_address, convert(Address::ACCEL_OFFSET_Y_LSB));
                offsets.accel_z = (bus->get(this->m_address, convert(Address::ACCEL_OFFSET_Z_MSB)) << 8)
                    | bus->get(this->m_address, convert(Address::ACCEL_OFFSET_Z_LSB));

                offsets.gyro_x = (bus->get(this->m_address, convert(Address::GYRO_OFFSET_X_MSB)) << 8)
                    | bus->get(this->m_address, convert(Address::GYRO_OFFSET_X_LSB));
                offsets.gyro_y = (bus->get(this->m_address, convert(Address::GYRO_OFFSET_Y_MSB)) << 8)
                    | bus->get(this->m_address, convert(Address::GYRO_OFFSET_Y_LSB));
                offsets.gyro_z = (bus->get(this->m_address, convert(Address::GYRO_OFFSET_Z_MSB)) << 8)
                    | bus->get(this->m_address, convert(Address::GYRO_OFFSET_Z_LSB));

                offsets.mag_x = (bus->get(this->m_address, convert(Address::MAG_OFFSET_X_MSB)) << 8)
                    | bus->get(this->m_address, convert(Address::MAG_OFFSET_X_LSB));
                offsets.mag_y = (bus->get(this->m_address, convert(Address::MAG_OFFSET_Y_MSB)) << 8)
                    | bus->get(this->m_address, convert(Address::MAG_OFFSET_Y_LSB));
                offsets.mag_z = (bus->get(this->m_address, convert(Address::MAG_OFFSET_Z_MSB)) << 8)
                    | bus->get(this->m_address, convert(Address::MAG_OFFSET_Z_LSB));

                offsets.accel_radius = (bus->get(this->m_address, convert(Address::ACCEL_RADIUS_MSB)) << 8)
                    | bus->get(this->m_address, convert(Address::ACCEL_RADIUS_LSB));
                offsets.mag_radius   = (bus->get(this->m_address, convert(Address::MAG_RADIUS_MSB)) << 8)
                    | bus->get(this->m_address, convert(Address::MAG_RADIUS_LSB));

                this->set_mode(lastMode);
                return true;
            } else
                return false;
        }

        void set_sensor_offsets (const uint8_t *calibrationData) {
            const OperationMode lastMode = this->_mode;
            this->set_mode(OperationMode::CONFIG);
            waitcnt(25 * MILLISECOND + CNT);

            /* A writeLen() would make this much cleaner */
            this->m_bus->put(this->m_address, convert(Address::ACCEL_OFFSET_X_LSB), calibrationData[0]);
            this->m_bus->put(this->m_address, convert(Address::ACCEL_OFFSET_X_MSB), calibrationData[1]);
            this->m_bus->put(this->m_address, convert(Address::ACCEL_OFFSET_Y_LSB), calibrationData[2]);
            this->m_bus->put(this->m_address, convert(Address::ACCEL_OFFSET_Y_MSB), calibrationData[3]);
            this->m_bus->put(this->m_address, convert(Address::ACCEL_OFFSET_Z_LSB), calibrationData[4]);
            this->m_bus->put(this->m_address, convert(Address::ACCEL_OFFSET_Z_MSB), calibrationData[5]);

            this->m_bus->put(this->m_address, convert(Address::GYRO_OFFSET_X_LSB), calibrationData[6]);
            this->m_bus->put(this->m_address, convert(Address::GYRO_OFFSET_X_MSB), calibrationData[7]);
            this->m_bus->put(this->m_address, convert(Address::GYRO_OFFSET_Y_LSB), calibrationData[8]);
            this->m_bus->put(this->m_address, convert(Address::GYRO_OFFSET_Y_MSB), calibrationData[9]);
            this->m_bus->put(this->m_address, convert(Address::GYRO_OFFSET_Z_LSB), calibrationData[10]);
            this->m_bus->put(this->m_address, convert(Address::GYRO_OFFSET_Z_MSB), calibrationData[11]);

            this->m_bus->put(this->m_address, convert(Address::MAG_OFFSET_X_LSB), calibrationData[12]);
            this->m_bus->put(this->m_address, convert(Address::MAG_OFFSET_X_MSB), calibrationData[13]);
            this->m_bus->put(this->m_address, convert(Address::MAG_OFFSET_Y_LSB), calibrationData[14]);
            this->m_bus->put(this->m_address, convert(Address::MAG_OFFSET_Y_MSB), calibrationData[15]);
            this->m_bus->put(this->m_address, convert(Address::MAG_OFFSET_Z_LSB), calibrationData[16]);
            this->m_bus->put(this->m_address, convert(Address::MAG_OFFSET_Z_MSB), calibrationData[17]);

            this->m_bus->put(this->m_address, convert(Address::ACCEL_RADIUS_LSB), calibrationData[18]);
            this->m_bus->put(this->m_address, convert(Address::ACCEL_RADIUS_MSB), calibrationData[19]);

            this->m_bus->put(this->m_address, convert(Address::MAG_RADIUS_LSB), calibrationData[20]);
            this->m_bus->put(this->m_address, convert(Address::MAG_RADIUS_MSB), calibrationData[21]);

            this->set_mode(lastMode);
        }

        void set_sensor_offsets (const Offsets &offsets) {
            const OperationMode lastMode = this->_mode;
            this->set_mode(OperationMode::CONFIG);
            waitcnt(25 * MILLISECOND + CNT);

            const auto bus = this->m_bus;
            bus->put(this->m_address, convert(Address::ACCEL_OFFSET_X_LSB), static_cast<uint8_t>(offsets.accel_x));
            bus->put(this->m_address, convert(Address::ACCEL_OFFSET_X_MSB), static_cast<uint8_t>(offsets.accel_x >> 8));
            bus->put(this->m_address, convert(Address::ACCEL_OFFSET_Y_LSB), static_cast<uint8_t>(offsets.accel_y));
            bus->put(this->m_address, convert(Address::ACCEL_OFFSET_Y_MSB), static_cast<uint8_t>(offsets.accel_y >> 8));
            bus->put(this->m_address, convert(Address::ACCEL_OFFSET_Z_LSB), static_cast<uint8_t>(offsets.accel_z));
            bus->put(this->m_address, convert(Address::ACCEL_OFFSET_Z_MSB), static_cast<uint8_t>(offsets.accel_z >> 8));

            bus->put(this->m_address, convert(Address::GYRO_OFFSET_X_LSB), static_cast<uint8_t>(offsets.gyro_x));
            bus->put(this->m_address, convert(Address::GYRO_OFFSET_X_MSB), static_cast<uint8_t>(offsets.gyro_x >> 8));
            bus->put(this->m_address, convert(Address::GYRO_OFFSET_Y_LSB), static_cast<uint8_t>(offsets.gyro_y));
            bus->put(this->m_address, convert(Address::GYRO_OFFSET_Y_MSB), static_cast<uint8_t>(offsets.gyro_y >> 8));
            bus->put(this->m_address, convert(Address::GYRO_OFFSET_Z_LSB), static_cast<uint8_t>(offsets.gyro_z));
            bus->put(this->m_address, convert(Address::GYRO_OFFSET_Z_MSB), static_cast<uint8_t>(offsets.gyro_z >> 8));

            bus->put(this->m_address, convert(Address::MAG_OFFSET_X_LSB), static_cast<uint8_t>(offsets.mag_x));
            bus->put(this->m_address, convert(Address::MAG_OFFSET_X_MSB), static_cast<uint8_t>(offsets.mag_x >> 8));
            bus->put(this->m_address, convert(Address::MAG_OFFSET_Y_LSB), static_cast<uint8_t>(offsets.mag_y));
            bus->put(this->m_address, convert(Address::MAG_OFFSET_Y_MSB), static_cast<uint8_t>(offsets.mag_y >> 8));
            bus->put(this->m_address, convert(Address::MAG_OFFSET_Z_LSB), static_cast<uint8_t>(offsets.mag_z));
            bus->put(this->m_address, convert(Address::MAG_OFFSET_Z_MSB), static_cast<uint8_t>(offsets.mag_z >> 8));

            bus->put(this->m_address, convert(Address::ACCEL_RADIUS_LSB), static_cast<uint8_t>(offsets.accel_radius));
            bus->put(this->m_address,
                     convert(Address::ACCEL_RADIUS_MSB),
                     static_cast<uint8_t>(offsets.accel_radius >> 8));

            bus->put(this->m_address, convert(Address::MAG_RADIUS_LSB), static_cast<uint8_t>(offsets.mag_radius));
            bus->put(this->m_address, convert(Address::MAG_RADIUS_MSB), static_cast<uint8_t>(offsets.mag_radius >> 8));

            this->set_mode(lastMode);
        }

        bool is_fully_calibrated () const {
            uint8_t system, gyro, accel, mag;
            this->get_calibration(&system, &gyro, &accel, &mag);
            return !(system < 3 || gyro < 3 || accel < 3 || mag < 3);
        }

    private:
        const I2CMaster *m_bus;
        const uint8_t   m_address;
        OperationMode   _mode;
};

}
