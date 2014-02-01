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

    if ((err = l3g_start(MOSI, MISO, SCLK, CS, L3G_2000_DPS)))
        error(err);

    // Though this functional call is not necessary (default value is 0), I
    // want to bring attention to this function. It will determine whether the
    // l3g_read* functions will always explicitly set the SPI modes before
    // each call, or assume that the SPI cog is still running in the proper
    // configuration
    l3g_always_set_spi_mode(1);

    while (1) {
        if ((err = l3g_read_all(gyroVals)))
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
    gpio_set_dir(DEBUG_LEDS, GPIO_DIR_OUT);

    while (1) {
        gpio_pin_write(DEBUG_LEDS, shiftedValue);
        waitcnt(CLKFREQ/5 + CNT);
        gpio_pin_clear(DEBUG_LEDS);
        waitcnt(CLKFREQ/5 + CNT);
    }
}
