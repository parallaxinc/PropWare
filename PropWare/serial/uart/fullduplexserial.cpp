/**
 * @file    PropWare/serial/uart/fullduplexserial.cpp
 *
 * @author  Chip Gracey
 * @author  Jeff Martin
 *
 * Assembly code from Full-Duplex Serial Driver
 *
 * @copyright
 * The MIT License (MIT)<br>
 * <br>2006-2009 Parallax, Inc.<br>
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

#include <stdint.h>


extern uint8_t _load_start_FullDuplexSerial_cog[];

namespace PropWare {

void *get_full_duplex_serial_driver () {
    return _load_start_FullDuplexSerial_cog;
}

}

__asm__ (
"            .section .FullDuplexSerial.cog, \"ax\"                   \n"
"            .compress off                                            \n"
"..start                                                              \n"
"            .org    0                                                \n"
"                                                                     \n"
"entry                                                                \n"
"            mov     t1, PAR                                          \n"
"            add     t1, #(4 << 2)                                    \n"
"            rdlong  t2, t1                                           \n"
"            mov     rxmask, #1                                       \n"
"            shl     rxmask, t2                                       \n"
"            add     t1, #4                                           \n"
"            rdlong  t2, t1                                           \n"
"            mov     txmask, #1                                       \n"
"            shl     txmask, t2                                       \n"
"            add     t1, #4                                           \n"
"            rdlong  rxtxmode, t1                                     \n"
"            add     t1, #4                                           \n"
"            rdlong  bitticks, t1                                     \n"
"            add     t1, #4                                           \n"
"            rdlong  rxbuff, t1                                       \n"
"            mov     txbuff, rxbuff                                   \n"
"            add     txbuff, #$10                                     \n"
"            test    rxtxmode, #4    wz                               \n"
"            test    rxtxmode, #2    wc                               \n"
"  if_z_ne_c or      OUTA, txmask                                     \n"
"  if_z      or      DIRA, txmask                                     \n"
"            mov     txcode, #((transmit-..start)/4)                  \n"
"            wrlong  zero, PAR                                        \n"
"                                                                     \n"
"receive                                                              \n"
"            jmpret  rxcode, txcode                                   \n"
"            test    rxtxmode, #1    wz                               \n"
"            test    rxmask, INA    wc                                \n"
"  if_z_eq_c jmp     #receive                                         \n"
"            mov     rxbits, #9                                       \n"
"            mov     rxcnt, bitticks                                  \n"
"            shr     rxcnt, #1                                        \n"
"            add     rxcnt, CNT                                       \n"
"                                                                     \n"
"Receive_bit                                                          \n"
"            add     rxcnt, bitticks                                  \n"
"                                                                     \n"
"Receive_wait                                                         \n"
"            jmpret  rxcode, txcode                                   \n"
"            mov     t1, rxcnt                                        \n"
"            sub     t1, CNT                                          \n"
"            cmps    t1, #0    wc                                     \n"
"  if_nc     jmp     #Receive_wait                                    \n"
"            test    rxmask, INA    wc                                \n"
"            rcr     rxdata, #1                                       \n"
"            djnz    rxbits, #Receive_bit                             \n"
"            shr     rxdata, #($20 - 9)                               \n"
"            and     rxdata, #$ff                                     \n"
"            test    rxtxmode, #1    wz                               \n"
"  if_nz     xor     rxdata, #$ff                                     \n"
"            rdlong  t2, PAR                                          \n"
"            add     t2, rxbuff                                       \n"
"            wrbyte  rxdata, t2                                       \n"
"            sub     t2, rxbuff                                       \n"
"            add     t2, #1                                           \n"
"            and     t2, #$f                                          \n"
"            wrlong  t2, PAR                                          \n"
"            jmp     #receive                                         \n"
"                                                                     \n"
"transmit                                                             \n"
"            jmpret  txcode, rxcode                                   \n"
"            mov     t1, PAR                                          \n"
"            add     t1, #(2 << 2)                                    \n"
"            rdlong  t2, t1                                           \n"
"            add     t1, #(1 << 2)                                    \n"
"            rdlong  t3, t1                                           \n"
"            cmp     t2, t3    wz                                     \n"
"  if_z      jmp     #transmit                                        \n"
"            add     t3, txbuff                                       \n"
"            rdbyte  txdata, t3                                       \n"
"            sub     t3, txbuff                                       \n"
"            add     t3, #1                                           \n"
"            and     t3, #$f                                          \n"
"            wrlong  t3, t1                                           \n"
"            or      txdata, #$100                                    \n"
"            shl     txdata, #2                                       \n"
"            or      txdata, #1                                       \n"
"            mov     txbits, #$b                                      \n"
"            mov     txcnt, CNT                                       \n"
"                                                                     \n"
"Transmit_bit                                                         \n"
"            test    rxtxmode, #4    wz                               \n"
"            test    rxtxmode, #2    wc                               \n"
"  if_z_and_c xor     txdata, #1                                      \n"
"            shr     txdata, #1    wc                                 \n"
"  if_z      muxc    OUTA, txmask                                     \n"
"  if_nz     muxnc   DIRA, txmask                                     \n"
"            add     txcnt, bitticks                                  \n"
"                                                                     \n"
"Transmit_wait                                                        \n"
"            jmpret  txcode, rxcode                                   \n"
"            mov     t1, txcnt                                        \n"
"            sub     t1, CNT                                          \n"
"            cmps    t1, #0    wc                                     \n"
"  if_nc     jmp     #Transmit_wait                                   \n"
"            djnz    txbits, #Transmit_bit                            \n"
"            jmp     #transmit                                        \n"
"                                                                     \n"
"zero                                                                 \n"
"            .long   0                                                \n"
"                                                                     \n"
"t1                                                                   \n"
"            .res    1                                                \n"
"                                                                     \n"
"t2                                                                   \n"
"            .res    1                                                \n"
"                                                                     \n"
"t3                                                                   \n"
"            .res    1                                                \n"
"                                                                     \n"
"rxtxmode                                                             \n"
"            .res    1                                                \n"
"                                                                     \n"
"bitticks                                                             \n"
"            .res    1                                                \n"
"                                                                     \n"
"rxmask                                                               \n"
"            .res    1                                                \n"
"                                                                     \n"
"rxbuff                                                               \n"
"            .res    1                                                \n"
"                                                                     \n"
"rxdata                                                               \n"
"            .res    1                                                \n"
"                                                                     \n"
"rxbits                                                               \n"
"            .res    1                                                \n"
"                                                                     \n"
"rxcnt                                                                \n"
"            .res    1                                                \n"
"                                                                     \n"
"rxcode                                                               \n"
"            .res    1                                                \n"
"                                                                     \n"
"txmask                                                               \n"
"            .res    1                                                \n"
"                                                                     \n"
"txbuff                                                               \n"
"            .res    1                                                \n"
"                                                                     \n"
"txdata                                                               \n"
"            .res    1                                                \n"
"                                                                     \n"
"txbits                                                               \n"
"            .res    1                                                \n"
"                                                                     \n"
"txcnt                                                                \n"
"            .res    1                                                \n"
"                                                                     \n"
"txcode                                                               \n"
"            .res    1                                                \n"
"            .compress default                                        \n"
"            .text                                                    \n"
);
