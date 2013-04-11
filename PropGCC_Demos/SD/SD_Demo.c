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
	__simple_printf("SD routine started. Mounting now...\n");
	if (err = SDMount())
		error(err);
	__simple_printf("FAT partition mounted!\n");

	__simple_printf("\nPrinting root directory\n");
	if (err = SD_Shell_ls())
		error(err);

	if (err = SD_Shell_cat("SD.C"))
		error(err);

	if (err = SD_Shell_cat("PROFILE.TXT"))
		error(err);

	if (err = SD_Shell_ls())
		error(err);

	GPIODirModeSet(BIT_16, GPIO_DIR_OUT);
	__simple_printf("Done!!!\n");
	while (1) {
		GPIOPinToggle(BIT_16);
		waitcnt(CLKFREQ + CNT);
	}
}

void error (const uint8 err) {
	__simple_printf("Unknown error %u\n", err);
	while (1)
		;
}
