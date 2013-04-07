/* File:    SD_Driver_gcc.c
 * 
 * Author:  David Zemon
 * Project: SPI_Lib_gcc
 */

#include "SD_Demo.h"

// Main function
void main (void) {
	uint8 err, i;

	__simple_printf("Beginning SD card initialization...\n");

	if (err = SDStart(MOSI, MISO, SCLK, CS))
		error(err);
	if (err = SDMount())
		error(err);

	GPIODirModeSet(BIT_16, GPIO_DIR_OUT);
	while (1) {
		GPIOPinToggle(BIT_16);
		__simple_printf("It's working!!!\n");
		waitcnt(CLKFREQ + CNT);
	}
}

void error (const uint8 err) {
	__simple_printf("Unknown error %u\n", err);
	while (1)
		;
}
