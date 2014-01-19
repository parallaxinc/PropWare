/**
 * @file    MAX6675_Demo.h
 *
 * @author  David Zemon
 * @project MAX6675_Demo
 */

#include "MAX6675_Demo.h"
#include <stdio.h>

void main (void) {
    int8_t err;
    uint16_t data;
    char buffer[32];
    uint32_t loopCounter;

    if ((err = MAX6675Start(MOSI, MISO, SCLK, CS)))
        error(err);

    if ((err = HD44780Start(DATA, RS, RW, EN, BITMODE, DIMENSIONS)))
        error(err);

    // Though this functional call is not necessary (default value is 0), I
    // want to bring attention to this function. It will determine whether the
    // MAX6675Read* functions will always explicitly set the SPI modes before
    // each call, or assume that the SPI cog is still running in the proper
    // configuration
    MAX6675AlwaysSetMode(1);

    HD44780_puts("Welcome to the MAX6675 demo!\n");

    while (1) {
        loopCounter = CLKFREQ / 2 + CNT;

        if ((err = MAX6675Read(&data)))
            error(err);

        sprintf(buffer, "Temp: %u.%uC\n", data >> 2, (data & 0x3) * 25);
        HD44780Clear();
        HD44780_puts(buffer);

        waitcnt(loopCounter);
    }
}

void error (int8_t err) {
    uint32_t shiftedValue = (uint8_t) err;

    // Shift the error bits by 16 to put them atop the QUICKSTART LEDs
    shiftedValue <<= 16;

    GPIODirModeSet(DEBUG_LEDS, GPIO_DIR_OUT);

    while (1) {
        GPIOPinWrite(DEBUG_LEDS, shiftedValue);
        waitcnt(CLKFREQ/5 + CNT);
        GPIOPinClear(DEBUG_LEDS);
        waitcnt(CLKFREQ/5 + CNT);
    }
}
