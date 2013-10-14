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

//	__simple_printf("Hello!!!\n");

	if ((err = HD44780Start(DATA, RS, RW, EN, BITMODE, DIMENSIONS)))
		error(err);

//	HD44780_putchar('H');
//	HD44780_putchar('e');
//	HD44780_putchar('l');
//	HD44780_putchar('l');
//	HD44780_putchar('o');
//	HD44780_putchar('!');
	HD44780_puts("0123456789abcdef0123456789abcdefHello world!!!");

//	HD44780Move(0, 0);
//	HD44780_puts("Hello!");
//	HD44780Move(0, 4);

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
