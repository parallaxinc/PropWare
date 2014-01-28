/**
 * @file    L3G_Demo.c
 * 
 * @author  David Zemon
 * @project L3G_Demo
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

    // Though this functional call is not necessary (default value is 0), I
    // want to bring attention to this function. It will determine whether the
    // L3GRead* functions will always explicitly set the SPI modes before
    // each call, or assume that the SPI cog is still running in the proper
    // configuration
    L3GAlwaysSetSPIMode(1);

    while (1) {
        if ((err = L3GReadAll(gyroVals)))
            error(err);
        printf("Gyro vals... X: %i\tY: %i\tZ: %i\n", gyroVals[0], gyroVals[1],
                gyroVals[2]);
        waitcnt(CLKFREQ/20 + CNT);
    }
}

void error (const int8_t err) {
    uint32_t shiftedValue = (uint8_t) err;

    // Shift the error bits by 16 to put them atop the QUICKSTART LEDs
    shiftedValue <<= 16;

    // Set the Quickstart LEDs for output (used to display the error code)
    GPIODirModeSet(DEBUG_LEDS, GPIO_DIR_OUT);

    while (1) {
        GPIOPinWrite(DEBUG_LEDS, shiftedValue);
        waitcnt(CLKFREQ/5 + CNT);
        GPIOPinClear(DEBUG_LEDS);
        waitcnt(CLKFREQ/5 + CNT);
    }
}
