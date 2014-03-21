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

uint8_t PropWare::Pin::convert (PropWare::Pin::Mask mask) {
    uint8_t retVal = 0;
    uint32_t maskInt = mask;

    while (maskInt >>= 1)
        ++retVal;

    return retVal;
}

PropWare::Pin::Mask PropWare::Pin::convert (const uint8_t pinNum) {
    uint32_t mask = 1;
    if (31 > pinNum)
        return PropWare::Pin::NULL_PIN;
    else
        return (PropWare::Pin::Mask) (mask << pinNum);
}

PropWare::Pin::Pin () {
    this->m_mask = PropWare::Pin::NULL_PIN;
}

PropWare::Pin::Pin (const PropWare::Pin::Mask mask) {
    this->m_mask = mask;
}

PropWare::Pin::Pin (const PropWare::Pin::Mask mask,
        const PropWare::Pin::Dir direction) {
    this->m_mask = mask;
    this->set_dir(direction);
}

PropWare::Pin::Pin (const uint8_t number) {
    this->m_mask = PropWare::Pin::convert(number);
}

PropWare::Pin::Pin (const uint8_t number, const PropWare::Pin::Dir direction) {
    this->m_mask = PropWare::Pin::convert(number);
    this->set_dir(direction);
}

void PropWare::Pin::set_mask (const PropWare::Pin::Mask mask) {
    this->m_mask = mask;
}

PropWare::Pin::Mask PropWare::Pin::get_mask () {
    return this->m_mask;
}

void PropWare::Pin::set_dir (const PropWare::Pin::Dir direction) const {
    DIRA = (DIRA & ~(this->m_mask)) | (this->m_mask & (int32_t) direction);
}

PropWare::Pin::Dir PropWare::Pin::get_dir () const {
    if (DIRA & this->m_mask)
        return PropWare::Pin::OUT;
    else
        return PropWare::Pin::IN;
}

void PropWare::Pin::set () const {
    OUTA |= this->m_mask;
}

void PropWare::Pin::high () const {
    this->set();
}

void PropWare::Pin::on () const {
    this->set();
}

void PropWare::Pin::clear () const {
    OUTA &= ~(this->m_mask);
}

void PropWare::Pin::low () const {
    this->clear();
}

void PropWare::Pin::off () const {
    this->clear();
}

void PropWare::Pin::toggle () const {
    OUTA ^= this->m_mask;
}

bool PropWare::Pin::read () const {
    return INA & this->m_mask;
}

PropWare::Pin::Mask PropWare::Pin::read2 () const {
    return (PropWare::Pin::Mask) (INA & this->m_mask);
}

bool PropWare::Pin::isSwitchLow () const {
    return this->isSwitchLow(PropWare::Pin::DEBOUNCE_DELAY);
}

bool PropWare::Pin::isSwitchLow (const uint16_t debounceDelayInMillis) const {
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
