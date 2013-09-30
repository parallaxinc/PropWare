/* File:    gpio.c
 *
 * Author:  David Zemon
 */

#include <gpio.h>

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
