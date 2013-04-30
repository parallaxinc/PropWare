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
	char c;
	char str[128];

	sd_file f, f2;

#ifndef LOW_RAM_MODE
	/* Option 1: Create at least one new sd_buffer variable
	 *
	 * An extra 526 bytes of memory are required to create a new sd_buffer
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
#elif (defined SD_FILE_WRITE)
	// Create a blank file and copy the contents of STUFF.TXT into it
	printf("Changing into /STUFF/\n");
	SDchdir("STUFF");
	SDfopen("SOME.TXT", &f, SD_FILE_MODE_R);
	SDfopen("NEW.BAK", &f2, SD_FILE_MODE_R_PLUS);

	printf("Both files opened...\n");

	while (!SDfeof(&f)) {
		c = SDfgetc(&f);
		SDfputc(c, &f2);
#ifdef _STDIO_H
		putchar(SDfgetc(&f2));
#endif
	}

	printf("\nFile printed...\n");

	SDfclose(&f);
	SDfclose(&f2);

	printf("Files closed...\n");

	SDfopen("NEW.BAK", &f2, SD_FILE_MODE_R);
	while (!SDfeof(&f2))
		putchar(SDfgetc(&f));
	SDfclose(&f2);
#else

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
