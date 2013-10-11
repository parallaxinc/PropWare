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

#define RS					BIT_29
#define RW					BIT_28
#define EN					BIT_27

#define DATA				BIT_26 | BIT_25 | BIT_24 | BIT_23 | \
							BIT_22 | BIT_21 | BIT_20 | BIT_19

#define BITMODE				HD44780_8BIT
#define DIMENSIONS			HD44780_16x2

#endif /* HD44780_DEMO_H_ */
