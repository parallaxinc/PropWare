/* File:    gpio.h
 *
 * Author:  David Zemon
 */

#ifndef __GPIO_H
#define __GPIO_H

#include <propeller.h>
#include <types.h>
#include <bit_expansion.h>

#define DEBOUNCE_DELAY		3

#define GPIO_DIR_IN			0
#define GPIO_DIR_OUT		-1

/* @Brief: Set selected pins as either input or output
 *
 * @param	pins		bit mask to control which pins are set as input or output
 * @param	dir			I/O direction to set selected pins; must be one of
 * 						GPIO_DIR_IN or GPIO_DIR_OUT
 */
#define GPIODirModeSet(pins,dir)		DIRA = (DIRA & (~(pins))) | ((pins) & dir)

/* @Brief: Set selected pins high
 *
 * @param	pins		Bit mask to control which pins will be set high
 */
#define GPIOPinSet(pins)				OUTA |= (pins)

/* @Brief: Clear selected pins low
 *
 * @param	pins		Bit mask to control which pins will be cleared low
 */
#define GPIOPinClear(pins)				OUTA &= ~(pins)

/* @Brief: Allow easy write to a port w/o destroying data elsewhere in the port
 *
 * @param	pins		bit mask to control which pins will be written to
 * @param	value		value to be bit-masked and then written to the port
 */
#define GPIOPinWrite(pins,value)		OUTA = (OUTA & (~(pins))) | ((value) & (pins))

/* @Brief: Allow easy write to a port w/o destroying data elsewhere in the port
 *
 * @param	port		Port # to write to (like 0, for P0 or 1 for P1)
 * @param	pin			pin to toggle
 */
#define GPIOPinToggle(pins)				OUTA ^= pins

/* @Brief: Allow easy reading of only selected pins from a port
 *
 */
#define GPIOPinRead(pins)				return (INA & (pins))

/* @Brief: Allow easy switch-press detection of any port and pin with debounce
 *				protection
 *				NOTE: "_Low" denotes active low switch-press
 *
 * @param	pin			bit mask to control which pin will be read
 *
 * @return		Returns 1 or 0 depending on whether the switch was pressed
 */
uint8 GPIOSwitchRead_Low (uint32 pin);

#endif /* __GPIO_H */
