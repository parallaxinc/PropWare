/* File:    SPI_Demo.h
 * 
 * Author:  David Zemon
 * Project: SPI_Demo
 */

#ifndef SPI_DEMO_H_
#define SPI_DEMO_H_

// Includes
#include <propeller.h>
#include <stdio.h>
#include <PropWare.h>
#include <gpio.h>
#include <spi.h>

// Constants
#define CS						BIT_6
#define MOSI					BIT_1
#define SCLK					BIT_2
#define MISO					BIT_3

#define FREQ					100000
#define MODE					SPI_MODE_0
#define BITMODE					SPI_MSB_FIRST
#endif /* SPI_DEMO_H_ */
