/**
 * @file        pin.h
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

#ifndef PROPWARE_PIN_H_
#define PROPWARE_PIN_H_

#include <PropWare/PropWare.h>

namespace PropWare {

/**
 * @brief   Utility class to handle general purpose I/O pins
 */
class Pin {
    public:
        /**
         * Bit-mask of GPIO pins
         */
        typedef enum {
            /** GPIO pin 0  */P0 = BIT_0,
            /** GPIO pin 1  */P1 = BIT_1,
            /** GPIO pin 2  */P2 = BIT_2,
            /** GPIO pin 3  */P3 = BIT_3,
            /** GPIO pin 4  */P4 = BIT_4,
            /** GPIO pin 5  */P5 = BIT_5,
            /** GPIO pin 6  */P6 = BIT_6,
            /** GPIO pin 7  */P7 = BIT_7,
            /** GPIO pin 8  */P8 = BIT_8,
            /** GPIO pin 9  */P9 = BIT_9,
            /** GPIO pin 10 */P10 = BIT_10,
            /** GPIO pin 11 */P11 = BIT_11,
            /** GPIO pin 12 */P12 = BIT_12,
            /** GPIO pin 13 */P13 = BIT_13,
            /** GPIO pin 14 */P14 = BIT_14,
            /** GPIO pin 15 */P15 = BIT_15,
            /** GPIO pin 16 */P16 = BIT_16,
            /** GPIO pin 17 */P17 = BIT_17,
            /** GPIO pin 18 */P18 = BIT_18,
            /** GPIO pin 19 */P19 = BIT_19,
            /** GPIO pin 20 */P20 = BIT_20,
            /** GPIO pin 21 */P21 = BIT_21,
            /** GPIO pin 22 */P22 = BIT_22,
            /** GPIO pin 23 */P23 = BIT_23,
            /** GPIO pin 24 */P24 = BIT_24,
            /** GPIO pin 25 */P25 = BIT_25,
            /** GPIO pin 26 */P26 = BIT_26,
            /** GPIO pin 27 */P27 = BIT_27,
            /** GPIO pin 28 */P28 = BIT_28,
            /** GPIO pin 29 */P29 = BIT_29,
            /** GPIO pin 30 */P30 = BIT_30,
            /** GPIO pin 31 */P31 = BIT_31,
            /** Null pin; Marks end of Mask array */NULL_PIN = 0
        } Mask;

        /**
         * Direction of GPIO pin
         */
        typedef enum {
            /** Set pin(s) as input */IN = 0,
            /** Set pin(s) as output */OUT = -1
        } Dir;

    public:
        /** Number of milliseconds to delay during debounce */
        static const uint8_t DEBOUNCE_DELAY = 3;

    public:
        /**
         * @brief       Determine which pin number based on a pin mask
         *
         * @note        Return value is 0-indexed
         *
         * @param[in]   pinMask     Value with only a single bit set high
         *                          representing Propeller pin (i.e.: 0x80 would
         *                          be pin 7)
         *
         * @return      Return the pin number of pin
         */
        static uint8_t convert (Pin::Mask mask);

        /**
         * @brief       Return a PropWare::Pin::Mask type based on a pin number
         *
         * @note        Value is 0-indexed
         *
         * @param[in]   pinNum  PinNum number
         *
         * @return      PinNum represented by a bit-mask
         */
        static Pin::Mask convert (const uint8_t pinNum);

    public:
        /**
         * @brief   Public no-arg constructor - useful when you want a member
         *          variable in a class but don't want to require the pin be
         *          passed into the constructor
         */
        Pin ();

        /**
         * @brief       Create a Pin variable
         *
         * @param[in]   mask    Bit-mask of pin; One of PropWare::Pin::Mask
         */
        Pin (const Pin::Mask mask);

        /**
         * @brief       Create a Pin variable
         *
         * @param[in]   mask        Bit-mask of pin; One of PropWare::Pin::Mask
         * @param[in]   direction   Direction to initialize pin; One of
         *                          PropWare::Pin::Dir
         */
        Pin (const Pin::Mask mask, const Pin::Dir direction);

        /**
         * @brief       Create a Pin variable
         *
         * @param[in]   pinNum  0-indexed integer value representing pin-number
         */
        Pin (const uint8_t pinNum);

        /**
         * @brief       Create a Pin variable
         *
         * @param[in]   pinNum      0-indexed integer value representing
         *                          pin-number
         * @param[in]   direction   Direction to initialize pin; One of
         *                          PropWare::Pin::Dir
         */
        Pin (const uint8_t pinNum, const Pin::Dir direction);

        /**
         * @brief       Set the pin mask for a pin
         *
         * @param[in]   mask    Pin mask
         */
        void set_mask (const PropWare::Pin::Mask mask);

        /**
         * @brief   Return this pin's bit-mask
         *
         * @return  One of PropWare::Pin::Mask
         */
        Pin::Mask get_mask ();

        /**
         * @brief       Set selected pin as either input or output
         *
         * @param[in]   dir     I/O direction to set selected pin; must be one
         *                      of PropWare::Pin::IN or PropWare::Pin::OUT
         */
        void set_dir (const PropWare::Pin::Dir direction) const;

        /**
         * @brief   Determine the direction this pin is currently set to
         *
         * @return  Returns either PropWare::Pin::OUT or PropWare::Pin::IN
         */
        Pin::Dir get_dir () const;

        /**
         * @brief   Set selected output pin high (set it to 1)
         *
         * @pre     If pin is not set as output, statement will have no affect
         */
        void set () const;

        /**
         * @see PropWare::Pin::set()
         */
        void high () const;

        /**
         * @see PropWare::Pin::set()
         */
        void on () const;

        /**
         * @brief   Clear selected output pin (set it to 0)
         *
         * @pre     If pin is not set as output, statement will have no affect
         */
        void clear () const;

        /**
         * @see PropWare::Pin::clear()
         */
        void low () const;

        /**
         * @see PropWare::Pin::clear()
         */
        void off () const;

        /**
         * @brief   Toggle the output value of a pin
         *
         * @pre     If pin is not set as output, statement will have no affect
         */
        void toggle () const;

        /**
         * @brief   Read the value from a single pin and return its state
         *
         * @return  True if the pin is high, False if the pin is low
         */
        bool read () const;

        /**
         * @brief   Return the state of a pin via it's bit-mask
         *
         * @return  If the pin is high, the pin's mask will be returned; if low,
         *          PropWare::Pin::NULL_MASK (0) will be returned
         */
        Pin::Mask read2 () const;

        /**
         * @brief   Allow easy switch-press detection of any pin; Includes
         *          de-bounce protection
         *
         * @return  Returns 1 or 0 depending on whether the switch was pressed
         */
        bool isSwitchLow () const;

        /**
         * @brief       Allow easy switch-press detection of any pin; Includes
         *              de-bounce protection
         *
         * @param[in]   debounceDelayInMillis   Set the de-bounce delay in units
         *                                      of milliseconds
         *
         * @return      Returns 1 or 0 depending on whether the switch was
         *              pressed
         */
        bool isSwitchLow (const uint16_t debounceDelayInMillis) const;

    public:
        PropWare::Pin* operator= (const PropWare::Pin &rhs);

    private:
        Pin::Mask m_mask;
};

}

#endif /* PROPWARE_PIN_H_ */
