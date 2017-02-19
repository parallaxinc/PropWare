/**
 * @file        PropWare/gpio/simpleport.h
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

#include <PropWare/gpio/port.h>

namespace PropWare {

/**
 * @brief   The PropWare::SimplePort is the recommended way to use data ports on the Propeller. All pins are
 *          consecutive, which allows for some simple shortcuts in reading, writing, and initialization
 */
class SimplePort : public Port {
    public:
        /**
         * @brief       Great for quick debugging to ensure a line of code is executed, this will quickly flash a given
         *              set of pins a specific number of times
         *
         * @param[in]   firstPin    First pin in sequence
         * @param[in]   portWidth   Number of pins that should be flashed
         * @param[in]   iterations  Number of times that the pins should flicker on and back off again
         */
        static void flash_port (const Port::Mask firstPin, const uint8_t portWidth, const uint16_t iterations = 10) {
            const SimplePort port(firstPin, portWidth);
            Port::flash_port(port.m_mask, iterations);
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
         * @brief       Initialize a port and configures its bit-mask and direction
         *
         * @param[in]   firstPin    Least significant pin in the data port mask
         * @param[in]   portWidth   The number of pins in the data port
         * @param[in]   direction   Determine input or output for the port
         */
        SimplePort (const PropWare::Port::Mask firstPin, uint8_t portWidth, const PropWare::Port::Dir direction) {
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
            this->m_mask        = static_cast<uint32_t>(firstPin);
            this->m_firstPinNum = PropWare::Port::from_mask(firstPin);

            // For every pin in the port...
            while (--portWidth)
                // Add the next pin to the mask
                this->m_mask |= this->m_mask << 1;
        }

        /**
         * @brief       Allow easy writing to a port w/o destroying data elsewhere in the port; A shift is performed
         *              before writing to align the LSB of the input parameter with the least significant pin in the
         *              data port
         *
         * @param[in]   value   value to be bit-masked and then written to the port
         */
        void write (uint32_t value) const {
            this->Port::write_fast(value << this->m_firstPinNum);
        }

        /**
         * @brief   Allow easy reading of only selected pins from a port
         *
         * @pre     Port direction must be set to input
         *
         * @return  No manipulation of the return value is necessary (such as shifting to the right or masking)
         */
        uint32_t read () const {
            return this->Port::read_fast() >> this->m_firstPinNum;
        }

    private:
        uint8_t m_firstPinNum;
};

}
