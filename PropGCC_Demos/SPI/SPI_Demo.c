/* File:    SPI_Demo.c
 * 
 * Author:  David Zemon
 * Project: SPI_Demo
 */

#include "SPI_Demo.h"

// Main function
void main (void) {
	char string[50] = "Hello world!\n";					// Create a string constant to output over SPI
	char *s;											// Create a pointer variable that can be incremented in a loop
	char in;											// Create an input variable to store received values from SPI

	SPIStart(MOSI, MISO, SCLK, FREQ, POLARITY);			// Initialize SPI module, giving it pin masks for the physical pins, frequency for the clock, and polarity of the clock
	GPIODirModeSet(CS, GPIO_DIR_OUT);					// Set chip select as an output (Note: the SPI module does not control chip select)

	while (1) {
		s = string;										// Set the pointer to the beginning of the string
		while (*s) {									// Loop until we read the null-terminator

			waitcnt(CLKFREQ/100 + CNT);

			in = 0xff;									// Reset input variable
			GPIOPinClear(CS);							// Enable the SPI slave attached to CS
			SPIShiftOut(8, *(s++), SPI_MSB_FIRST);		// Output the next character of the string and increment the pointer to following character - note the "SPI_MSB_FIRST", meaning we intend to send the data out MSB-first
			SPIWait();
			GPIOPinSet(CS);								// Disable the SPI slave attached to CS

			waitcnt(CLKFREQ/100 + CNT);

			GPIOPinClear(CS);
			SPIShiftIn(8, SPI_MSB_PRE, &in, 1);			// Read in a value from the SPI device - note the "SPI_MSB_PRE", meaning we intend to interpret the data as MSB-first, and the data will be valid before (PRE) the clock cycle
			GPIOPinSet(CS);

			putchar(in);								// Print the character to the screen
		}
	}
}
