/* File:    SD_Demo.h
 * 
 * Author:  David Zemon
 * Project: SD_Demo
 */

#ifndef SD_DEMO_H_
#define SD_DEMO_H_

// Includes
#include <propeller.h>
#include <gpio.h>
#include <stdio.h>
#include <sd.h>

#define CS						BIT_0
#define MOSI					BIT_1
#define SCLK					BIT_2
#define MISO					BIT_3

void error (const uint8 err);

#endif /* SD_DEMO_H_ */
