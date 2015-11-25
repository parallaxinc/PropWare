// ***************************************
// *  VGA Text 32x15 v1.0                *
// *  Author: Chip Gracey                *
// *  Copyright (c) 2006 Parallax, Inc.  *
// *  See end of file for terms of use.  *
// ***************************************

#include <propeller.h>
#include <PropWare/hmi/output/video/vgatext.h>

#define INLINE__ static inline
#define PostEffect__(X, Y) __extension__({ int32_t tmp__ = (X); (X) = (Y); tmp__; })

namespace PropWare {

static inline int32_t Rotl__ (uint32_t a, uint32_t b) {
    return (a << b) | (a >> (32 - b));
}

static inline int32_t Rotr__ (uint32_t a, uint32_t b) {
    return (a >> b) | (a << (32 - b));
}

static inline int32_t Between__ (int32_t x, int32_t a, int32_t b) {
    if (a <= b)
        return x >= a && x <= b;
    return x >= b && x <= a;
}

static inline int32_t Lookup__ (int32_t x, int32_t b, int32_t a[], int32_t n) {
    int32_t i = (x) - (b);
    return ((unsigned) i >= n) ? 0 : (a)[i];
}

/**
 * Default color palette
 */
static uint8_t palette[] = {
        0x3f, 0x01,    // 0    white / dark blue
        0x3c, 0x14,    // 1   yellow / brown
        0x22, 0x00,    // 2  magenta / black
        0x15, 0x3f,    // 3     grey / white
        0x0f, 0x05,    // 4     cyan / dark cyan
        0x08, 0x2e,    // 5    green / gray-green
        0x10, 0x35,    // 6      red / pink
        0x0f, 0x03     // 7     cyan / blue
};

int32_t VGAText::start (int32_t basepin) {
    int32_t okay = 0;

    setcolors(palette);
    out(0);

    vga.params.vga_status = 0;   // 0/1/2 = off/visible/invisible      read-only
    vga.params.vga_enable = 1;   // 0/non-0 = off/on                   write-only
    vga.params.vga_pins   = basepin | 0x7;    // %pppttt = pins                     write-only
    vga.params.vga_mode   = 0b1000; // %tihv = tile,interlace,hpol,vpol   write-only
    vga.params.vga_screen = (int32_t) &screen[0];   // pointer to screen (words)          write-only
    vga.params.vga_colors = (int32_t) &colors[0];   // pointer to colors (longs)          write-only
    vga.params.vga_ht     = cols;    // horizontal tiles                   write-only
    vga.params.vga_vt     = rows;    // vertical tiles                     write-only
    vga.params.vga_hx     = 1;       // horizontal tile expansion          write-only
    vga.params.vga_vx     = 1;       // vertical tile expansion            write-only
    vga.params.vga_ho     = 0;       // horizontal offset                  write-only
    vga.params.vga_vo     = 0;       // vertical offset                    write-only
    vga.params.vga_hd     = 512;     // horizontal display ticks           write-only
    vga.params.vga_hf     = 10;      // horizontal front porch ticks       write-only
    vga.params.vga_hs     = 75;      // horizontal sync ticks              write-only
    vga.params.vga_hb     = 43;      // horizontal back porch ticks        write-only
    vga.params.vga_vd     = 480;     // vertical display lines             write-only
    vga.params.vga_vf     = 11;      // vertical front porch lines         write-only
    vga.params.vga_vs     = 2;       // vertical sync lines                write-only
    vga.params.vga_vb     = 31;      // vertical back porch lines          write-only
    vga.params.vga_rate   = CLKFREQ >> 2; // tick rate (Hz)                     write-only

    okay = vga.start();

    return okay;
}

void VGAText::stop (void) {
    vga.stop();
}

void VGAText::str (const char *stringptr) {
    while (*stringptr != 0) {
        out(*stringptr++);
    }
}

void VGAText::dec (int32_t value) {
    int32_t i, idx;
    int32_t result = 0;

    if (value < 0) {
        value = -value;
        out('-');
    }
    i              = 1000000000;
    for (idx       = 1; idx <= 10; idx = idx + 1) {
        if (value >= i) {
            out(((value / i) + '0'));
            value  = value % i;
            result = -1;
        } else {
            if ((result) || (i == 1)) {
                out('0');
            }
        }
        i = i / 10;
    }
}

void VGAText::hex (int32_t value, int32_t digits) {
    int32_t        idx;
    int32_t        limit        = digits;
    static int32_t look__0003[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    value    = value << ((8 - digits) << 2);
    for (idx = 1; idx <= limit; idx = idx + 1) {
        out(Lookup__(((value = Rotl__(value, 4)) & 0xf), 0, look__0003, 16));
    }
}

void VGAText::bin (int32_t value, int32_t digits) {
    int32_t idx;
    int32_t limit = digits;

    value    = value << (32 - digits);
    for (idx = 1; idx <= limit; idx = idx + 1) {
        out((((value = Rotl__(value, 1)) & 0x1) + '0'));
    }
}

int32_t VGAText::out (char c) {
    int32_t result = 0;

    if (flag == 0) {
        if (c == 0) {
            int32_t  fill;
            uint16_t *ptr = (uint16_t *) &screen[0];
            uint16_t val  = 0x220;
            for (fill = screensize; fill > 0; --fill) {
                *ptr++ = val;
            }
            col = (row = 0);
        } else if (c == 1) {
            col = (row = 0);
        } else if (c == 8) {
            if (col) {
                (col--);
            }
        } else if (c == 9) {
            do {
                print(' ');
            } while (col & 0x7);
        } else if (Between__(c, 10, 12)) {
            flag = c;
            return result;
        } else if (c == 13) {
            newline();
        } else if (1) {
            print(c);
        }
    } else if (flag == 10) {
        col = c % cols;
    } else if (flag == 11) {
        row = c % rows;
    } else if (flag == 12) {
        color = c & 0x7;
    }
    flag           = 0;
    return result;
}

void VGAText::setcolors (uint8_t *colorptr) {
    int32_t i, fore, back;

    for (i = 0; i <= 7; i = i + 1) {
        fore = (colorptr)[(i << 1)] << 2;
        back = (colorptr)[((i << 1) + 1)] << 2;
        colors[(i << 1)]       = (((fore << 24) + (back << 16)) + (fore << 8)) + back;
        colors[((i << 1) + 1)] = (((fore << 24) + (fore << 16)) + (back << 8)) + back;
    }
}

void VGAText::print (int32_t c) {
    screen[((row * cols) + col)] = ((((color << 1) + (c & 0x1)) << 10) + 512) + (c & 0xfe);
    if ((++col) == cols) {
        newline();
    }
}

void VGAText::newline (void) {
    col = 0;
    if ((++row) == rows) {
        (row--);
        // scroll lines
        memmove((void *) &screen[0], (void *) &screen[cols], 2 * (lastrow));
        // clear new line
        {
            int32_t  fill;
            uint16_t *ptr = (uint16_t *) &screen[lastrow];
            uint16_t val  = 0x220;
            for (fill     = cols; fill > 0; --fill) {
                *ptr++ = val;
            }
        };
    }
}

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
