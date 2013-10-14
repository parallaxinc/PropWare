/* File:    HD44780_Demo.h
 * 
 * Author:  David Zemon
 * Project: HD44780_Demo
 */

#ifndef HD44780_DEMO_H_
#define HD44780_DEMO_H_

#include <propeller.h>
#include <stdio.h>
#include <PropWare.h>
#include <hd44780.h>

#define RS					BIT_15
#define RW					BIT_13
#define EN					BIT_11

#define DATA_H				BIT_26 | BIT_25 | BIT_24 | BIT_23
#define DATA_L				BIT_22 | BIT_21 | BIT_20 | BIT_19

#ifdef DATA_L
#define BITMODE				HD44780_8BIT
#define DATA				DATA_H | DATA_L
#else
#define BITMODE				HD44780_4BIT
#define DATA				DATA_H
#endif
#define DIMENSIONS			HD44780_16x2

void error (const uint8_t err);

#endif /* HD44780_DEMO_H_ */
