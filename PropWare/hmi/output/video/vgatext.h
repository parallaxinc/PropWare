// ***************************************
// *  VGA Text 32x15 v1.0                *
// *  Author: Chip Gracey                *
// *  Copyright (c) 2006 Parallax, Inc.  *
// *  See end of file for terms of use.  *
// ***************************************

#pragma once

#include <PropWare/hmi/output/video/vga.h>

namespace PropWare {

class VGAText {
public:
    static const int cols = 32;
    static const int rows = 15;
    static const int screensize = (cols * rows);
    static const int lastrow = (screensize - cols);

    /**
     * Start terminal - starts a cog
     * returns false if no cog available
     *
     * requires at least 80MHz system clock
     */
    int32_t start(int32_t basepin);

    /**
     * Stop terminal - frees a cog
     */
    void stop();

    /**
     * Print a zero-terminated string
     */
    void str(const char *stringptr);

    /**
     * Print a decimal number
     */
    void dec(int32_t value);

    /**
     * Print a hexadecimal number
     */
    void hex(int32_t value, int32_t digits);

    /**
     * Print a binary number
     */
    void bin(int32_t value, int32_t digits);

    /**
     * Output a character
     *
     *     $00 = clear screen
     *     $01 = home
     *     $08 = backspace
     *     $09 = tab (8 spaces per)
     *     $0A = set X position (X follows)
     *     $0B = set Y position (Y follows)
     *     $0C = set color (color follows)
     *     $0D = return
     *     others = printable characters
     */
    int32_t out(char c);

    /**
     * Override default color palette
     *
     * colorptr must point to a list of up to 8 colors
     * arranged as follows (where r, g, b are 0..3):
     *
     *               fore   back
     *               ------------
     * palette  byte %%rgb, %%rgb     'color 0
     *          byte %%rgb, %%rgb     'color 1
     *          byte %%rgb, %%rgb     'color 2
     */
    void setcolors(uint8_t *colorptr);

private:
    VGA vga;

    volatile int32_t col, row, color, flag;
    volatile int32_t colors[16];
    volatile uint16_t screen[480];

    void print(int32_t c);

    void newline();
};

}

/*
 ┌──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
 │                                                   TERMS OF USE: MIT License                                                  │
 ├──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┤
 │Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation    │
 │files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,    │
 │modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software│
 │is furnished to do so, subject to the following conditions:                                                                   │
 │                                                                                                                              │
 │The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.│
 │                                                                                                                              │
 │THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE          │
 │WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR         │
 │COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,   │
 │ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                         │
 └──────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
 */
