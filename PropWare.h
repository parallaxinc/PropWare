/**
 * @file    PropWare.h
 *
 * @author  David Zemon
 */

#ifndef PROPWARE_H
#define PROPWARE_H

#ifndef ASM_OBJ_FILE
#include <propeller.h>
#endif

#ifdef DAREDEVIL
#define checkErrors(x)      x
#else
#define checkErrors(x)      if ((err = x)) return err
#endif

#define SECOND              ((unsigned long long) CLKFREQ)
#define MILLISECOND         ((unsigned long long) (CLKFREQ / 1000))
#define MICROSECOND         ((unsigned long long) (MILLISECOND / 1000))

#define BIT_0               0x1
#define BIT_1               0x2
#define BIT_2               0x4
#define BIT_3               0x8
#define BIT_4               0x10
#define BIT_5               0x20
#define BIT_6               0x40
#define BIT_7               0x80
#define BIT_8               0x100
#define BIT_9               0x200
#define BIT_10              0x400
#define BIT_11              0x800
#define BIT_12              0x1000
#define BIT_13              0x2000
#define BIT_14              0x4000
#define BIT_15              0x8000
#define BIT_16              0x10000
#define BIT_17              0x20000
#define BIT_18              0x40000
#define BIT_19              0x80000
#define BIT_20              0x100000
#define BIT_21              0x200000
#define BIT_22              0x400000
#define BIT_23              0x800000
#define BIT_24              0x1000000
#define BIT_25              0x2000000
#define BIT_26              0x4000000
#define BIT_27              0x8000000
#define BIT_28              0x10000000
#define BIT_29              0x20000000
#define BIT_30              0x40000000
#define BIT_31              0x80000000

#define NIBBLE_0            0xf
#define NIBBLE_1            0xf0
#define NIBBLE_2            0xf00
#define NIBBLE_3            0xf000
#define NIBBLE_4            0xf0000
#define NIBBLE_5            0xf00000
#define NIBBLE_6            0xf000000
#define NIBBLE_7            0xf0000000

#define BYTE_0              0xff
#define BYTE_1              0xff00
#define BYTE_2              0xff0000
#define BYTE_3              0xff000000

#define WORD_0              0xffff
#define WORD_1              0xffff0000

#define DEBOUNCE_DELAY      3

#define GPIO_DIR_IN         0
#define GPIO_DIR_OUT        -1

/* @brief: Set selected pins as either input or output
 *
 * @param    pins        bit mask to control which pins are set as input or output
 * @param    dir            I/O direction to set selected pins; must be one of
 *                         GPIO_DIR_IN or GPIO_DIR_OUT
 */
#define GPIODirModeSet(pins,dir)    DIRA = (DIRA & (~(pins))) | ((pins) & dir)

/* @brief: Set selected pins high
 *
 * @param    pins        Bit mask to control which pins will be set high
 */
#define GPIOPinSet(pins)            OUTA |= (pins)

/* @brief: Clear selected pins low
 *
 * @param    pins        Bit mask to control which pins will be cleared low
 */
#define GPIOPinClear(pins)          OUTA &= ~(pins)

/* @Brief: Allow easy write to a port w/o destroying data elsewhere in the port
 *
 * @param    pins        bit mask to control which pins will be written to
 * @param    value        value to be bit-masked and then written to the port
 */
#define GPIOPinWrite(pins,value)    OUTA = (OUTA & (~(pins))) | ((value) & (pins))

/* @brief: Allow easy write to a port w/o destroying data elsewhere in the port
 *
 * @param    port        Port # to write to (like 0, for P0 or 1 for P1)
 * @param    pin            pin to toggle
 */
#define GPIOPinToggle(pins)         OUTA ^= pins

/* @Brief: Allow easy reading of only selected pins from a port
 *
 */
#define GPIOPinRead(pins)           (INA & (pins))

#ifndef ASM_OBJ_FILE
/**
 * @brief   Allow easy switch-press detection of any pin with de-bounce
 *          protection
 * @note    "_Low" denotes active low switch-press
 *
 * @param   pin     bit mask to control which pin will be read
 *
 * @return  Returns 1 or 0 depending on whether the switch was pressed
 */
uint8_t GPIOSwitchRead_Low (uint32_t pin);

/**
 * TODO: Do me
 */
uint8_t PropWareCountBits (uint32_t par);

/**
 * TODO: Do me
 */
uint8_t PropWareGetPinNum (const uint32_t pinMask);
#endif
#endif /* PROPWARE_H */
