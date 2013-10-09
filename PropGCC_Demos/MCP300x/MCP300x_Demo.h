/* File:    MCP300x_Demo.h
 *
 * Author:  David Zemon
 * Project: MCP300x_Demo
 */

#ifndef MCP300X_DEMO_H_
#define MCP300X_DEMO_H_

#include <propeller.h>
#include <stdio.h>
#include <stdlib.h>

#include <PropWare.h>
#include <mcp300x.h>

#define MOSI			BIT_0
#define MISO			BIT_1
#define SCLK			BIT_2
#define CS				BIT_3
#define FREQ			100000

// We're going to read from just channel 0 in this demo, but feel free to read from any that you like
#define CHANNEL			MCP_CHANNEL_1

#define DEBUG_LEDS		BYTE_2

void error (int8_t err) {
	uint32_t shiftedValue = err;
	shiftedValue <<= 16;  // Shift the error bits by 16 to put them atop the QUICKSTART LEDs

	GPIODirModeSet(DEBUG_LEDS, GPIO_DIR_OUT);

	while (1) {
		GPIOPinWrite(DEBUG_LEDS, shiftedValue);
		waitcnt(CLKFREQ/5 + CNT);
		GPIOPinClear(DEBUG_LEDS);
		waitcnt(CLKFREQ/5 + CNT);
	}
}

#endif /* MCP300X_DEMO_H_ */
