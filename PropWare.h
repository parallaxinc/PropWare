/**
 * @file    PropWare.h
 */
/**
 * @brief   Generic definitions and functions for the Parallax Propeller
 *
 * @project PropWare
 *
 * @author  David Zemon
 *
 * @copyright
 * The MIT License (MIT)<br>
 * <br>Copyright (c) 2013 David Zemon<br>
 * <br>Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:<br>
 * <br>The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.<br>
 * <br>THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PROPWARE_H
#define PROPWARE_H

/**
 * @defgroup    _propware   PropWare
 * @{
 */

#ifndef ASM_OBJ_FILE
#include <propeller.h>
#include <stdint.h>
#endif

#ifdef DAREDEVIL
#define check_errors(x)      x
#else
#define check_errors(x)      if ((err = x)) return err
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

/**
 * @brief       Set selected pins as either input or output
 *
 * @param[in]   pins        bit mask to control which pins are set as input or
 *                          output
 * @param[in]   dir         I/O direction to set selected pins; must be one of
 *                          GPIO_DIR_IN or GPIO_DIR_OUT
 */
#define gpio_set_dir(pins,dir)    DIRA = (DIRA & (~(pins))) | ((pins) & dir)

/**
 * @brief       Set selected pins high
 *
 * @param[in]   pins        Bit mask to control which pins will be set high
 */
#define gpio_pin_set(pins)            OUTA |= (pins)

/**
 * @brief       Clear selected output pins (set them 0)
 *
 * @param[in]   pins    Bit mask to control which pins will be cleared low
 */
#define gpio_pin_clear(pins)          OUTA &= ~(pins)

/**
 * @brief       Allow easy write to a port w/o destroying data elsewhere in the port
 *
 * @param[in]   pins    bit mask to control which pins will be written to
 * @param[in]   value   value to be bit-masked and then written to the port
 */
#define gpio_pin_write(pins,value)    \
    OUTA = (OUTA & (~(pins))) | ((value) & (pins))

/**
 * @brief       Allow easy write to a port w/o destroying data elsewhere in the port
 *
 * @param[in]   port    Port # to write to (like 0, for P0 or 1 for P1)
 * @param[in]   pin     pin to toggle
 */
#define gpio_pin_toggle(pins)         OUTA ^= pins

/**
 * @brief       Allow easy reading of only selected pins from a port
 *
 * @param[in]   pins    Bit mask to control which pins will be read from the
 *                      input register, INA
 *
 * @return      Value of INA masked by `pins` parameter
 */
#define gpio_pin_read(pins)           (INA & (pins))

#ifndef ASM_OBJ_FILE
/**
 * @brief       Allow easy switch-press detection of any pin; Includes de-bounce
 *              protection
 *
 * @note        "_Low" denotes active low switch-press
 *
 * @param[in]   pin     bit mask to control which pin will be read
 *
 * @return      Returns 1 or 0 depending on whether the switch was pressed
 */
uint8_t gpio_read_switch_low (const uint32_t pin);

/**
 * @brief       Count the number of set bits in a parameter
 *
 * @param[in]   par     Parameter whose bits should be counted
 *
 * @return      Number of bits that are non-zero in par
 */
uint8_t propware_count_bits (uint32_t par);

/**
 * @brief       Determine which pin number based on a pin mask
 *
 * @note        Return value is 0-indexed
 *
 * @param[in]   pinMask     Value with only a single bit set high representing
 *                          Propeller pin (i.e.: 0x80 would be pin 7)
 *
 * @return      Return the pin number of pinMask
 */
uint8_t propware_get_pin_num (const uint32_t pinMask);
#endif

/**@}*/

#endif /* PROPWARE_H */
