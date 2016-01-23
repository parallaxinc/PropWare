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
        /** First byte above the standard 32kB EEPROM */
        static const uint16_t DEFAULT_INITIAL_MEMORY_ADDRESS = 32 * 1024;
        /** Standard EEPROM I2C address used for Propeller microcontrollers */
        static const uint8_t  DEFAULT_DEVICE_ADDRESS         = 0x50 << 1;

    public:
        /**
         * @brief       Construct an instance that, by default, will read from and write to the standard EEPROM
         *
         * @param[in]   driver                  I2C bus driver
         * @param[in]   initialMemoryAddress    Byte of EEPROM that should be read from or written to first when using
         *                                      methods that do not accept an address as a parameter
         * @param[in]   deviceAddress           EEPROM address on the I2C bus (for instance, the default device
         *                                      address would be passed in as `0x50 << 1`, _not_ `0x50`)
         * @param[in]   autoIncrement           Should the memory address pointer be automatically incremented upon
         *                                      reading and writing bytes from and to the EEPROM, similar to
         *                                      reading/writing files
         */
        Eeprom(const I2C &driver = pwI2c,
               const uint16_t initialMemoryAddress = DEFAULT_INITIAL_MEMORY_ADDRESS,
               const uint8_t deviceAddress = DEFAULT_DEVICE_ADDRESS,
               const bool autoIncrement = true)
            : m_driver(&driver),
              m_memoryAddress(initialMemoryAddress),
              m_deviceAddress(deviceAddress),
              m_autoIncrement(autoIncrement) { }

        virtual ~Eeprom() { }

        /**
         * @brief       Check that the EEPROM is responding
         *
         * @returns     True if the EEPROM is responding, false otherwise
         */
        bool ping() const {
            return this->m_driver->ping(this->m_deviceAddress);
        }

        /**
         * @brief       Place a byte of data into a specific address
         *
         * @param[in]   address     Address in EEPROM to place data
         * @param[in]   byte        Data to be written to EEPROM
         *
         * @returns     True if the data was successfully written, false otherwise
         */
        bool put(const uint16_t address, const uint8_t byte) const {
            // Wait for any current operation to finish
            while (!this->ping());

            return this->m_driver->put(this->m_deviceAddress, address, byte);
        }

        /**
         * @brief       Place multiple bytes of data into sequential memory locations in EEPROM
         *
         * @param[in]   startAddress    Address to store the first byte of data
         * @param[in]   bytes[]         Array of data - no null-terminator necessary
         * @param[in]   length          Number of bytes in the array that should be sent
         *
         * @returns     True if the data was successfully written, false otherwise
         */
        bool put(const uint16_t startAddress, const uint8_t bytes[], const size_t length) const {
            // Wait for any current operation to finish
            while (!this->ping());

            return this->m_driver->put(this->m_deviceAddress, startAddress, bytes, length);
        }

        /**
         * @see PropWare::PrintCapable::put_char
         *
         * @post    Internal memory address pointer will be incremented
         */
        virtual void put_char(const char c) {
            this->put(this->m_memoryAddress, (uint8_t) c);
            if (this->m_autoIncrement)
                ++this->m_memoryAddress;
        }

        /**
         * @see PropWare::PrintCapable::puts
         *
         * @post    Internal memory address pointer will be incremented by the length of the string
         */
        virtual void puts(const char *string) {
            const size_t stringLength = strlen(string);
            this->put(this->m_memoryAddress, (const uint8_t *) string, stringLength);
            if (this->m_autoIncrement)
                this->m_memoryAddress += stringLength;
        }

        /**
         * @brief       Read a byte from EEPROM
         *
         * @param[in]   address     Address in EEPROM to be read
         *
         * @returns     Data in EEPROM
         */
        uint8_t get(const uint16_t address) const {
            // Wait for any current operation to finish
            while (!this->ping());

            return this->m_driver->get(this->m_deviceAddress, address);
        }

        /**
         * @brief       Read an array of sequential bytes from EEPROM
         *
         * @param[in]   address     Address of the first byte in EEPROM
         * @param[in]   *buffer     Address in HUB memory to store data
         * @param[in]   length      Number of bytes to read
         *
         * @returns     True if successful, false otherwise
         */
        bool get(const uint16_t address, uint8_t *buffer, const size_t length) {
            // Wait for any current operation to finish
            while (!this->ping());

            return this->m_driver->get(this->m_deviceAddress, address, buffer, length);
        }

        /**
         * @see PropWare::PrintCapable::get_char
         *
         * @post    Internal memory address pointer will be incremented
         */
        virtual char get_char() {
            const uint8_t byte = this->get(this->m_memoryAddress);
            if (this->m_autoIncrement)
                ++this->m_memoryAddress;
            return byte;
        }

        /**
         * @brief       Retrieve the current address of the internal pointer
         *
         * @returns     Address in EEPROM used for reading or writing the next byte
         */
        uint16_t get_memory_address() const {
            return this->m_memoryAddress;
        }

        /**
         * @brief       Set the current address of the internal pointer
         *
         * @param[in]   address     Address in EEPROM used for reading or writing the next byte
         */
        void set_memory_address(const uint16_t address) {
            this->m_memoryAddress = address;
        }

        /**
         * @brief       Determine if auto incrementing of the internal address pointer is enabled
         *
         * @returns     True if enabled, false otherwise
         */
        bool is_auto_increment() const {
            return m_autoIncrement;
        }

        /**
         * @brief       Set auto incrementing of the internal address pointer to enabled or disabled
         *
         * @param[in]   autoIncrement   True to enabled, false to disable
         */
        void set_auto_increment(const bool autoIncrement) {
            m_autoIncrement = autoIncrement;
        }

    private:
        const I2C     *m_driver;
        uint16_t      m_memoryAddress;
        const uint8_t m_deviceAddress;
        bool          m_autoIncrement;
};

}
