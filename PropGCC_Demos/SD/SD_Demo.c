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

	sd_file f, f2;

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
	sd_buffer fileBuf2;
	f.buf = &fileBuf;
	f2.buf = &fileBuf2;
#else
	/* Option 2: Use the generic buffer, g_sd_buf, as the buffer
	 *
	 * Good for low-RAM situations due to the re-use of g_sd_buf. Speed is
	 * decreased when multiple files are used often.
	 *
	 */
	f.buf = &g_sd_buf;
	f2.buf = &g_sd_buf;
#endif

#ifdef DEBUG
	printf("Beginning SD card initialization...\n");
#endif

	if (err = SDStart(MOSI, MISO, SCLK, CS))
		error(err);
#ifdef DEBUG
	printf("SD routine started. Mounting now...\n");
#endif
	if (err = SDMount())
		error(err);
#ifdef DEBUG
	printf("FAT partition mounted!\n");
#endif

#ifdef SD_SHELL
	SD_Shell(&f);
#else
	// Copy the contents of SD.H into SD.C (SD.H -> SD.C as opposed to SD.C -> SD.H
	// because this will not increase the file size. Increasing file size is not implemented)
	SDfopen("SD.C", &f);
	SDfopen("SD.H", &f2);

	while (!SDfeof(&f2)) {
		SDfgets(str, 128, &f2);
		SDfputs(str, &f);
	}

//	// Re-open the file and print the (hopefully) new contents to the screenSDfopen("SPI_AS.S", &f);
//	while (!SDfeof(&f)) {
//		SDfgets(str, 128, &f);
//		printf("%s", str);
//	}
#endif

#ifdef DEBUG
	printf("Execution complete!\n");
#endif

	GPIODirModeSet(BIT_16, GPIO_DIR_OUT);
	while (1) {
		GPIOPinToggle(BIT_16);
		waitcnt(CLKFREQ/2 + CNT);
	}
}

void error (const uint8 err) {
#ifdef DEBUG
	printf("Unknown error %u\n", err);
#endif
	while (1)
		;
}
