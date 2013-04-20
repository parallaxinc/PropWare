/* File:    SD_Driver_gcc.c
 * 
 * Author:  David Zemon
 * Project: SPI_Lib_gcc
 */

#include "SD_Demo.h"

// Main function
void main (void) {
	uint8 err, i;
	uint16 temp = 0;
	uint32 len;
	char str[128];

#ifdef DEBUG
	__simple_printf("Beginning SD card initialization...\n");
#endif

	if (err = SDStart(MOSI, MISO, SCLK, CS))
		error(err);
#ifdef DEBUG
	__simple_printf("SD routine started. Mounting now...\n");
#endif
	if (err = SDMount())
		error(err);
#ifdef DEBUG
	__simple_printf("FAT partition mounted!\n");
#endif

#ifdef SD_SHELL
	SD_Shell();
#else
	SDFind("SPI.H", &temp);
#endif

	GPIODirModeSet(BIT_16, GPIO_DIR_OUT);
//	__simple_printf("Done!!!\n");
	while (1) {
		GPIOPinToggle(BIT_16);
		waitcnt(CLKFREQ + CNT);
	}
}

void error (const uint8 err) {
#ifdef DEBUG
	__simple_printf("Unknown error %u\n", err);
#endif
	while (1)
		;
}
