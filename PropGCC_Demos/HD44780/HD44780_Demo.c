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

	__simple_printf("Hello!!!\n");

	if ((err = HD44780Start(DATA, RS, RW, EN, BITMODE, DIMENSIONS)))
		__simple_printf("Oh shit... %u\n", err - HD44780_ERRORS_BASE);

//	HD44780_putchar('H');
//	HD44780_putchar('e');
//	HD44780_putchar('l');
//	HD44780_putchar('l');
//	HD44780_putchar('o');
//	HD44780_putchar('!');
	HD44780_puts("Hello");

//	HD44780Move(0, 4);

	return;
}
