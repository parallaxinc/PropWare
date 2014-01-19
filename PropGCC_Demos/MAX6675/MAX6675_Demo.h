/**
 * @file    MAX6675_Demo.h
 *
 * @author  David Zemon
 * @project MAX6675_Demo
 */

#ifndef MAX6675_DEMO_H_
#define MAX6675_DEMO_H_

#include <stdio.h>
#include <PropWare.h>
#include <spi.h>
#include <hd44780.h>
#include <max6675.h>

#define MOSI            BIT_0
#define MISO            BIT_1
#define SCLK            BIT_2
#define CS              BIT_5
#define FREQ            10000

#define DEBUG_LEDS      BYTE_2

#define RS              BIT_14
#define RW              BIT_12
#define EN              BIT_10

#define DATA_H          BIT_26 | BIT_25 | BIT_24 | BIT_23
#define DATA_L          BIT_22 | BIT_21 | BIT_20 | BIT_19

#ifdef DATA_L
#define BITMODE         HD44780_8BIT
#define DATA            DATA_H | DATA_L
#else
#define BITMODE         HD44780_4BIT
#define DATA            DATA_H
#endif

#define DIMENSIONS      HD44780_16x2

void error (int8_t err);

#endif /* MAX6675_DEMO_H_ */
