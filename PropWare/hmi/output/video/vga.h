/**
 * @file vga.h
 * @author Chip Gracey
 *
 * @copyright
 * The MIT License (MIT)<br>
 * <br>Copyright (c) 2006 Parallax, Inc.<br>
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
 *
 * v1.0 - 01 May 2006 - original version
 * v1.1 - 15 May 2006 - pixel tile size can now be 16 x 32 to enable more efficient
 *                       character displays utilizing the internal font - see 'vga_mode'
 */

#pragma once

#include <stdint.h>
#include <propeller.h>

extern "C" {
extern int _VGADriverStartCog (void *arg);
}

namespace PropWare {

/**
 * VGA Driver v1.1
 */
class VGA {
public:
    /*
     * VGA parameters - 21 contiguous longs
     */
    struct {
        uint32_t vga_status;   // 0/1/2 = off/visible/invisible      read-only
        uint32_t vga_enable;   // 0/non-0 = off/on                   write-only
        uint32_t vga_pins;     // %pppttt = pins                     write-only
        uint32_t vga_mode;     // %tihv = tile,interlace,hpol,vpol   write-only
        uint32_t vga_screen;   // pointer to screen (words)          write-only
        uint32_t vga_colors;   // pointer to colors (longs)          write-only
        uint32_t vga_ht;       // horizontal tiles                   write-only
        uint32_t vga_vt;       // vertical tiles                     write-only
        uint32_t vga_hx;       // horizontal tile expansion          write-only
        uint32_t vga_vx;       // vertical tile expansion            write-only
        uint32_t vga_ho;       // horizontal offset                  write-only
        uint32_t vga_vo;       // vertical offset                    write-only
        uint32_t vga_hd;       // horizontal display ticks           write-only
        uint32_t vga_hf;       // horizontal front porch ticks       write-only
        uint32_t vga_hs;       // horizontal sync ticks              write-only
        uint32_t vga_hb;       // horizontal back porch ticks        write-only
        uint32_t vga_vd;       // vertical display lines             write-only
        uint32_t vga_vf;       // vertical front porch lines         write-only
        uint32_t vga_vs;       // vertical sync lines                write-only
        uint32_t vga_vb;       // vertical back porch lines          write-only
        uint32_t vga_rate;     // tick rate (Hz)                     write-only
    } params;

    /*
     * Start VGA driver - starts a cog
     * returns false if no cog available
     *
     *   vgaptr = pointer to VGA parameters
     */
    int32_t start() {
        int32_t okay = 0;
        stop();
        okay = (cog = _VGADriverStartCog(&params) + 1);
        return okay;
    }

    /*
     * Stop VGA driver - frees a cog
     */
    int32_t stop() {
#define PostEffect__(X, Y) __extension__({ int32_t tmp__ = (X); (X) = (Y); tmp__; })
        if (cog) {
            cogstop((PostEffect__(cog, 0) - 1));
        }
        return 0;
    }

private:
    int32_t cog;
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
