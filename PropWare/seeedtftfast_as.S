/**
 * @file    seeedtft_as.S
 *
 * @brief   SeeedStudio Touchscreen TFT Driver
 *
 * @project PropWare
 *
 * @author  localroger
 *          This code is more or less directly translated from the Arduino
 *          source, which was adapted for the Propeller ASC by Jazzed.
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

#define ASM_OBJ_FILE
#include <PropWare/PropWare.h>

#define pmb_idle      0
#define pmb_sendCMD   1
#define pmb_sendDATA  2
#define pmb_repeat    3

' PAR passes the address of the Long mailbox.
'
' Mailbox = 0 = idle
' Low byte mailbox determines function

              .section seeedtft_as.cog, "ax"
              .compress off
              .pasm

              org 0

TFT_PASM_IMAGE
             '
              'InitStart
              '
              mov       outa,#0
              mov       dira,all_ctl_mask

tpi_clear     mov       tmp, par
              wrlong    zero, tmp
              '
tpi_loop      mov       tmp,par
              rdlong    pmb_data, tmp
              test      pmb_data, pmb_data wz
        if_z  jmp       #tpi_loop

              mov       tmp,pmb_data
              and       tmp,#$FF

              cmp       tmp,#pmb_repeat wz
        if_z  shr       pmb_data,#8
        if_z  mov       rcount, pmb_data
        if_z  jmp       #tpi_clear

              cmp       tmp,#pmb_sendCMD wz
        if_nz jmp       #tpi_notCMD
              '
dloop         andn      outa,cs_bit_mask
              andn      outa,rs_bit_mask
              or        outa,rd_bit_mask
              or        outa,wr_bit_mask
              andn      outa,wr_bit_mask

              andn      outa,all_pin_mask
              'or with zero is free

              or        outa,wr_bit_mask
              andn      outa,wr_bit_mask

              mov       tmp,pmb_data
              shr       tmp,#6
              and       tmp,all_pin_mask
              andn      outa,all_pin_mask
              or        outa,tmp
              or        outa,wr_bit_mask
              or        outa,cs_bit_mask
              jmp       #tpi_clear

tpi_notCMD    cmp       tmp,#pmb_sendDATA wz
        if_nz jmp       #tpi_clear

:loop         andn      outa,cs_bit_mask
              or        outa,rs_bit_mask
              or        outa,rd_bit_mask

              andn      outa,wr_bit_mask
'
              andn      outa,all_pin_mask
              mov       tmp,pmb_data
              shr       tmp,#14
              and       tmp,all_pin_mask
              or        outa,tmp

              or        outa,wr_bit_mask

              andn      outa,wr_bit_mask
              andn      outa,all_pin_mask
              mov       tmp,pmb_data
              shr       tmp,#6
              and       tmp,all_pin_mask
              or        outa,tmp
              or        outa,wr_bit_mask
              or        outa,cs_bit_mask

              test      rcount, rcount wz
        if_z  jmp       #tpi_clear

              sub       rcount, #1
              jmp       #:loop

cs_bit_mask long  BIT_10
rs_bit_mask long  BIT_11
wr_bit_mask long  BIT_12
rd_bit_mask long  BIT_13

all_pin_mask long  0x3fc   ' 0b0000_0011_1111_1100
all_ctl_mask long  0x3ffc  ' 0b0011_1111_1111_1100

zero        long  0
rcount      long  0

pmb_data    res       1
tmp         res       1

                        .compress default

/**
 * function to start the SeeedTFT code in its own COG
 * C interface is:
 *   int _SeeedTftStartCog(void *arg)
 *
 * returns the number of the COG, or -1 if no COGs are left
 */
                        .text
                        .global __SeeedTftStartCog
__SeeedTftStartCog      mviw r7, #__load_start_seeedtft_as_cog  '' linker magic for the start of the spi_as.cog section
                        shl r7, #2
                        or r7, #8                          '' 8 means first available cog
                        shl r0, #16                        '' assumes bottom two bits of r0 are 0, i.e. arg must be long aligned
                        or r0, r7
                        coginit r0 wc,wr
        if_b            neg r0, #1                         '' if C is set, return -1
                        // Temporary hack until fix for GCC is released
#ifdef __PROPELLER_CMM__
                        lret
#else
                        mov pc, lr
#endif
