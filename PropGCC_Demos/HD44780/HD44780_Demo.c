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
		__simple_printf("Oh shit...\n");

	HD44780_puts("hello");

	return;
}
