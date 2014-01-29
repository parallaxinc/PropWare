/**
 * @file    SPI_Demo.h
 *
 * @author  David Zemon
 * @project SPI_Demo
 */

#ifndef SPI_DEMO_H_
#define SPI_DEMO_H_

// Includes
#include <propeller.h>
#include <stdio.h>
#include <PropWare.h>
#include <spi.h>

// Constants
/** Pin number for chip select */
#define CS                  BIT_6
/** Pin number for MOSI (master out - slave in) */
#define MOSI                BIT_0
/** Pin number for the clock signal */
#define SCLK                BIT_1
/** Pin number for MISO (master in - slave out) */
#define MISO                BIT_2

/** Frequency (in hertz) to run the SPI module */
#define FREQ                100000
/** The SPI mode to run */
#define MODE                SPI_MODE_0
/** Determine if the LSB or MSB should be sent first for each byte */
#define BITMODE             SPI_MSB_FIRST

#endif /* SPI_DEMO_H_ */
