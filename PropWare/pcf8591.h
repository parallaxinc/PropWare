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

class PCF8591 {
    public:
        static const uint8_t DEFAULT_DEVICE_ADDRESS = 0x90;
        static const uint8_t DAC_ENABLE             = BIT_6;
        static const uint8_t CHANNEL_BITS           = BIT_1 | BIT_0;

        typedef enum {
            CHANNEL_0,
            CHANNEL_1,
            CHANNEL_2,
            CHANNEL_3,
            CHANNELS
        }                    ADCChannel;

        typedef enum {
            AUTO_INC_OFF,
            AUTO_INC_ON = 0x04
        }                    ADCAutoIncrement;

        typedef enum {
            AllSingleEnded,
            ThreeDifferential     = 0x10,
            SingleAndDifferential = 0x20,
            TwoDifferential       = 0x30
        }                    ReadMode;

    public:
        PCF8591 (const uint8_t deviceAddress = DEFAULT_DEVICE_ADDRESS, const I2C *i2cBus = &pwI2c)
                : m_i2c(i2cBus),
                  m_deviceAddress(deviceAddress),
                  m_currentProgram(0) {
        }

        bool ping () const {
            return this->m_i2c->ping(this->m_deviceAddress);
        }

        uint8_t read_channel (const ADCChannel channel) {
            this->set_channel(channel);
            this->set_auto_increment(false);
            uint8_t buffer[2];
            this->m_i2c->get(this->m_deviceAddress, buffer, sizeof(buffer));
            return buffer[sizeof(buffer) - 1];
        }

        bool read_next (uint8_t *data) const {
            return this->m_i2c->get(this->m_deviceAddress, data, 1);
        }

        bool read_multi (uint8_t data[], const size_t size) const {
            return this->m_i2c->get(this->m_deviceAddress, data, size);
        }

        bool set_channel (const ADCChannel channel) {
            this->m_currentProgram &= ~CHANNEL_BITS;
            this->m_currentProgram |= channel;
            return this->program();
        }

        bool set_auto_increment (const bool autoIncrement) {
            if (autoIncrement)
                this->m_currentProgram |= AUTO_INC_ON;
            else
                this->m_currentProgram &= ~AUTO_INC_ON;
            return this->program();
        }

        bool set_read_mode (const ReadMode mode) {
            this->m_currentProgram &= ~0x30;
            this->m_currentProgram |= mode;
            return this->program();
        }

        bool enable_dac () {
            this->m_currentProgram |= DAC_ENABLE;
            return this->program();
        }

        bool disable_dac () {
            this->m_currentProgram &= ~DAC_ENABLE;
            return this->program();
        }

        bool write (const uint8_t dacValue) {
            this->m_currentProgram |= DAC_ENABLE;
            return this->m_i2c->put(this->m_deviceAddress, this->m_currentProgram, dacValue);
        }

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
