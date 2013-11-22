/**
 * @file    PropWare.c
 *
 * @author  David Zemon
 */

#include <PropWare.h>

uint8_t GPIOSwitchRead_Low (uint32_t pin) {
    DIRA &= ~pin; // Set the pin as input

    if ((OUTA & pin) ^ pin) {	// If pin is grounded (aka, pressed)
        // Delay 3 ms
        waitcnt(CLKFREQ*DEBOUNCE_DELAY/100 + CNT);

        if ((OUTA & pin) ^ pin)	// Check if it's still pressed
            return 1;
    }

    return 0;
}

uint8_t PropWareCountBits (uint32_t par) {
    /* Brian Kernighan's method for counting set bits in a variable */
    uint32_t c;					// c accumulates the total bits set in par
    for (c = 0; par; ++c)
        par &= par - 1;				// clear the least significant bit set

    return c;
}

uint8_t PropWareGetPinNum (const uint32_t pinMask) {
    uint8_t temp = 0;
    while (!(0x01 & (pinMask >> temp++)))
        ;
    return --temp;
}
