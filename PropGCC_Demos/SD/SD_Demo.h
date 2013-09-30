/* File:    SD_Demo.h
 * 
 * Author:  David Zemon
 * Project: SD_Demo
 */

#ifndef SD_DEMO_H_
#define SD_DEMO_H_

#define DEBUG
//#define LOW_RAM_MODE

// Includes
#include <propeller.h>
#include <PropWare.h>
#include <gpio.h>
#include <sd.h>

#ifdef DEBUG
#include <stdio.h>
#endif

#if (!(defined SD_VERBOSE) && !(defined SD_VERBOSE_BLOCKS))
#define printf		__simple_printf
#endif

#define CS						BIT_0
#define MOSI					BIT_1
#define SCLK					BIT_2
#define MISO					BIT_3

#define NEW_FILE				"TEST.TXT"

void error (const uint8 err);

#endif /* SD_DEMO_H_ */
