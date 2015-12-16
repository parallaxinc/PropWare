/**
 * @file    PropWare/pcf8591.h
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

#include <PropWare/i2c.h>
#include "utility.h"

namespace PropWare {

/**
 * @brief   A simple 4-channel, 8-bit ADC and single-channel 8-bit DAC both integrated into one chip
 */
class PCF8591 {
    public:
        typedef enum {
            CHANNEL_0,
            CHANNEL_1,
            CHANNEL_2,
            CHANNEL_3,
            CHANNELS
        } ADCChannel;

        typedef enum {
            AUTO_INC_OFF,
            AUTO_INC_ON = 0x04
        } ADCAutoIncrement;

        typedef enum {
            AllSingleEnded,
            ThreeDifferential     = 0x10,
            SingleAndDifferential = 0x20,
            TwoDifferential       = 0x30
        } ReadMode;

        static const uint8_t DEFAULT_DEVICE_ADDRESS = 0x90;
        static const uint8_t DAC_ENABLE             = BIT_6;
        static const uint8_t CHANNEL_BITS           = BIT_1 | BIT_0;

    public:
        /**
         * @brief       Constructor
         *
         * @param[in]   deviceAddress   The PCF8591 supports variable different device addresses, which can be set here;
         *                              The default address should satisfy most use cases
         * @param[in]   *i2cBus         HUB memory address (as opposed to I2C address!) of the I2C instance that should
         *                              be used for communication; Usually the default bus will be satisfactory
         */
        PCF8591 (const uint8_t deviceAddress = DEFAULT_DEVICE_ADDRESS, const I2C *i2cBus = &pwI2c)
                : m_i2c(i2cBus),
                  m_deviceAddress(deviceAddress),
                  m_currentProgram(0) {
        }

        /**
         * @brief       Ensure the device is responding on the I2C bus
         *
         * @returns     True if device responded, false otherwise
         */
        bool ping () const {
            return this->m_i2c->ping(this->m_deviceAddress);
        }

        /**
         * @brief       Read the ADC value from a given channel
         *
         * @param[in]   channel     The channel to be read
         *
         * @returns     a value between 0 and 255 representing the analog voltage
         */
        uint8_t read_channel (const ADCChannel channel) {
            this->set_channel(channel);
            this->set_auto_increment(false);
            uint8_t buffer[2];
            this->m_i2c->get(this->m_deviceAddress, buffer, sizeof(buffer));
            return buffer[sizeof(buffer) - 1];
        }

        /**
         * @brief       Retrieve the next ADC value for the previously set channel
         *
         * If you need sequential readings from the same channel, this method will more efficiently grab readings after
         * the first versus multiple calls to `PropWare::PCF8591::read_channel`.
         *
         * @param[out]  *data   Address where the ADC result will be stored
         *
         * @returns     True if successful, false otherwise
         */
        bool read_next (uint8_t *data) const {
            return this->m_i2c->get(this->m_deviceAddress, data, 1);
        }

        /**
         * @brief       Get multiple sequential readings from the same channel
         *
         * @param[out]  data[]  Array where readings can be stored
         * @param[in]   size    Length of the array and number readings that should be retrieved
         *
         * @returns     True if successfully, false if any errors occurred
         */
        bool read_multi (uint8_t data[], const size_t size) const {
            return this->m_i2c->get(this->m_deviceAddress, data, size);
        }

        /**
         * @brief       Use the given channel for subsequent queries
         *
         * @param[in]   channel     Channel number that should be used
         *
         * @returns     True if successfully set, false otherwise
         */
        bool set_channel (const ADCChannel channel) {
            this->m_currentProgram &= ~CHANNEL_BITS;
            this->m_currentProgram |= channel;
            return this->program();
        }

        /**
         * @brief       When enabled, auto increment will change to the next ADC channel with every query. This is
         *              normally off
         *
         * @param[in]   autoIncrement   True to enable, false to disable
         *
         * @returns     True if successfully set, false otherwise
         */
        bool set_auto_increment (const bool autoIncrement) {
            if (autoIncrement)
                this->m_currentProgram |= AUTO_INC_ON;
            else
                this->m_currentProgram &= ~AUTO_INC_ON;
            return this->program();
        }

        /**
         * @brief       Switch between the different modes for the ADC
         *
         * @param[in]   mode    Switch between single channel (default) and various differential modes
         *
         * @returns     True if successfully set, false otherwise
         */
        bool set_read_mode (const ReadMode mode) {
            this->m_currentProgram &= ~0x30;
            this->m_currentProgram |= mode;
            return this->program();
        }

        /**
         * @brief       Begin outputting the current value out the DAC line
         *
         * @returns     True if successful, false otherwise
         */
        bool enable_dac () {
            this->m_currentProgram |= DAC_ENABLE;
            return this->program();
        }

        /**
         * @brief       Turn off the DAC
         *
         * @returns     True if successful, false otherwise
         */
        bool disable_dac () {
            this->m_currentProgram &= ~DAC_ENABLE;
            return this->program();
        }

        /**
         * @brief       Set the current value for the DAC
         *
         * @param[in]   dacValue    Digital value which will be converted to an analog voltage
         *
         * @returns     True if successful, false otherwise
         */
        bool write (const uint8_t dacValue) {
            this->m_currentProgram |= DAC_ENABLE;
            return this->m_i2c->put(this->m_deviceAddress, this->m_currentProgram, dacValue);
        }

        /**
         * @brief       Write a series of values to the DAC sequentially
         *
         * @param[in]   dacValues   Array of values that should be sent out the DAC line
         * @param[in]   size        Number of values in the array
         *
         * @returns     True if successful, false otherwise
         */
        bool write_multi (const uint8_t dacValues[], const size_t size) {
            this->m_currentProgram |= DAC_ENABLE;
            return this->m_i2c->put(this->m_deviceAddress, this->m_currentProgram, dacValues, size);
        }

    private:
        bool program () const {
            return this->m_i2c->put(this->m_deviceAddress, this->m_currentProgram);
        }

    private:
        const I2C     *m_i2c;
        const uint8_t m_deviceAddress;

        uint8_t m_currentProgram;
};

}
