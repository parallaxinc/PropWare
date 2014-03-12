/**
 * @file    PropWare.cpp
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


#include <tinyio.h>
#include <PropWare/PropWare.h>

void PropWare::GPIO::set_dir (const uint32_t pins, PropWare::GPIO::Dir dir) {
    DIRA = (DIRA & (~pins)) | (pins & (int32_t) dir);
}

void PropWare::GPIO::pin_set (const uint32_t pins) {
    OUTA |= pins;
}

void PropWare::GPIO::pin_clear (const uint32_t pins) {
    OUTA &= ~pins;
}

void PropWare::GPIO::pin_toggle (const uint32_t pins) {
    OUTA ^= pins;
}

void PropWare::GPIO::pin_write (const uint32_t pins, const uint32_t value) {
    OUTA = (OUTA & ~pins) | (value & pins);
}

uint32_t PropWare::GPIO::read_multi_pin (const uint32_t pins) {
    return INA & pins;
}

bool PropWare::GPIO::read_pin (const PropWare::GPIO::Pin pin) {
    return INA & pin;
}

bool PropWare::GPIO::read_switch_low (const PropWare::GPIO::Pin pin) {
    PropWare::GPIO::set_dir(pin, PropWare::GPIO::IN); // Set the pin as input

    if ((OUTA & pin) ^ pin) {   // If pin is grounded (aka, pressed)
        // Delay 3 ms
        waitcnt(CLKFREQ*DEBOUNCE_DELAY/100 + CNT);

        if ((OUTA & pin) ^ pin) // Check if it's still pressed
            return true;
    }

    return false;
}

uint8_t PropWare::count_bits (uint32_t par) {
    /* Brian Kernighan's method for counting set bits in a variable */
    uint32_t c;                     // c accumulates the total bits set in par
    for (c = 0; par; ++c)
        par &= par - 1;             // clear the least significant bit set

    return c;
}

uint8_t PropWare::get_pin_num (const PropWare::GPIO::Pin pin) {
    uint8_t temp = 0;
    while (!(0x01 & (pin >> temp++)))
        ;
    return --temp;
}

PropWare::GPIO::Pin get_pin_from_num (const uint8_t pinNum) {
    switch (pinNum) {
        case 0:
            return PropWare::GPIO::P0;
        case 1:
            return PropWare::GPIO::P1;
        case 2:
            return PropWare::GPIO::P2;
        case 3:
            return PropWare::GPIO::P3;
        case 4:
            return PropWare::GPIO::P4;
        case 5:
            return PropWare::GPIO::P5;
        case 6:
            return PropWare::GPIO::P6;
        case 7:
            return PropWare::GPIO::P7;
        case 8:
            return PropWare::GPIO::P8;
        case 9:
            return PropWare::GPIO::P9;
        case 10:
            return PropWare::GPIO::P10;
        case 11:
            return PropWare::GPIO::P11;
        case 12:
            return PropWare::GPIO::P12;
        case 13:
            return PropWare::GPIO::P13;
        case 14:
            return PropWare::GPIO::P14;
        case 15:
            return PropWare::GPIO::P15;
        case 16:
            return PropWare::GPIO::P16;
        case 17:
            return PropWare::GPIO::P17;
        case 18:
            return PropWare::GPIO::P18;
        case 19:
            return PropWare::GPIO::P19;
        case 20:
            return PropWare::GPIO::P20;
        case 21:
            return PropWare::GPIO::P21;
        case 22:
            return PropWare::GPIO::P22;
        case 23:
            return PropWare::GPIO::P23;
        case 24:
            return PropWare::GPIO::P24;
        case 25:
            return PropWare::GPIO::P25;
        case 26:
            return PropWare::GPIO::P26;
        case 27:
            return PropWare::GPIO::P27;
        case 28:
            return PropWare::GPIO::P28;
        case 29:
            return PropWare::GPIO::P29;
        case 30:
            return PropWare::GPIO::P30;
        case 31:
            return PropWare::GPIO::P31;
    }
}
