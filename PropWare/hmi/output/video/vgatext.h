/**
 * @file vgatext.h
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
 */

#pragma once

#include <PropWare/hmi/output/video/vga.h>
#include <PropWare/hmi/output/printcapable.h>
#include <PropWare/utility/utility.h>

namespace PropWare {

class VGAText : public PrintCapable {
    public:
        static const int COLUMNS = 32;
        static const int ROWS = 15;
        static const int SCREEN_SIZE = (COLUMNS * ROWS);
        static const int LAST_ROW = (SCREEN_SIZE - COLUMNS);

        /**
         * Start terminal - starts a cog
         * returns false if no cog available
         *
         * requires at least 80MHz system clock
         */
        int32_t start(const int32_t basepin) {
            // Default color palette
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

            this->set_colors(palette);
            this->put_char(0);

            this->m_vga.params.vga_status = 0;   // 0/1/2 = off/visible/invisible      read-only
            this->m_vga.params.vga_enable = 1;   // 0/non-0 = off/on                   write-only
            this->m_vga.params.vga_pins = basepin | 0x7;    // %pppttt = pins                     write-only
            this->m_vga.params.vga_mode = 0b1000; // %tihv = tile,interlace,hpol,vpol   write-only
            this->m_vga.params.vga_screen = (uint32_t) &m_screen[0];   // pointer to screen (words)          write-only
            this->m_vga.params.vga_colors = (uint32_t) &m_colors[0];   // pointer to colors (longs)          write-only
            this->m_vga.params.vga_ht = COLUMNS;    // horizontal tiles                   write-only
            this->m_vga.params.vga_vt = ROWS;    // vertical tiles                     write-only
            this->m_vga.params.vga_hx = 1;       // horizontal tile expansion          write-only
            this->m_vga.params.vga_vx = 1;       // vertical tile expansion            write-only
            this->m_vga.params.vga_ho = 0;       // horizontal offset                  write-only
            this->m_vga.params.vga_vo = 0;       // vertical offset                    write-only
            this->m_vga.params.vga_hd = 512;     // horizontal display ticks           write-only
            this->m_vga.params.vga_hf = 10;      // horizontal front porch ticks       write-only
            this->m_vga.params.vga_hs = 75;      // horizontal sync ticks              write-only
            this->m_vga.params.vga_hb = 43;      // horizontal back porch ticks        write-only
            this->m_vga.params.vga_vd = 480;     // vertical display lines             write-only
            this->m_vga.params.vga_vf = 11;      // vertical front porch lines         write-only
            this->m_vga.params.vga_vs = 2;       // vertical sync lines                write-only
            this->m_vga.params.vga_vb = 31;      // vertical back porch lines          write-only
            this->m_vga.params.vga_rate = CLKFREQ >> 2; // tick rate (Hz)                     write-only

            return this->m_vga.start();
        }

        /**
         * Stop terminal - frees a cog
         */
        void stop() {
            this->m_vga.stop();
        }

        /**
         * Print a zero-terminated string
         */
        void puts(const char *string) {
            while (*string)
                this->put_char(*string++);
        }

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
        void put_char(const char c) {
            if (this->m_flag == 0) {
                if (c == 0) {
                    int32_t fill;
                    uint16_t *ptr = (uint16_t *) &this->m_screen[0];
                    uint16_t val = 0x220;
                    for (fill = SCREEN_SIZE; fill > 0; --fill) {
                        *ptr++ = val;
                    }
                    this->m_col = (this->m_row = 0);
                } else if (c == 1)
                    this->m_col = (this->m_row = 0);
                else if (c == 8) {
                    if (this->m_col)
                        this->m_col--;
                } else if (c == 9)
                    do {
                        print(' ');
                    } while (this->m_col & 0x7);
                else if (Utility::between(c, 10, 12)) {
                    this->m_flag = c;
                    return;
                } else if (c == 13)
                    newline();
                else
                    print(c);
            } else if (this->m_flag == 10) {
                this->m_col = c % COLUMNS;
            } else if (this->m_flag == 11) {
                this->m_row = c % ROWS;
            } else if (this->m_flag == 12) {
                this->m_color = c & 0x7;
            }
            this->m_flag = 0;
        }

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
        void set_colors(const uint8_t *colorptr) {
            int32_t i, fore, back;

            for (i = 0; i <= 7; i = i + 1) {
                fore = (colorptr)[(i << 1)] << 2;
                back = (colorptr)[((i << 1) + 1)] << 2;
                this->m_colors[(i << 1)] = (((fore << 24) + (back << 16)) + (fore << 8)) + back;
                this->m_colors[((i << 1) + 1)] = (((fore << 24) + (fore << 16)) + (back << 8)) + back;
            }
        }

    private:
        void print(int32_t c) {
            m_screen[((m_row * COLUMNS) + m_col)] = ((((m_color << 1) + (c & 0x1)) << 10) + 512) + (c & 0xfe);
            if ((++m_col) == COLUMNS) {
                newline();
            }
        }

        void newline() {
            this->m_col = 0;
            if ((++this->m_row) == ROWS) {
                (this->m_row--);
                // scroll lines
                memmove((void *) &this->m_screen[0], (void *) &this->m_screen[COLUMNS], (size_t) (2 * (LAST_ROW)));
                // clear new line
                {
                    int32_t fill;
                    uint16_t *ptr = (uint16_t *) &this->m_screen[LAST_ROW];
                    uint16_t val = 0x220;
                    for (fill = COLUMNS; fill > 0; --fill) {
                        *ptr++ = val;
                    }
                };
            }
        }

    private:
        VGA m_vga;

        volatile int32_t m_col, m_row, m_color, m_flag;
        volatile int32_t m_colors[16];
        volatile uint16_t m_screen[480];
};

}
