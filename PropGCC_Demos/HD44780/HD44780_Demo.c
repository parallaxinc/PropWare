/* File:    HD44780_Demo.c
 * 
 * Author:  David Zemon
 * Project: HD44780_Demo
 */

// Includes
#include "HD44780_Demo.h"

// Main function
void main (void) {
	uint8_t err;

	if ((err = HD44780Start(DATA, RS, RW, EN, BITMODE, DIMENSIONS)))
		error(err);

	HD44780_puts("0123456789abcdef0123456789abcdef");
	HD44780_puts("Hello world!!!");

	return;
}

void error (const uint8_t err) {
	uint32_t out = err;
	out <<= 16;

	GPIODirModeSet(BYTE_2, GPIO_DIR_OUT);

	while (1) {
		GPIOPinWrite(BYTE_2, out);
		waitcnt(150*MILLISECOND + CNT);
		GPIOPinClear(BYTE_2);
		waitcnt(150*MILLISECOND + CNT);
	}
}
