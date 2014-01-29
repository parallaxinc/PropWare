/**
 * @file    SPI_Demo.c
 *
 * @author  David Zemon
 * @project SPI_Demo
 */

#include "SPI_Demo.h"

// Main function
void main (void) {
    const char string[] = "Hello world!\n";  // Create the test string
    char *s;    // Create a pointer variable that can be incremented in a loop
    char in;    // Create an input variable to store received values from SPI

    // Initialize SPI module, giving it pin masks for the physical pins,
    // frequency for the clock, mode of SPI, and bitmode
    SPIStart(MOSI, MISO, SCLK, FREQ, MODE, BITMODE);

    // Set chip select as an output (Note: the SPI module does not control chip
    // select)
    GPIODirModeSet(CS, GPIO_DIR_OUT);

    GPIODirModeSet(BYTE_2, GPIO_DIR_OUT);

    while (1) {
        s = string;         // Set the pointer to the beginning of the string
        while (*s) {        // Loop until we read the null-terminator

            waitcnt(CLKFREQ/100 + CNT);

            GPIOPinClear(CS);       // Enable the SPI slave attached to CS
            SPIShiftOut(8, *s);  // Output the next character of the string

            // Be sure to wait until the SPI communication has FINISHED before
            // proceeding to set chip select high
            SPIWait();
            GPIOPinSet(CS);

            waitcnt(CLKFREQ/100 + CNT);
            in = 0xff;              // Reset input variable
            while (in != *s) {
                GPIOPinClear(CS);
                SPIShiftIn(8, &in, 1);  // Read in a value from the SPI device
                GPIOPinSet(CS);
            }

            ++s;

            // Print the character to the screen
            putchar(in);            
        }

        // Signal that the entire string has been sent
        GPIOPinToggle(BYTE_2);
    }
}
