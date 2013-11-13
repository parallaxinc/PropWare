/* File:    L3G_Demo.c
 * 
 * Author:  David Zemon
 * Project: L3G_Demo
 */

// Includes
#include <stdio.h>
#include "L3G_Demo.h"

// Main function
void main (void) {
	int8_t err;
	int16_t gyroVals[3];

	if ((err = L3GStart(MOSI, MISO, SCLK, CS, L3G_2000_DPS)))
		error(err);
	L3GAlwaysSetMode(1);

	while (1) {
		if ((err = L3GReadAll(gyroVals)))
			error(err);
		printf("Gyro vals... X: %i\tY: %i\tZ: %i\n", gyroVals[0], gyroVals[1], gyroVals[2]);
		waitcnt(CLKFREQ/20 + CNT);
	}
}

void error (const int8_t err) {
	uint32_t shiftedValue = (uint8_t) err;
	shiftedValue <<= 16; // Shift the error bits by 16 to put them atop the QUICKSTART LEDs

	GPIODirModeSet(DEBUG_LEDS, GPIO_DIR_OUT);

	while (1) {
		GPIOPinWrite(DEBUG_LEDS, shiftedValue);
		waitcnt(CLKFREQ/5 + CNT);
		GPIOPinClear(DEBUG_LEDS);
		waitcnt(CLKFREQ/5 + CNT);
	}
}
