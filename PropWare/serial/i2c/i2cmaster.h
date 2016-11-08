/**
 * @file        PropWare/serial/i2c/i2cmaster.h
 *
 * @author      SRLM
 * @author      David Zemon
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

#include <PropWare/gpio/pin.h>

namespace PropWare {

/**
 * @brief   Basic I2C driver
 *
 * Requires that the SDA and SCL pins have sufficient pullups. These should be selected based on the capacitance of the
 * devices on the I2C bus, and the expected clock speed (400kHz currently).
 *
 * All "device" fields should be the 7 bit address of the device, with the low bit set to 0 (the 7 addres bits are the
 * upper bits). This applies to both the Put (write) and Get (read) cases.
 *
 * Put and Get are based on I2C communication specification as described by ST in the LSM303DLHC and L3GD20 datasheets.
 * Terms:
 *
 *    + ST - Start
 *    + SAD - Slave Address (device)
 *    + SAK - Slave Acknowledge
 *    + SUB - SubAddress (slave register address)
 *    + SP - Stop
 *    + +W - plus write (lowest device bit set to 0)
 *    + +R - plus read (lowest device bit set to 1)
 *    + NMAK - Master No Acknowledge
 *
 * I2C differs based on the device that you use. For functions that might be device specific, there is a reference
 * indicator. These references are:
 *
 *    + ST - ST Microelectronics, particularly the LSM303DLHC and L3GD20 devices.
 *    + MS - Measurement Specialties, particularly the MS5607 and MS5611 devices.
 *
 * If you're using the multibyte Get and Put with ST based devices, be sure to bitwise OR the register address with 0x80
 * (the MSb to 1) in order to turn on the auto-increment function (see datasheet for L3GD20 for example). This is not
 * done automatically by this library.
 */
class I2CMaster {
    public:
        static const Pin::Mask    DEFAULT_SCL_MASK  = Pin::Mask::P28;
        static const Pin::Mask    DEFAULT_SDA_MASK  = Pin::Mask::P29;
        static const unsigned int DEFAULT_FREQUENCY = 400000;

    public:
        /**
         * @brief       Create a basic I2C instance
         *
         * @param[in]   sclMask     Pin mask for the SCL pin (default value uses the EEPROM SCL line)
         * @param[in]   sdaMask     Pin mask for the SDA pin (default value uses the EEPROM SDA line)
         * @param[in]   frequency   Frequency to run the bus (default is highest standard I2C frequency of 400 kHz)
         */
        I2CMaster (const Pin::Mask sclMask = DEFAULT_SCL_MASK, const Pin::Mask sdaMask = DEFAULT_SDA_MASK,
             const unsigned int frequency = DEFAULT_FREQUENCY)
                : m_scl(sclMask, Pin::Dir::IN),
                  m_sda(sdaMask, Pin::Dir::IN) {
            this->set_frequency(frequency);

            //Set outputs low
            this->m_scl.clear();
            this->m_sda.clear();
        }

        /**
         * @brief       Set the bus frequency
         *
         * @param[in]   frequency   Frequency in Hz to run the bus
         */
        void set_frequency (const unsigned int frequency) {
            this->m_clockDelay = CLKFREQ / (frequency << 1);
        }

        /**
         * @brief   Output a start condition on the I2C bus
         */
        void start () const {
			this->m_scl.set();
			this->m_sda.set();

			this->m_scl.set_dir_out();
			this->m_sda.set_dir_out();

			asm volatile("nop; nop; nop; nop; nop; nop;");
			this->m_sda.clear();
			asm volatile("nop; nop; nop; nop; nop; nop;");
			this->m_scl.clear();
		}

        /**
         * @brief   Output a stop condition on the I2C bus
         */
		void stop () const {
			this->m_sda.clear();
			this->m_scl.clear();

			asm volatile("nop; nop; nop; nop; nop; nop;");
			this->m_scl.set_dir_in();
			asm volatile("nop; nop; nop; nop; nop; nop;");
			this->m_sda.set_dir_in();
		}

        /**
         * @brief   Output a byte on the I2C bus.
         *
         * @param[in]   byte    8 bits to send on the bus
         *
         * @return      true if the device acknowledges, false otherwise
         */
        bool send_byte (const uint8_t byte) const {
            int result   = 0;
            int datamask = 0;
            int nextCNT  = 0;
            int temp     = 0;

            __asm__ volatile(
            FC_START("PutByteStart", "PutByteEnd")
                    /* Setup for transmit loop */
                    "         mov %[datamask], #256          \n\t" /* 0x100 */
                    "         mov %[result],   #0            \n\t"
                    "         mov %[nextCNT],  cnt           \n\t"
                    "         add %[nextCNT],  %[clockDelay] \n\t"

                    /* Transmit Loop (8x) */
                    //Output bit of byte
                    "PutByteLoop%=: "
                    "         shr  %[datamask], #1                \n\t" // Set up mask
                    "         and  %[datamask], %[databyte] wz,nr \n\t" // Move the bit into Z flag
                    "         muxz dira,        %[SDAMask]        \n\t"

                    //Pulse clock
                    "         waitcnt %[nextCNT], %[clockDelay] \n\t"
                    "         andn    dira,       %[SCLMask]    \n\t" // Set SCL high
                    "         waitcnt %[nextCNT], %[clockDelay] \n\t"
                    "         or      dira,       %[SCLMask]    \n\t" // Set SCL low

                    //Return for more bits
                    "         djnz %[datamask], #" FC_ADDR("PutByteLoop%=", "PutByteStart") " nr \n\t"

                    // Get ACK
                    "         andn    dira,       %[SDAMask]    \n\t" // Float SDA high (release SDA)
                    "         waitcnt %[nextCNT], %[clockDelay] \n\t"
                    "         andn    dira,       %[SCLMask]    \n\t" // SCL high (by float)
                    "         waitcnt %[nextCNT], %[clockDelay] \n\t"
                    "         mov     %[temp],    ina           \n\t" //Sample input
                    "         and     %[SDAMask], %[temp] wz,nr \n\t" // If != 0, ack'd, else nack
                    "         muxz    %[result],  #1            \n\t" // Set result to equal to Z flag (aka, 1 if ack'd)
                    "         or      dira,       %[SCLMask]    \n\t" // Set scl low
                    "         or      dira,       %[SDAMask]    \n\t" // Set sda low

                    FC_END("PutByteEnd")
            : // Outputs
            [datamask] "=&r"(datamask),
            [result] "=&r"(result),
            [nextCNT] "=&r"(nextCNT),
            [temp] "=&r"(temp)
            : // Inputs
            [SDAMask] "r"(this->m_sda.get_mask()),
            [SCLMask] "r"(this->m_scl.get_mask()),
            [databyte] "r"(byte),
            [clockDelay] "r"(m_clockDelay));

            return (bool) result;
        }

        /**
         * @brief       Get a byte from the bus
         *
         * @param[in]   acknowledge     true to acknowledge the byte received, false otherwise
         *
         * @return      Byte clocked in from the bus
         */
        uint8_t read_byte (const bool acknowledge) const {
            int result = 0;
            int datamask, nextCNT, temp;

            __asm__ volatile(
            FC_START("GetByteStart", "GetByteEnd")
                    // Setup for receive loop
                    "         andn dira,        %[SDAMask]    \n\t"
                    "         mov  %[datamask], #256          \n\t" /* 0x100 */
                    "         mov  %[result],   #0            \n\t"
                    "         mov  %[nextCNT],  cnt           \n\t"
                    "         add  %[nextCNT],  %[clockDelay] \n\t"

                    // Recieve Loop (8x)
                    //Get bit of byte
                    "GetByteLoop%=: "

                    "         waitcnt %[nextCNT],  %[clockDelay] \n\t"
                    "         shr     %[datamask], #1            \n\t" // Set up mask

                    //Pulse clock
                    "         andn    dira,       %[SCLMask]       \n\t" // Set SCL high
                    "         waitcnt %[nextCNT], %[clockDelay]    \n\t"
                    "         mov     %[temp],    ina              \n\t" //Sample the input
                    "         and     %[temp],    %[SDAMask] nr,wz \n\t"
                    "         muxnz   %[result],  %[datamask]      \n\t"
                    "         or      dira,       %[SCLMask]       \n\t" // Set SCL low

                    //Return for more bits
                    "         djnz %[datamask], #" FC_ADDR("GetByteLoop%=", "GetByteStart") " nr \n\t"

                    // Put ACK

                    "         and     %[acknowledge], #1 nr,wz  \n\t" //Output ACK

                    "         muxnz   dira,       %[SDAMask]    \n\t"
                    "         waitcnt %[nextCNT], %[clockDelay] \n\t"
                    "         andn    dira,       %[SCLMask]    \n\t" // SCL high (by float)
                    "         waitcnt %[nextCNT], %[clockDelay] \n\t"

                    "         or   dira, %[SCLMask]       \n\t" // Set scl low
                    "         or   dira, %[SDAMask]       \n\t" // Set sda low
                    FC_END("GetByteEnd")
            : // Outputs
            [datamask] "=&r"(datamask),
            [result] "=&r"(result),
            [temp] "=&r"(temp),
            [nextCNT] "=&r"(nextCNT)

            : // Inputs
            [SDAMask] "r"(this->m_sda.get_mask()),
            [SCLMask] "r"(this->m_scl.get_mask()),
            [acknowledge] "r"(acknowledge),
            [clockDelay] "r"(m_clockDelay));

            return (uint8_t) result;

        }

        /**
         * @brief   Test for the Acknowledge of a device by sending start and the slave address.
         *
         * Useful for polling the bus and seeing what devices are available. Ping uses the following format:
         *
         *  +--------+----+-------+-----+----+
         *  | Master | ST | SAD+W |     | SP |
         *  | Slave  |    |       | SAK |    |
         *  +--------+----+-------+-----+----+
         *
         * @param[in]   device  7-bit shifted address device (in bits 7-1, not 6-0)
         *
         * @return      true if ack was received, false otherwise
         */
        bool ping (const uint8_t device) const {
            this->start();
            bool result = this->send_byte(device);
            this->stop();
            return result;
        }

        /**
         * @brief       Put a single byte with the following format:
         *
         *  +--------+----+-------+-----+-----+-----+------+-----+----+
         *  | Master | ST | SAD+W |     | SUB |     | BYTE |     | SP |
         *  | Slave  |    |       | SAK |     | SAK |      | SAK |    |
         *  +--------+----+-------+-----+-----+-----+------+-----+----+
         *
         * Reference: ST
         *
         * @param[in]   device      7 bit slave I2C address (in bits 7-1, with bit 0 set to 0)
         * @param[in]   address     slave register address
         * @param[in]   byte        8 bits of data to store at `address`
         *
         * @return      false if one or more nAcks is received, true otherwise
         */
        template<typename T>
        bool put (const uint8_t device, const T address, const uint8_t byte) const {
            bool result;

            this->start();
            result = this->send_byte(device);
            result &= this->send_address(address);
            result &= this->send_byte(byte);
            this->stop();

            return result;
        }

        /**
         * @brief       Get a single byte with the following format:
         *
         *  +--------+----+-------+-----+-----+-----+----+-------+-----+------+------+----+
         *  | Master | ST | SAD+W |     | SUB |     | ST | SAD+R |     |      | NMAK | SP |
         *  | Slave  |    |       | SAK |     | SAK |    |       | SAK | DATA |      |    |
         *  +--------+----+-------+-----+-----+-----+----+-------+-----+------+------+----+
         *
         * Reference: ST
         *
         * @param[in]   device      7 bit slave I2C address (in bits 7-1, with bit 0 set to 0)
         * @param[in]   address     slave register address
         *
         * @return      the 8 bits of data read from register @a address.
         */
        template<typename T>
        uint8_t get (const uint8_t device, const T address) const {
            bool result;

            this->start();
            result = this->send_byte(device);
            result &= this->send_address(address);

            this->start();
            result &= this->send_byte((uint8_t) (device | 0x01)); //Set read bit
            uint8_t dataByte = this->read_byte(false);
            this->stop();
            return dataByte;
        }

        /**
         * @brief       Put multiple bytes with the following format:
         *
         *                                          |Repeat for # of bytes    |
         *  +--------+----+-------+-----+-----+-----+------+-----+------+-----+----+
         *  | Master | ST | SAD+W |     | SUB |     | DATA |     | DATA |     | SP |
         *  | Slave  |    |       | SAK |     | SAK |      | SAK |      | SAK |    |
         *  +--------+----+-------+-----+-----+-----+------+-----+------+-----+----+
         *
         * Reference: ST
         *
         * @param[in]   device      7 bit slave I2C address (in bits 7-1, with bit 0 set to 0)
         * @param[in]   address     slave register address
         * @param[in]   bytes       set of bytes to store on @a device, starting at register @a address.
         * @param[in]   size        number of bytes to write
         *
         * @return      false if one or more nAcks is received, true otherwise
         */
        template<typename T>
        bool put (const uint8_t device, const T address, const uint8_t bytes[], const size_t size) const {
            bool result;
            this->start();
            result = this->send_byte(device);
            result &= this->send_address(address);

            for (unsigned int i = 0; i < size; ++i)
                result &= this->send_byte(bytes[i]);

            this->stop();

            return result;
        }

        /**
         * @brief   Read multiple bytes
         *
         * Get multiple bytes with the following format:
         *
         *                                                             |Repeat for # of bytes -1 | Last byte   |
         *  +--------+----+-------+-----+-----+-----+----+-------+-----+------+-----+------+-----+------+------+----+
         *  | Master | ST | SAD+W |     | SUB |     | ST | SAD+R |     |      | MAK |      | MAK |      | NMAK | SP |
         *  | Slave  |    |       | SAK |     | SAK |    |       | SAK | DATA |     | DATA |     | DATA |      |    |
         *  +--------+----+-------+-----+-----+-----+----+-------+-----+------+-----+------+-----+------+------+----+
         *
         * Reference: ST
         *
         * @param[in]   device      the 7 bit slave I2C address (in bits 7-1, with bit 0 set to 0).
         * @param[in]   address     the 8 bit slave register address.
         * @param[out]  bytes       the address to begin storing the read bytes at.
         * @param[in]   size        the number of bytes to read.
         *
         * @return      false if one or more nAcks is received, true otherwise
         */
        template<typename T>
        bool get (const uint8_t device, const T address, uint8_t bytes[], const size_t size) const {
            bool result;
            this->start();
            result = this->send_byte(device);
            result &= this->send_address(address);
            this->start();
            result &= this->send_byte((uint8_t) (device | 0x01));

            unsigned int i = 0;
            for (; i < size - 1; ++i)
                bytes[i] = this->read_byte(true); //Send true to keep on reading bytes
            bytes[i]     = this->read_byte(false); //Trailing NAK

            this->stop();
            return result;
        }

        /**
         * @brief   Put a single byte, no register address, on the bus
         *
         * Follows the format:
         *
         *  +--------+----+-------+-----+------+-----+----+
         *  | Master | ST | SAD+W |     | DATA |     | SP |
         *  | Slave  |    |       | SAK |      | SAK |    |
         *  +--------+----+-------+-----+------+-----+----+
         *
         * Reference: MS
         *
         * @warning         Notice the lack of a specified register!
         *
         * @param[in]   device  the 7 bit slave I2C address (in bits 7-1, with bit 0 set to 0)
         * @param[in]   byte    the 8 bits of data to send to @a device
         *
         * @return      false if one or more nAcks is received, true otherwise
         */
        bool put (const uint8_t device, const uint8_t byte) const {
            //Warning: this method is not unit tested! (it's run, but the MS5611 does
            //not have a register that can be written to and read from).

            this->start();
            bool result = this->send_byte(device);
            result &= this->send_byte(byte);
            this->stop();

            return result;
        }

        /**
         * @brief   Get multiple bytes, no register address
         *
         * Reference: MS
         * Follows the format:
         *
         *                              |Repeat      |
         *  +--------+----+-------+-----+------+-----+------+------+----+
         *  | Master | ST | SAD+R |     |      | MAK |      | NMAK | SP |
         *  | Slave  |    |       | SAK | DATA |     | DATA |      |    |
         *  +--------+----+-------+-----+------+-----+------+------+----+
         *
         * @warning Notice the lack of a specified register!
         *
         * @param[in]   device  the 7 bit slave I2C address (in bits 7-1, with bit 0 set to 0)
         * @param[out]  bytes   the address to begin storing the read bytes at.
         * @param[in]   size    the number of bytes to read.
         *
         * @return      false if one or more nAcks is received, true otherwise
         */
        bool get (const uint8_t device, uint8_t bytes[], const size_t size) const {
            this->start();
            bool result = this->send_byte((uint8_t) (device | 0x01));

            unsigned int i;
            for (i = 0; i < size - 1; ++i)
                bytes[i] = this->read_byte(true);
            bytes[i]     = this->read_byte(false);

            this->stop();
            return result;
        }

    private:
        bool send_address (const uint8_t address) const {
            return this->send_byte(address);
        }

        bool send_address (const uint16_t address) const {
            bool result = this->send_byte((const uint8_t) (address >> 8));
            return result && this->send_byte((const uint8_t) address);
        }

    private:
        const Pin    m_scl;
        const Pin    m_sda;
        unsigned int m_clockDelay;
};

}

/**
 * @brief   Global I2C instance for easy and shared use by Propeller applications (not thread safe!)
 */
extern PropWare::I2CMaster pwI2c;
