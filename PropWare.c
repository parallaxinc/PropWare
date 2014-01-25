/**
 * @file    PropWare.c
 *
 * @author  David Zemon
 * @project PropWare
 */

/**
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

#include <PropWare.h>

uint8_t GPIOSwitchRead_Low (const uint32_t pin) {
    DIRA &= ~pin; // Set the pin as input

    if ((OUTA & pin) ^ pin) {   // If pin is grounded (aka, pressed)
        // Delay 3 ms
        waitcnt(CLKFREQ*DEBOUNCE_DELAY/100 + CNT);

        if ((OUTA & pin) ^ pin) // Check if it's still pressed
            return 1;
    }

    return 0;
}

uint8_t PropWareCountBits (uint32_t par) {
    /* Brian Kernighan's method for counting set bits in a variable */
    uint32_t c;                     // c accumulates the total bits set in par
    for (c = 0; par; ++c)
        par &= par - 1;             // clear the least significant bit set

    return c;
}

uint8_t PropWareGetPinNum (const uint32_t pinMask) {
    uint8_t temp = 0;
    while (!(0x01 & (pinMask >> temp++)))
        ;
    return --temp;
}
