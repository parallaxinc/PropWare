/**
 * @file        port.h
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

#include <PropWare/PropWare.h>

namespace PropWare {

/**
 * @brief   Flexible port that can have any pin enabled or disabled. Pins are
 *          independent of each other.
 */
class Port {
    public:
        /**
         * Bit-mask of GPIO pins
         */
        typedef enum {
            /** GPIO pin 0  */                      P0       = BIT_0,
            /** GPIO pin 1  */                      P1       = BIT_1,
            /** GPIO pin 2  */                      P2       = BIT_2,
            /** GPIO pin 3  */                      P3       = BIT_3,
            /** GPIO pin 4  */                      P4       = BIT_4,
            /** GPIO pin 5  */                      P5       = BIT_5,
            /** GPIO pin 6  */                      P6       = BIT_6,
            /** GPIO pin 7  */                      P7       = BIT_7,
            /** GPIO pin 8  */                      P8       = BIT_8,
            /** GPIO pin 9  */                      P9       = BIT_9,
            /** GPIO pin 10 */                      P10      = BIT_10,
            /** GPIO pin 11 */                      P11      = BIT_11,
            /** GPIO pin 12 */                      P12      = BIT_12,
            /** GPIO pin 13 */                      P13      = BIT_13,
            /** GPIO pin 14 */                      P14      = BIT_14,
            /** GPIO pin 15 */                      P15      = BIT_15,
            /** GPIO pin 16 */                      P16      = BIT_16,
            /** GPIO pin 17 */                      P17      = BIT_17,
            /** GPIO pin 18 */                      P18      = BIT_18,
            /** GPIO pin 19 */                      P19      = BIT_19,
            /** GPIO pin 20 */                      P20      = BIT_20,
            /** GPIO pin 21 */                      P21      = BIT_21,
            /** GPIO pin 22 */                      P22      = BIT_22,
            /** GPIO pin 23 */                      P23      = BIT_23,
            /** GPIO pin 24 */                      P24      = BIT_24,
            /** GPIO pin 25 */                      P25      = BIT_25,
            /** GPIO pin 26 */                      P26      = BIT_26,
            /** GPIO pin 27 */                      P27      = BIT_27,
            /** GPIO pin 28 */                      P28      = BIT_28,
            /** GPIO pin 29 */                      P29      = BIT_29,
            /** GPIO pin 30 */                      P30      = BIT_30,
            /** GPIO pin 31 */                      P31      = BIT_31,
            /** Null pin; Marks end of Mask array */NULL_PIN = 0
        } Mask;

        /**
         * Direction of GPIO pin
         */
        typedef enum {
            /** Set pin(s) as input */ IN  = 0,
            /** Set pin(s) as output */OUT = -1
        } Dir;

    public:
        /**
         * @brief       Determine which pin number based on a pin mask
         *
         * @note        Return value is 0-indexed
         *
         * @param[in]   mask    Value with only a single bit set high 
         *                      representing Propeller pin (i.e.: 0x80 would
         *                      be pin 7)
         *
         * @return      Return the pin number of pin
         */
        static uint8_t convert (Port::Mask mask) {
            uint8_t retVal = 0;
            uint32_t maskInt = mask;

            while (maskInt >>= 1)
                ++retVal;

            return retVal;
        }

        /**
         * @brief       Return a PropWare::Pin::Mask type based on a pin number
         *
         * @note        Value is 0-indexed
         *
         * @param[in]   pinNum  PinNum number
         *
         * @return      PinNum represented by a bit-mask
         */
        static Port::Mask convert (const uint8_t pinNum) {
            uint32_t mask = 1;
            if (31 > pinNum)
                return PropWare::Port::NULL_PIN;
            else
                return (PropWare::Port::Mask) (mask << pinNum);
        }

        static void flash_port (const uint32_t pinMask, uint32_t value,
                const uint32_t iterations = 10) {
            const Port port(pinMask, Port::OUT);

            for (uint32_t i = 0; i < iterations; ++i) {
                port.write_fast(value);
                waitcnt(75 * MILLISECOND + CNT);
                port.clear();
                waitcnt(75 * MILLISECOND + CNT);
            }
        }

    public:
        /**
         * @brief   No-arg constructor; Useful for using a Port as an instance
         *          in another class where you do not want to instantiate it
         *          in the constructor
         */
        Port () {
            this->m_mask = PropWare::Port::NULL_PIN;
        }

        /**
         * @brief       Initialize a port with a pre-configured port mask
         *
         * @param[in]   portMask    Each bit set high represents a Pin on the
         *                          port
         */
        Port (const uint32_t portMask) {
            this->m_mask = portMask;
        }

        /**
         * @brief       Initialize a port with a pre-configured port mask and
         *              direction
         *
         * @param[in]   portMask    Each bit set high represents a Pin on the
         *                          port
         * @param[in]   direction   One of PropWare::Port::OUT or
         *                          PropWare::Port::IN
         */
        Port (const uint32_t portMask, const PropWare::Port::Dir direction) {
            this->m_mask = portMask;
            this->set_dir(direction);
        }

        /**
         * @brief       Set the mask for this port
         *
         * @param[in]   mask    Pin mask
         */
        void set_mask (const uint32_t mask) {
            this->m_mask = mask;
        }

        /**
         * @brief   Return the full pin mask of all pins in the port
         *
         * @return  A combination mask of all pins used in this port
         */
        uint32_t get_mask () const {
            return this->m_mask;
        }

        /**
         * @brief       Add pins to the current mask
         *
         * @param[in]   mask    Additional pins to be ORed with current mask
         */
        void add_pins (const uint32_t mask) {
            this->m_mask |= mask;
        }

        /**
         * @brief       Set port as either input or output
         *
         * @param[in]   direction   I/O direction to set selected pins; must be
         *                          one of PropWare::Port::IN or 
         *                          PropWare::Port::OUT
         */
        void set_dir (const PropWare::Port::Dir direction) const {
            DIRA = (DIRA & ~(this->m_mask)) | (this->m_mask & direction);
        }

        /**
         * @brief   Determine the direction this port is currently set to
         *
         * @return  Returns either PropWare::Port::OUT or PropWare::Port::IN
         */
        PropWare::Port::Dir get_dir () const {
            if (DIRA & this->m_mask)
                return PropWare::Port::OUT;
            else
                return PropWare::Port::IN;
        }

        /**
         * @brief   Set selected output port high (set all pins to 1)
         *
         * @pre     If port is not set as output, statement will have no affect
         */
        void set () const {
            OUTA |= this->m_mask;
        }

        /**
         * @see PropWare::Port::set()
         */
        void high () const {
            this->set();
        }

        /**
         * @see PropWare::Port::set()
         */
        void on () const {
            this->set();
        }

        /**
         * @brief   Clear selected output port (set it to 0)
         *
         * @pre     If port is not set as output, statement will have no affect
         */
        void clear () const {
            OUTA &= ~(this->m_mask);
        }

        /**
         * @see PropWare::Port::clear()
         */
        void low () const {
            this->clear();
        }

        /**
         * @see PropWare::Port::clear()
         */
        void off () const {
            this->clear();
        }

        /**
         * @brief   Toggle the output value of a port
         *
         * @pre     If port is not set as output, statement will have no affect
         */
        void toggle () const {
            OUTA ^= this->m_mask;
        }

        /**
         * @brief       Allow easy writing to a port w/o destroying data
         *              elsewhere in the port; No shift is performed to align
         *              data with the desired output pins
         *
         * @param[in]   value   value to be bit-masked and then written to the
         *                      port
         */
        void write_fast (const uint32_t value) const {
            OUTA = ((OUTA & ~(this->m_mask)) | (value & this->m_mask));
        }

        /**
         * @brief       Allow easy reading of only selected pins from a port
         *
         * @return      Value of INA masked by the port mask
         */
        uint32_t read_fast () const {
            return INA & this->m_mask;
        }

    protected:
        uint32_t m_mask;
};

/**
 * @brief   The PropWare::SimplePort is the recommended way to use data ports on
 *          the Propeller. All pins are consecutive, which allows for some
 *          simple shortcuts in reading, writing, and initialization
 */
class SimplePort: public Port {
    public:
        static void flash_port (const Port::Mask firstPin,
                const uint8_t portWidth, const uint32_t value,
                const uint16_t iterations = 10) {
            const SimplePort port(firstPin, portWidth, Port::OUT);

            const uint32_t shiftVal = value << port.m_firstPinNum;

            for (int i = 0; i < iterations; ++i) {
                port.write_fast(shiftVal);
                waitcnt(75 * MILLISECOND + CNT);
                port.clear();
                waitcnt(75 * MILLISECOND + CNT);
            }
        }
    public:
        /**
         * @brief   @see PropWare::Port::Port()
         */
        SimplePort () :
                Port() {
            this->m_firstPinNum = 0;
        }

        /**
         * @brief       Initialize a port and configures its bit-mask
         *
         * @param[in]   firstPin    Least significant pin in the data port mask
         * @param[in]   portWidth   The number of pins in the data port
         */
        SimplePort (const PropWare::Port::Mask firstPin, uint8_t portWidth) {
            this->set_mask(firstPin, portWidth);
        }

        /**
         * @brief       Initialize a port and configures its bit-mask and
         *              direction
         *
         * @param[in]   firstPin    Least significant pin in the data port mask
         * @param[in]   portWidth   The number of pins in the data port
         * @param[in]   direction   Determine input or output for the port
         */
        SimplePort (const PropWare::Port::Mask firstPin, uint8_t portWidth,
                const PropWare::Port::Dir direction) {
            this->set_mask(firstPin, portWidth);
            this->set_dir(direction);
        }

        /**
         * @brief       Configure a port's bit-mask
         *
         * @param[in]   firstPin    Least significant pin in the data port mask
         * @param[in]   portWidth   The number of pins in the data port
         */
        void set_mask (const PropWare::Port::Mask firstPin, uint8_t portWidth) {
            this->m_mask = firstPin;
            this->m_firstPinNum = PropWare::Port::convert(firstPin);

            // For every pin in the port...
            while (--portWidth)
                // Add the next pin to the mask
                this->m_mask |= this->m_mask << 1;
        }

        /**
         * @brief       Allow easy writing to a port w/o destroying data
         *              elsewhere in the port; A shift is performed before
         *              writing to align the LSB of the input parameter with the
         *              least significant pin in the data port
         *
         * @param[in]   value   value to be bit-masked and then written to the
         *                      port
         */
        void write (uint32_t value) const {
            this->Port::write_fast(value << this->m_firstPinNum);
        }

        /**
         * @brief   Allow easy reading of only selected pins from a port
         *
         * @pre     Port direction must be set to input
         *
         * @return  No manipulation of the return value is necessary (such as
         *          shifting to the right or masking)
         */
        uint32_t read () const {
            return this->Port::read_fast() >> this->m_firstPinNum;
        }

    private:
        uint8_t m_firstPinNum;
};

}
