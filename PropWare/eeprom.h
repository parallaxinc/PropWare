/**
 * @file    eeprom.h
 *
 * @author  David Zemon
 *
 * @copyright
 * The MIT License (MIT)<br>
 * <br>Copyright (c) 2013 David Zemon<br>
 * <br>Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
 * to permit persons to whom the Software is furnished to do so, subject to the following conditions:<br>
 * <br>The above copyright notice and this permission notice shall be included in all copies or substantial portions
 * of the Software.<br>
 * <br>THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#pragma once

#include <PropWare/i2c.h>
#include <PropWare/printcapable.h>
#include <PropWare/scancapable.h>

namespace PropWare {

/**
 * @brief   EEPROM reader/writer
 */
class Eeprom: public PrintCapable,
              public ScanCapable {
    public:
        static const uint8_t  DEFAULT_DEVICE_ADDRESS         = 0x50 << 1;
        /** First byte above the standard 32kB EEPROM */
        static const uint16_t DEFAULT_INITIAL_MEMORY_ADDRESS = 32 * 1024;

    public:
        Eeprom(const I2C &driver = pwI2c,
               const uint16_t initialMemoryAddress = DEFAULT_INITIAL_MEMORY_ADDRESS,
               const uint8_t deviceAddress = DEFAULT_DEVICE_ADDRESS,
               bool autoIncrement = true)
            : m_driver(&driver),
              m_memoryAddress(initialMemoryAddress),
              m_deviceAddress(deviceAddress),
              m_autoIncrement(autoIncrement) {
        }

        bool ping() const {
            return this->m_driver->ping(this->m_deviceAddress);
        }

        bool put(const uint16_t address, const uint8_t byte) const {
            // Wait for any current operation to finish
            while (!this->ping());

            return this->m_driver->put(this->m_deviceAddress, address, byte);
        }

        bool put(const uint16_t startAddress, const uint8_t bytes[], const size_t length) const {
            // Wait for any current operation to finish
            while (!this->ping());

            return this->m_driver->put(this->m_deviceAddress, startAddress, bytes, length);
        }

        virtual void put_char(const char c) {
            this->put(this->m_memoryAddress, (uint8_t) c);
            if (this->m_autoIncrement)
                ++this->m_memoryAddress;
        }

        virtual void puts(const char *string) {
            const size_t stringLength = strlen(string);
            this->put(this->m_memoryAddress, (const uint8_t *) string, stringLength);
            if (this->m_autoIncrement)
                this->m_memoryAddress += stringLength;
        }

        uint8_t get(const uint16_t address) const {
            // Wait for any current operation to finish
            while (!this->ping());

            return this->m_driver->get(this->m_deviceAddress, address);
        }

        virtual char get_char() {
            const uint8_t byte = this->get(this->m_memoryAddress);
            if (this->m_autoIncrement)
                ++this->m_memoryAddress;
            return byte;
        }

        uint16_t get_memory_address() const {
            return this->m_memoryAddress;
        }

        void set_memory_address(const uint16_t address) {
            this->m_memoryAddress = address;
        }

    private:
        const I2C     *m_driver;
        uint16_t      m_memoryAddress;
        const uint8_t m_deviceAddress;
        bool          m_autoIncrement;
};

}
