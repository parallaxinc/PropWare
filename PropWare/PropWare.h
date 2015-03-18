/**
 * @file    PropWare.h
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

#pragma once

#ifdef ASM_OBJ_FILE
#include <PropWare/PropWare_asm.h>
#else

#include <propeller.h>
#include <cstdint>
#include <cstdlib>
#include <cctype>

/**
 * @brief   Generic definitions and functions for the Parallax Propeller
 */
namespace PropWare {

#ifdef DAREDEVIL
#define check_errors(x)      x
#else
#define check_errors(x)     if ((err = x)) return err
#endif

#define SECOND              ((uint32_t) CLKFREQ)
#define MILLISECOND         ((uint32_t) (CLKFREQ / 1000))
#define MICROSECOND         ((uint32_t) (MILLISECOND / 1000))

typedef int16_t ErrorCode;

typedef enum {
    BIT_0  = 0x1,
    BIT_1  = 0x2,
    BIT_2  = 0x4,
    BIT_3  = 0x8,
    BIT_4  = 0x10,
    BIT_5  = 0x20,
    BIT_6  = 0x40,
    BIT_7  = 0x80,
    BIT_8  = 0x100,
    BIT_9  = 0x200,
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

}

#endif /* ifdef ASM_OBJ_FILE */
