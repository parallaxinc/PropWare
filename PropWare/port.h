/**
 * @file        port.h
 *
 * @project     PropWare
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

#ifndef PROPWARE_PORT_H
#define PROPWARE_PORT_H

#include <PropWare/PropWare.h>
#include <PropWare/pin.h>

namespace PropWare {

/**
 * @brief   Utility class to handle a conglomerate of pins all used together
 */
class Port {
    public:
        /**
         * @brief   Return the full pin mask of all pins in the port
         *
         * @return  A combination mask of all pins used in this port
         */
        uint32_t get_mask ();

        /**
         * @brief       Set selected pins as either input or output
         *
         * @param[in]   dir     I/O direction to set selected pins; must be one
         *                      of PropWare::Pin::IN or PropWare::Pin::OUT
         */
        void set_dir (const PropWare::Pin::Dir direction) const;

        /**
         * @brief   Toggle the output on all pins in the port
         */
        void toggle () const;

    protected:
        /**
         * @brief   No-arg constructor; Useful for using a Port as an instance
         *          in another class where you do not want to instantiate it
         *          in the constructor
         */
        Port ();

        /**
         * @brief       Allow easy writing to a port w/o destroying data
         *              elsewhere in the port; No shift is performed to align
         *              data with the desired output pins
         *
         * @param[in]   value   value to be bit-masked and then written to the
         *                      port
         */
        void writeFast (const uint32_t value) const;

        /**
         * @brief       Allow easy reading of only selected pins from a port
         *
         * @return      Value of INA masked by the port mask
         */
        uint32_t readFast () const;

    protected:
        uint32_t m_mask;
};

class SimplePort: public Port {
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
        SimplePort (const PropWare::Pin::Mask firstPin, uint8_t portWidth);

        /**
         * @brief       Initialize a port and configures its bit-mask and
         *              direction
         *
         * @param[in]   firstPin    Least significant pin in the data port mask
         * @param[in]   portWidth   The number of pins in the data port
         * @param[in[   direction   Determine input or output for the port
         */
        SimplePort (const PropWare::Pin::Mask firstPin, uint8_t portWidth,
                const PropWare::Pin::Dir direction);

        /**
         * @brief       Initialize a port and configures its bit-mask
         *
         * @param[in]   firstPin    Least significant pin in the data port;
         *                          0-indexed
         * @param[in]   portWidth   The number of pins in the data port
         */
        SimplePort (const uint8_t firstPin, uint8_t portWidth);

        /**
         * @brief       Initialize a port and configures its bit-mask and
         *              direction
         *
         * @param[in]   firstPin    Least significant pin in the data port;
         *                          0-indexed
         * @param[in]   portWidth   The number of pins in the data port
         * @param[in[   direction   Determine input or output for the port
         */
        SimplePort (const uint8_t firstPin, uint8_t portWidth,
                const PropWare::Pin::Dir direction);

        /**
         * @brief       Configure a port's bit-mask
         *
         * @param[in]   firstPin    Least significant pin in the data port mask
         * @param[in]   portWidth   The number of pins in the data port
         */
        void set_mask (const PropWare::Pin::Mask firstPin, uint8_t portWidth);

        /**
         * @brief       Configure a port's bit-mask
         *
         * @param[in]   firstPin    Least significant pin in the data port;
         *                          0-indexed
         * @param[in]   portWidth   The number of pins in the data port
         */
        void set_mask (const uint8_t firstPin, uint8_t portWidth);

        /**
         * @brief       Allow easy writing to a port w/o destroying data
         *              elsewhere in the port; A shift is performed before
         *              writing to align the LSB of the input parameter with the
         *              least significant pin in the data port
         *
         * @param[in]   value   value to be bit-masked and then written to the
         *                      port
         */
        void write (uint32_t value);

        /**
         * @brief   Allow easy reading of only selected pins from a port
         *
         * @pre     Port direction must be set to input
         *
         * @return  No manipulation of the return value is necessary (such as
         *          shifting to the right or masking)
         */
        uint32_t read ();

    protected:
        uint8_t m_firstPinNum;
};

class FlexPort: public Port {
    public:
        /**
         * @brief   @see PropWare::Port::Port()
         */
        FlexPort () :
                Port() {
        }

        /**
         * @brief       Initialize a port with a pre-configured port mask
         *
         * @param[in]   portMask    Each bit set high represents a Pin on the
         *                          port
         */
        FlexPort (const uint32_t portMask);

        /**
         * @brief       Initialize a port with a pre-configured port mask and
         *              direction
         *
         * @param[in]   portMask    Each bit set high represents a Pin on the
         *                          port
         * @param[in]   direction   One of PropWare::Pin::OUT or
         *                          PropWare::Pin::IN
         */
        FlexPort (const uint32_t portMask, const PropWare::Pin::Dir direction);

        /**
         * @brief       Initialize a port with an array of PropWare::Pin
         *              variables
         *
         * @param[in]   *pins       Array of PropWare::Pin variables; Last
         *                          element MUST BE PropWare::Pin::NULL_PIN or
         *                          segfault will occur
         */
        FlexPort (PropWare::Pin *pins);

        /**
         * @brief       Initialize a port with an array of PropWare::Pin
         *              variables
         *
         * @param[in]   *pins       Array of PropWare::Pin variables; Last
         *                          element MUST BE PropWare::Pin::NULL_PIN or
         *                          segfault will occur
         * @param[in]   direction   One of PropWare::Pin::OUT or
         *                          PropWare::Pin::IN
         */
        FlexPort (PropWare::Pin *pins, const PropWare::Pin::Dir direction);

        /**
         * @brief       Set the mask for this port
         *
         * @param[in]   mask    Pin mask
         */
        void set_mask (const uint32_t mask);

        /**
         * @brief       Add pins to the current mask
         *
         * @param[in]   mask    Additional pins to be ORed with current mask
         */
        void add_pins (const uint32_t mask);

        /**
         * @brief   @see PropWare::Port::writeFast()
         */
        void writeFast (const uint32_t value) const;

        /**
         * @brief   @see PropWare::Port::read()
         */
        uint32_t readFast () const;

        FlexPort& operator= (SimplePort &rhs);
};

}

#endif /* PROPWARE_PORT_H */
