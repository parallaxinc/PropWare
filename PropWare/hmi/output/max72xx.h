/**
 * @file        PropWare/hmi/output/max72xx.h
 *
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

#include <PropWare/serial/spi/spi.h>

namespace PropWare {

/**
 * @brief   Serially interfaced, 8-Digit LED display driver
 *
 * Example of enabling the device and counting from -200 to 200 with a 20 millisecond delay between each number
 *
 * @code
 * #include <PropWare/hmi/output/max72xx.h>
 *
 * using PropWare::SPI;
 * using PropWare::Pin;
 * using PropWare::MAX72xx;
 *
 * int main () {
 *     SPI::get_instance().set_mosi(Pin::P2);
 *     SPI::get_instance().set_sclk(Pin::P0);
 *     const MAX72xx leds(Port::P4);
 *
 *     leds.start();
 *     for (int i = -200; i <= 200; ++i) {
 *         leds.put_int(i, MAX72xx::BcdCharacter::ZERO, 4);
 *         waitcnt(20 * MILLISECOND + CNT);
 *     }
 *
 *     return 0;
 * }
 * @endcode
 */
class MAX72xx {
    public:
        /**
         * @brief   Register addresses
         */
        // Addresses start at 0x10 to avoid conflicts with MAX695x devices
        enum class Register {
                /** No-op register for chaining multiple devices together */                    NO_OP,
                /** Lowest order digit */                                                       DIGIT_ZERO   = 0x11,
                /** Lowest order digit + 1 */                                                   DIGIT_ONE    = 0x12,
                /** Lowest order digit + 2 */                                                   DIGIT_TWO    = 0x13,
                /** Lowest order digit + 3 */                                                   DIGIT_THREE  = 0x14,
                /** Lowest order digit + 4 */                                                   DIGIT_FOUR   = 0x15,
                /** Lowest order digit + 5 */                                                   DIGIT_FIVE   = 0x16,
                /** Lowest order digit + 6 */                                                   DIGIT_SIX    = 0x17,
                /** Highest order digit */                                                      DIGIT_SEVEN  = 0x18,
                /** Determines whether values should be decoded before writing to the device */ DECODE_MODE  = 0x19,
                /** Sets the display intensity (brightness) */                                  INTENSITY    = 0x1A,
                /** Enables specific digits on the device */                                    SCAN_LIMIT   = 0x1B,
                /** Determines the on/off status of the device */                               SHUTDOWN     = 0x1C,
                /** Run a test of all segments on the device */                                 DISPLAY_TEST = 0x1D
        };

        /**
         * @brief Characters that will utilize the device's built-in font
         *
         * When the device is configured to use decode mode, these characters can be written to the device and they
         * will be automatically converted to BCD for appropriate display.
         */
        enum class BcdCharacter {
                ZERO,
                ONE,
                TWO,
                THREE,
                FOUR,
                FIVE,
                SIX,
                SEVEN,
                EIGHT,
                NINE,
                DASH,
                E,
                H,
                L,
                P,
                BLANK
        };

        /**
         * The MAX72xx family uses SPI's mode 1 - that is, a low clock polarity and rising-edge phase.
         */
        static const SPI::Mode SPI_MODE                = SPI::Mode::MODE_1;
        static const uint8_t   DEFAULT_INTENSITY       = 15;
        static const uint8_t   DEFAULT_SCAN_LIMIT      = 7;
        static const uint8_t   DEFAULT_BCD_DECODE_BITS = 0xFF;
        static const uint8_t   DECIMAL_POINT           = BIT_7;

    public:
        /**
         * @brief       Get the Register address of the requested 0-indexed digit on the device
         *
         * @param[in]   zeroIndexedDigit     Low values are to the right of the device and high values to the left
         *
         * @return      Register enumeration value for the requested digit
         */
        static Register to_register (unsigned int zeroIndexedDigit) {
            return static_cast<Register>(static_cast<unsigned int>(Register::DIGIT_ZERO) + zeroIndexedDigit);
        }

    public:
        /**
         * @brief                           Create an object which communicates over the given SPI bus
         *
         * @param[in]   bus                 SPI bus for communication
         * @param[in]   csMask              Chip-select's pin mask
         * @param[in]   alwaysSetSPIMode    When true, the SPI bus's mode will always be set prior to communication
         */
        MAX72xx (SPI &bus, const Pin::Mask csMask, const bool alwaysSetSPIMode = false)
            : m_bus(&bus),
              m_cs(csMask, Pin::Dir::OUT),
              m_alwaysSetMode(alwaysSetSPIMode) {
            if (!alwaysSetSPIMode)
                this->m_bus->set_mode(SPI_MODE);
            this->m_cs.set();
        }

        /**
         * @brief                           Create an object which communicates over the shared SPI bus
         *
         * @param[in]   csMask              Chip-select's pin mask
         * @param[in]   alwaysSetSPIMode    When true, the SPI bus's mode will always be set prior to communication
         */
        MAX72xx (const Pin::Mask csMask, const bool alwaysSetSPIMode = true)
            : m_bus(&SPI::get_instance()),
              m_cs(csMask, Pin::Dir::OUT),
              m_alwaysSetMode(alwaysSetSPIMode) {
            if (!alwaysSetSPIMode)
                this->m_bus->set_mode(SPI::Mode::MODE_1);
            this->m_cs.set();
        }

        /**
         * @brief       Choose whether to always set the SPI mode before writing to the device; Useful when multiple
         *              devices with different SPI specifications are connected to the bus
         *
         * @param[in]   alwaysSetMode   For any non-zero value, the SPI modes will always be set before a write routine
         */
        void always_set_spi_mode (const bool alwaysSetMode) {
            this->m_alwaysSetMode = alwaysSetMode;
        }

        /**
         * @brief       Initialize the device after power-on
         *
         * @param[in]   intensity       Brightness value between 0 and 15, with 15 being maximum brightness
         * @param[in]   scanLimit       0 will enable only the least significant digit and 7 will enable all digits.
         *                              Value must be between 0 and 7.
         * @param[in]   bcdDecodeBits   Each bit represents one of the digits. A digit with its decode bit set high will
         *                              be decoded by the built-in font, and a digit with its decode bit cleared low
         *                              will have its register output directly to the LED segments.
         */
        void start (const uint8_t intensity = DEFAULT_INTENSITY, const uint8_t scanLimit = DEFAULT_SCAN_LIMIT,
                    const uint8_t bcdDecodeBits = DEFAULT_BCD_DECODE_BITS) const {
            this->set_intensity(intensity);
            this->set_scan_limit(scanLimit);
            this->set_decode_mode(bcdDecodeBits);
            this->set_test_mode(false);
            this->clear();
            this->write(Register::SHUTDOWN, 1);
        }

        /**
         * @brief   Disable the device by writing to the SHUTDOWN register. Values in other registers will not be lost.
         */
        void shutdown () const {
            this->write(Register::SHUTDOWN, 0);
        }

        /**
         * @brief       Set the device's brightness
         *
         * @param[in]   intensity         Brightness value between 0 and 15, with 15 being maximum brightness
         */
        void set_intensity (const uint8_t intensity) const {
            if (16 > intensity)
                this->write(Register::INTENSITY, intensity);
        }

        /**
         * @brief       Enables or disables specific digits on the device
         *
         * @param[in]   scanLimit   0 will enable only the least significant digit and 7 will enable all digits. Value
         *                          must be between 0 and 7.
         */
        void set_scan_limit (const uint8_t scanLimit) const {
            if (8 > scanLimit)
                this->write(Register::SCAN_LIMIT, scanLimit);
        }

        /**
         * @brief       Determines which digits will be decoded and which ones will be converted to the build-in font
         *              prior to output
         *
         * @param[in]   bcdDecodeBits   Each bit represents one of the digits. A digit with its decode bit set high will
         *                              be decoded by the built-in font, and a digit with its decode bit cleared low
         *                              will have its register output directly to the LED segments.
         */
        void set_decode_mode (const uint8_t bcdDecodeBits) const {
            this->write(Register::DECODE_MODE, bcdDecodeBits);
        }

        /**
         * @brief       Enable or disable the test mode of the device. When enable, all LED segments will be lit.
         *              Test mode will override the shutdown register.
         *
         * @param[in]   enableTestMode  True to enable, false to disable.
         */
        void set_test_mode (const bool enableTestMode) const {
            this->write(Register::DISPLAY_TEST, static_cast<const uint8_t>(enableTestMode));
        }

        /**
         * @brief       Write an integer to the device
         *
         * @param[in]   x           Value to be written. If the value is larger than 99,999,999 then it will be
         *                          truncated and only it's lowest 8 digits will be displayed. Numbers smaller than
         *                          -9,999,999 will be truncated (the sign will be truncated before the digits).
         * @param[in]   fillChar    When the number of digits in the value is smaller than the width, this character
         *                          will be used to fit empty digits
         * @param[in]   width       Minimum number of digits used to display the value. For negative numbers, the
         *                          sign will count as one digit in the width.
         */
        void put_int (int32_t x, const BcdCharacter fillChar = BcdCharacter::BLANK, unsigned int width = 0) const {
            if (width > 8)
                width = 8;

            const bool isNegative = 0 > x;
            if (isNegative) {
                x = abs(x);
            }

            uint_fast8_t i = 0;
            do {
                const uint8_t digit = static_cast<const uint8_t>(x % 10);
                if (x)
                    this->write(to_register(i), digit);
                x /= 10;
                ++i;
            } while (x && 8 > i);

            if (isNegative) {
                if (width)
                    --width;
                while (width > i) {
                    this->write(to_register(i++), static_cast<const uint8_t>(fillChar));
                }
                if (8 > i)
                    this->write(to_register(i), static_cast<const uint8_t>(BcdCharacter::DASH));
            } else {
                while (width > i) {
                    this->write(to_register(i), static_cast<const uint8_t>(fillChar));
                    ++i;
                }
            }
        }

        /**
         * @brief   Write BcdCharacter::BLANK to all eight digits
         */
        void clear () const {
            for (uint8_t i = 0; i < 8; ++i)
                this->write(to_register(i), static_cast<const uint8_t>(BcdCharacter::BLANK));
        }

        /**
         * @brief       Perform a manual write to the device
         *
         * @param[in]   address     Address of the register that should be modified
         * @param[in]   value       Value that should be written to the register
         * @param[in]   decimal     When writing to a digit register, the decimal LED will be enabled if and only if
         *                          this parameter is set to `true`.
         */
        void write (const Register address, const uint8_t value, const bool decimal = false) const {
            if (this->m_alwaysSetMode)
                this->m_bus->set_mode(SPI_MODE);

            this->m_cs.clear();
            this->m_bus->shift_out(8, static_cast<uint32_t>(address));
            if (decimal)
                this->m_bus->shift_out(8, value | DECIMAL_POINT);
            else
                this->m_bus->shift_out(8, value);
            this->m_cs.set();
        }

    private:
        SPI       *m_bus;
        const Pin m_cs;
        bool      m_alwaysSetMode;
};

}
