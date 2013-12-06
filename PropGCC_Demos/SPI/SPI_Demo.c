/* File:    SPI_Demo.c
 *
 * Author:  David Zemon
 * Project: SPI_Demo
 */

#include "SPI_Demo.h"

extern uint32_t SPI_DUMMY_VAR[];

// Main function
_NORETURN void main (void) {
	char string[] = "Hello world!\n";		// Create a string constant to output over SPI
	char *s;				// Create a pointer variable that can be incremented in a loop
	char in;				// Create an input variable to store received values from SPI

	SPIStart(MOSI, MISO, SCLK, FREQ, MODE, BITMODE);// Initialize SPI module, giving it pin masks for the physical pins, frequency for the clock, mode of SPI, and bitmode
	GPIODirModeSet(CS, GPIO_DIR_OUT);// Set chip select as an output (Note: the SPI module does not control chip select)

	GPIODirModeSet(BYTE_2, GPIO_DIR_OUT);

	while (1) {
		s = string;						// Set the pointer to the beginning of the string
		while (*s) {							// Loop until we read the null-terminator

			waitcnt(CLKFREQ/100 + CNT);

			GPIOPinClear(CS);						// Enable the SPI slave attached to CS
			SPIShiftOut(8, *s);  // Output the next character of the string and increment the pointer to following character
			SPIWait();  // Be sure to wait until the assembly cog has *finished* shifting out data
			GPIOPinSet(CS);						// Disable the SPI slave attached to CS

			waitcnt(CLKFREQ/100 + CNT);
			in = 0xff;									// Reset input variable
			while (in != *s) {
				GPIOPinClear(CS);
				SPIShiftIn(8, &in, 1);  // Read in a value from the SPI device
				GPIOPinSet(CS);
			}

			++s;

			putchar(in);							// Print the character to the screen

			in = SPI_DUMMY_VAR;
		}

		GPIOPinToggle(BYTE_2);
	}
}
