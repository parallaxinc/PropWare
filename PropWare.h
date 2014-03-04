/**
 * @file    PropWare.h
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


#ifdef ASM_OBJ_FILE
#include <PropWare_asm.h>
#endif

#ifndef PROPWARE_H
#define PROPWARE_H

#include <propeller.h>
#include <stdint.h>

/**
 * @brief   Generic definitions and functions for the Parallax Propeller
 */
namespace PropWare {

#ifdef DAREDEVIL
#define check_errors(x)      x
#else
#define check_errors(x)      if ((err = x)) return err
#endif

#define SECOND              ((unsigned long long) CLKFREQ)
#define MILLISECOND         ((unsigned long long) (CLKFREQ / 1000))
#define MICROSECOND         ((unsigned long long) (MILLISECOND / 1000))

typedef int8_t ErrorCode;

typedef enum {
    BIT_0 = 0x1,
    BIT_1 = 0x2,
    BIT_2 = 0x4,
    BIT_3 = 0x8,
    BIT_4 = 0x10,
    BIT_5 = 0x20,
    BIT_6 = 0x40,
    BIT_7 = 0x80,
    BIT_8 = 0x100,
    BIT_9 = 0x200,
    BIT_10 = 0x400,
    BIT_11 = 0x800,
    BIT_12 = 0x1000,
    BIT_13 = 0x2000,
    BIT_14 = 0x4000,
    BIT_15 = 0x8000,
    BIT_16 = 0x10000,
    BIT_17 = 0x20000,
    BIT_18 = 0x40000,
    BIT_19 = 0x80000,
    BIT_20 = 0x100000,
    BIT_21 = 0x200000,
    BIT_22 = 0x400000,
    BIT_23 = 0x800000,
    BIT_24 = 0x1000000,
    BIT_25 = 0x2000000,
    BIT_26 = 0x4000000,
    BIT_27 = 0x8000000,
    BIT_28 = 0x10000000,
    BIT_29 = 0x20000000,
    BIT_30 = 0x40000000,
    BIT_31 = 0x80000000
} Bit;

typedef enum {
    NIBBLE_0 = 0xf,
    NIBBLE_1 = 0xf0,
    NIBBLE_2 = 0xf00,
    NIBBLE_3 = 0xf000,
    NIBBLE_4 = 0xf0000,
    NIBBLE_5 = 0xf00000,
    NIBBLE_6 = 0xf000000,
    NIBBLE_7 = 0xf0000000
} Nibble;

typedef enum {
    BYTE_0 = 0xff,
    BYTE_1 = 0xff00,
    BYTE_2 = 0xff0000,
    BYTE_3 = 0xff000000
} Byte;

typedef enum {
    WORD_0 = 0xffff,
    WORD_1 = 0xffff0000
} Word;

/**
 * @brief   General purpose I/O functions and definitions
 */
namespace GPIO {

/** Number of milliseconds to delay during debounce */
#define DEBOUNCE_DELAY      3

/**
 * Bit-mask of GPIO pins
 */
typedef enum {
    /** GPIO pin 0  */P0 = BIT_0,
    /** GPIO pin 1  */P1 = BIT_1,
    /** GPIO pin 2  */P2 = BIT_2,
    /** GPIO pin 3  */P3 = BIT_3,
    /** GPIO pin 4  */P4 = BIT_4,
    /** GPIO pin 5  */P5 = BIT_5,
    /** GPIO pin 6  */P6 = BIT_6,
    /** GPIO pin 7  */P7 = BIT_7,
    /** GPIO pin 8  */P8 = BIT_8,
    /** GPIO pin 9  */P9 = BIT_9,
    /** GPIO pin 10 */P10 = BIT_10,
    /** GPIO pin 11 */P11 = BIT_11,
    /** GPIO pin 12 */P12 = BIT_12,
    /** GPIO pin 13 */P13 = BIT_13,
    /** GPIO pin 14 */P14 = BIT_14,
    /** GPIO pin 15 */P15 = BIT_15,
    /** GPIO pin 16 */P16 = BIT_16,
    /** GPIO pin 17 */P17 = BIT_17,
    /** GPIO pin 18 */P18 = BIT_18,
    /** GPIO pin 19 */P19 = BIT_19,
    /** GPIO pin 20 */P20 = BIT_20,
    /** GPIO pin 21 */P21 = BIT_21,
    /** GPIO pin 22 */P22 = BIT_22,
    /** GPIO pin 23 */P23 = BIT_23,
    /** GPIO pin 24 */P24 = BIT_24,
    /** GPIO pin 25 */P25 = BIT_25,
    /** GPIO pin 26 */P26 = BIT_26,
    /** GPIO pin 27 */P27 = BIT_27,
    /** GPIO pin 28 */P28 = BIT_28,
    /** GPIO pin 29 */P29 = BIT_29,
    /** GPIO pin 30 */P30 = BIT_30,
    /** GPIO pin 31 */P31 = BIT_31
} Pin;

/**
 * Direction of GPIO pin
 */
typedef enum {
    /** Set pin(s) as input */IN = 0,
    /** Set pin(s) as output */OUT = -1
} Dir;

/**
 * @brief       Set selected pins as either input or output
 *
 * @param[in]   pins        bit mask to control which pins are set as input or
 *                          output
 * @param[in]   dir         I/O direction to set selected pins; must be one of
 *                          GPIO_DIR_IN or GPIO_DIR_OUT
 */
void set_dir (const uint32_t pins, GPIO::Dir dir);

/**
 * @brief       Set selected pins high
 *
 * @param[in]   pins        Bit mask to control which pins will be set high
 */
void pin_set (const uint32_t pins);

/**
 * @brief       Clear selected output pins (set them 0)
 *
 * @param[in]   pins    Bit mask to control which pins will be cleared low
 */
void pin_clear (const uint32_t pins);

/**
 * @brief       Allow easy write to a port w/o destroying data elsewhere in the port
 *
 * @param[in]   port    Port # to write to (like 0, for P0 or 1 for P1)
 * @param[in]   pin     pin to toggle
 */
void pin_toggle (const uint32_t pins);

/**
 * @brief       Allow easy write to a port w/o destroying data elsewhere in the port
 *
 * @param[in]   pins    bit mask to control which pins will be written to
 * @param[in]   value   value to be bit-masked and then written to the port
 */
void pin_write (const uint32_t pins, const uint32_t value);

/**
 * @brief       Read the value from a single pin and return its state
 *
 * @param[in]   pin     A single pin to read
 *
 * @return      True if the pin is high, False if the pin is low
 */
bool read_pin (const GPIO::Pin pin);

/**
 * @brief       Allow easy reading of only selected pins from a port
 *
 * @param[in]   pins    Bit mask to control which pins will be read from the
 *                      input register, INA
 *
 * @return      Value of INA masked by `pins` parameter
 */
uint32_t read_multi_pin (const uint32_t pins);

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
bool read_switch_low (const GPIO::Pin pin);

}

/**
 * @brief       Count the number of set bits in a parameter
 *
 * @param[in]   par     Parameter whose bits should be counted
 *
 * @return      Number of bits that are non-zero in par
 */
uint8_t count_bits (uint32_t par);

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
uint8_t get_pin_num (const uint32_t pinMask);

/**
 * @brief       Copy a string into another buffer
 *
 * @param[out]  *dest    Destination
 * @param[in]   *src     Source string
 */
void strcpy (char *dest, char *src);
}

#endif /* PROPWARE_H */
