/**
 * @file        ping.h
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

#include <PropWare/pin.h>
#include <PropWare/utility.h>

namespace PropWare {

/**
 * @brief   Ultrasonic distance sensor such as the Parallax PING)))
 *
 * For 3-pin variants like the Parallax PING))) sensor, use the following template:
 *
 * @code
 * #include <PropWare/ping.h>
 * #include <PropWare/printer/printer.h>
 *
 * int main () {
 *     // Propeller pin P01 connected to the signal line of the PING)))
 *     const PropWare::Ping ping(PropWare::Pin::P01);
 *     unsigned int centimeters = ping.get_centimeters();
 *
 *     // User code here...
 *     pwOut << centimeters << '\n';
 *
 *     return 0;
 * }
 * @endcode
 */
class Ping {
    public:
        /**
         * @brief       Construct an instance for a 3-pin variant such as the Parallax PING))) sensor
         *
         * @param[in]   signalMask  Pin mask for the signal pin of the sensor
         */
        Ping(const PropWare::Pin::Mask signalMask) {
            this->m_trigger.set_mask(signalMask);
            this->m_echo.set_mask(signalMask);
        };

        /**
         * @brief       Construct an instance for a 4-pin variant, commonly found on eBay.
         *
         * @param[in]   triggerMask     Pin mask for the trigger pin
         * @param[in]   echoMask        Pin mask for the echo pin
         */
        Ping(const PropWare::Pin::Mask triggerMask, const PropWare::Pin::Mask echoMask) {
            this->m_trigger.set_mask(triggerMask);
            this->m_echo.set_mask(echoMask);
        };

        /**
         * @brief       Detect distance as measured in millimeters between sensor and object
         *
         * @note        Ultrasonic sensors and the Parallax Propeller are _not_ accurate enough for 1 mm precision. This
         *              method is only provided to give a finer grain of precision than Ping::get_centimeters without
         *              reverting to floating point numbers.
         *
         * @returns     Millimeters between sensor and object
         */
        unsigned int get_millimeters() const {
            return this->get_microseconds() * 17;
        }

        /**
         * @brief       Detect distance as measured in centimeters between sensor and object
         *
         * @returns     Centimeters between sensor and object. Fractional values are truncated.
         */
        unsigned int get_centimeters() const {
            return this->get_millimeters() / 1000;
        }

        /**
         * @brief       Detect distance as measured in inches between sensor and object
         *
         * @returns     Inches between sensor and object. Fractional values are truncated.
         */
        unsigned int get_inches() const {
            return this->get_microseconds() / 148;
        }

        /**
         * @brief       Time measured in microseconds for a sound wave to travel from the sensor to an object and back.
         *
         * @returns     Microseconds for a round trip. Fractional values are truncated.
         */
        unsigned int get_microseconds() const {
            return this->get_clock_ticks() / MICROSECOND;
        }

        /**
         * @brief       Time measured in system clock ticks for a sound wave to travel from the sensor to an object and
         *              back. This is the most accurate method on the Ping class.
         *
         * @returns     Clock ticks for a round trip.
         */
#ifdef PROPWARE_TEST
virtual
#endif
        unsigned int get_clock_ticks() const {
            this->m_trigger.set_dir_out();
            this->m_trigger.set();
            waitcnt(20 * MICROSECOND + CNT); // The spec for the ping sensor is 2us, but 20us is the fastest at CMM mode
            this->m_trigger.clear();

            this->m_echo.set_dir_in();
            waitpeq(this->m_echo.get_mask(), this->m_echo.get_mask());
            volatile uint32_t start = CNT;
            waitpne(this->m_echo.get_mask(), this->m_echo.get_mask());
            return CNT - start;
        }

    private:
        PropWare::Pin m_trigger;
        PropWare::Pin m_echo;
};

};
