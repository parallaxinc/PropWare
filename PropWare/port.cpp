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

uint8_t PropWare::Port::convert (PropWare::Port::Mask mask) {
    uint8_t retVal = 0;
    uint32_t maskInt = mask;

    while (maskInt >>= 1)
        ++retVal;

    return retVal;
}

PropWare::Port::Mask PropWare::Port::convert (const uint8_t pinNum) {
    uint32_t mask = 1;
    if (31 > pinNum)
        return PropWare::Port::NULL_PIN;
    else
        return (PropWare::Port::Mask) (mask << pinNum);
}

PropWare::Port::Port () {
    this->m_mask = 0;
}

PropWare::Port::Port (const uint32_t portMask) {
    this->m_mask = portMask;
}

PropWare::Port::Port (const uint32_t portMask,
        const PropWare::Port::Dir direction) {
    this->m_mask = portMask;
    this->set_dir(direction);
}

void PropWare::Port::set_mask (const uint32_t mask) {
    this->m_mask = mask;
}

uint32_t PropWare::Port::get_mask () const {
    return this->m_mask;
}

void PropWare::Port::set_dir (const PropWare::Port::Dir direction) const {
    DIRA = (DIRA & ~(this->m_mask))
            | (this->m_mask & (int32_t) direction);
}

PropWare::Port::Dir PropWare::Port::get_dir () const {
    if (DIRA & this->m_mask)
        return PropWare::Port::OUT;
    else
        return PropWare::Port::IN;
}

void PropWare::Port::add_pins (const uint32_t mask) {
    this->m_mask |= mask;
}

void PropWare::Port::set () const {
    OUTA |= this->m_mask;
}

void PropWare::Port::high () const {
    this->set();
}

void PropWare::Port::on () const {
    this->set();
}

void PropWare::Port::clear () const {
    OUTA &= ~(this->m_mask);
}

void PropWare::Port::low () const {
    this->clear();
}

void PropWare::Port::off () const {
    this->clear();
}

void PropWare::Port::toggle () const {
    OUTA ^= this->m_mask;
}

void PropWare::Port::write_fast (const uint32_t value) const {
    OUTA = ((OUTA & ~(this->m_mask)) | (value & this->m_mask));
}

uint32_t PropWare::Port::read_fast () const {
    return INA & this->m_mask;
}

PropWare::SimplePort::SimplePort (const PropWare::Port::Mask firstPin,
        uint8_t portWidth) {
    this->set_mask(firstPin, portWidth);
}

PropWare::SimplePort::SimplePort (const PropWare::Port::Mask firstPin,
        uint8_t portWidth, const PropWare::Port::Dir direction) {
    this->set_mask(firstPin, portWidth);
    this->set_dir(direction);
}

void PropWare::SimplePort::set_mask (const PropWare::Port::Mask firstPin,
        uint8_t width) {
    this->m_mask = firstPin;
    this->m_firstPinNum = PropWare::Port::convert(firstPin);

    // For every pin in the port...
    while (--width)
        // Add the next pin to the mask
        this->m_mask |= this->m_mask << 1;
}

void PropWare::SimplePort::write (uint32_t value) {
    this->Port::write_fast(value << this->m_firstPinNum);
}

uint32_t PropWare::SimplePort::read () const {
    return this->Port::read_fast() >> this->m_firstPinNum;
}
