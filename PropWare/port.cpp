/**
 * @file        port.cpp
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

#include <PropWare/port.h>

PropWare::Port::Port () {
    this->m_mask = 0;
}

uint32_t PropWare::Port::get_mask () {
    return this->m_mask;
}

void PropWare::Port::set_dir (const PropWare::Pin::Dir direction) const {
    DIRA = (DIRA & ~(this->m_mask))
            | (this->m_mask & (int32_t) direction);
}

void PropWare::Port::write (const uint32_t value) const {
    OUTA = ((OUTA & ~(this->m_mask)) | (value & this->m_mask));
}

void PropWare::Port::toggle () const {
    OUTA ^= this->m_mask;
}

uint32_t PropWare::Port::read () const {
    return INA & this->m_mask;
}

PropWare::SimplePort::SimplePort (const PropWare::Pin::Mask firstPin,
        uint8_t portWidth) {
    this->set_mask(firstPin, portWidth);
}

PropWare::SimplePort::SimplePort (const PropWare::Pin::Mask firstPin,
        uint8_t portWidth, const PropWare::Pin::Dir direction) {
    this->set_mask(firstPin, portWidth);
    this->set_dir(direction);
}

PropWare::SimplePort::SimplePort (const uint8_t firstPin, uint8_t portWidth) {
    this->set_mask(firstPin, portWidth);
}

PropWare::SimplePort::SimplePort (const uint8_t firstPin, uint8_t portWidth,
        const PropWare::Pin::Dir direction) {
    this->set_mask(firstPin, portWidth);
    this->set_dir(direction);
}

void PropWare::SimplePort::set_mask (const PropWare::Pin::Mask firstPin,
        uint8_t width) {
    this->m_mask = firstPin;
    this->m_firstPinNum = PropWare::Pin::convert(firstPin);

    // For every pin in the port...
    while (--width)
        // Add the next pin to the mask
        this->m_mask |= this->m_mask << 1;
}

void PropWare::SimplePort::set_mask (const uint8_t firstPin,
        uint8_t portWidth) {
    this->set_mask(PropWare::Pin::convert(firstPin), portWidth);
}

void PropWare::SimplePort::write (uint32_t value) {
    this->Port::write(value << this->m_firstPinNum);
}

uint32_t PropWare::SimplePort::read () {
    return this->Port::read() >> this->m_firstPinNum;
}

PropWare::FlexPort::FlexPort (const uint32_t portMask) {
    this->m_mask = portMask;
}

PropWare::FlexPort::FlexPort (const uint32_t portMask,
        const PropWare::Pin::Dir direction) {
    this->m_mask = portMask;
    this->set_dir(direction);
}

PropWare::FlexPort::FlexPort (PropWare::Pin *pins) {
    while (PropWare::Pin::NULL_PIN != pins->get_mask()) {
        this->m_mask |= pins->get_mask();
        ++pins;
    }
}

PropWare::FlexPort::FlexPort (PropWare::Pin *pins,
        const PropWare::Pin::Dir direction) {
    while (PropWare::Pin::NULL_PIN != pins->get_mask()) {
        this->m_mask |= pins->get_mask();
        ++pins;
    }

    this->set_dir(direction);
}

void PropWare::FlexPort::set_mask (const uint32_t mask) {
    this->m_mask = mask;
}

void PropWare::FlexPort::add_pins (const uint32_t mask) {
    this->m_mask |= mask;
}

void PropWare::FlexPort::write (const uint32_t value) const {
    this->Port::write(value);
}

uint32_t PropWare::FlexPort::read () const {
    return this->Port::read();
}

PropWare::FlexPort& PropWare::FlexPort::operator= (SimplePort &rhs) {
    this->m_mask = rhs.get_mask();
    return *this;
}
