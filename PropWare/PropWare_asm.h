/**
 * @file    PropWare/PropWare_asm.h
 *
 * @brief   Generic definitions for assembly files for the Parallax Propeller
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

#if (defined __cplusplus)
#error "This file is for assembly files only; Please use PropWare.h instead"
#endif

#pragma once

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

#define GPIO_DIR_IN         0
#define GPIO_DIR_OUT        -1
