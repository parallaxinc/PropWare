/**
 * @file    pin.cpp
 *
 *
 * @project PropWare
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

#include <PropWare/pin.h>

PropWare::Pin::Pin (const uint8_t number) {
    this->m_mask = PropWare::Pin::convert(number);
}

PropWare::Pin::Pin (const uint8_t number, const PropWare::Pin::Dir direction) {
    this->m_mask = PropWare::Pin::convert(number);
    this->set_dir(direction);
}

void PropWare::Pin::set_mask (const PropWare::Port::Mask mask) {
    this->PropWare::Port::set_mask(mask);
}

bool PropWare::Pin::read () const {
    return (bool) this->read_fast();
}

bool PropWare::Pin::is_switch_low () const {
    return this->is_switch_low(PropWare::Pin::DEBOUNCE_DELAY);
}

bool PropWare::Pin::is_switch_low (const uint16_t debounceDelayInMillis) const {
    this->set_dir(PropWare::Pin::IN);  // Set the pin as input

    if (!(this->read())) {   // If pin is grounded (aka, pressed)
        // Delay 3 ms
        waitcnt(debounceDelayInMillis*MILLISECOND + CNT);

        return !(this->read());  // Check if it's still pressed
    }

    return false;
}

PropWare::Pin* PropWare::Pin::operator= (const PropWare::Pin &rhs) {
    this->m_mask = rhs.m_mask;
    return this;
}

bool PropWare::Pin::operator== (const PropWare::Pin &rhs) {
    return this->m_mask == rhs.m_mask;
}
