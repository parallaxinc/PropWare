/* File:    SD_Demo.c
 * 
 * Author:  David Zemon
 * Project: SD_Demo
 */

#include "SD_Demo.h"

// Main function
void main (void) {
	uint8_t err;
	char c;

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
	/* Option 2: Use the generic buffer [i.e. g_sd_buf] as the buffer
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

	// Start your engines!!!
	if ((err = SDStart(MOSI, MISO, SCLK, CS, -1)))
		error(err);

#ifdef DEBUG
	printf("SD routine started. Mounting now...\n");
#endif
	if ((err = SDMount()))
		error(err);
#ifdef DEBUG
	printf("FAT partition mounted!\n");
#endif

#ifdef SD_SHELL
	SD_Shell(&f);
#elif (defined SD_FILE_WRITE)
	// Create a blank file and copy the contents of STUFF.TXT into it
	SDfopen(OLD_FILE, &f, SD_FILE_MODE_R);
	SDfopen(NEW_FILE, &f2, SD_FILE_MODE_R_PLUS);

#ifdef DEBUG
	printf("Both files opened...\n");
#endif

	while (!SDfeof(&f)) {
		c = SDfgetc(&f);
		SDfputc(c, &f2);
#ifdef _STDIO_H
		putchar(SDfgetc(&f2));
//		putchar(c);
#endif
	}

#ifdef DEBUG
	printf("\nFile printed...\n");

	printf("Now closing read-only file!\n");
#endif
	SDfclose(&f);
#ifdef DEBUG
	printf("***Now closing the modified file!***\n");
#endif
	SDfclose(&f2);

#ifdef DEBUG
	printf("Files closed...\n");

	SDfopen(NEW_FILE, &f2, SD_FILE_MODE_R);
	printf("File opened for a second time, now printing new contents...\n");
	while (!SDfeof(&f2))
		putchar(SDfgetc(&f2));
	SDfclose(&f2);
#endif

	SDUnmount();
#else
	SDchdir("JAZZ");
	SDfopen("DESKTOP.INI", &f, SD_FILE_MODE_R);

	while (!SDfeof(&f))
#ifdef DEBUG
		putchar(SDfgetc(&f));
#endif
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

void error (const uint8_t err) {
#ifdef DEBUG
	if (SD_ERRORS_BASE <= err && err < SD_ERRORS_LIMIT)
		printf("SD error %u\n", err - SD_ERRORS_BASE);
	else
		printf("Unknown error %u\n", err);
#endif
	while (1)
		;
}
