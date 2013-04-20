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

	sd_file f;

#ifndef LOW_RAM_MODE
	/* Option 1: Create a new sd_buffer variable
	 *
	 * An extra 525 bytes of memory are required to create a new sd_buffer
	 * for the file variable, but speed will be increased if files are
	 * being switched often. Using this option will allow the directory
	 * contents to be kept in RAM while a file is loaded.
	 *
	 */
	sd_buffer fileBuf;	// If extra RAM is available
	f.buf = &fileBuf;
#else
	/* Option 2: Use the generic buffer, g_sd_buf, as the buffer
	 *
	 * Good for low-RAM situations due to the re-use of g_sd_buf. Speed is
	 * decreased when multiple files are used often.
	 *
	 */
	f.buf = &g_sd_buf;
#endif

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
	SD_Shell(&f);
#else
	SDfopen("SPI_AS.S", &f);
	while (!SDfeof(&f)) {
		SDfgets(str, 128, &f);
		__simple_printf("%s", str);
	}
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
