/**
 * @file    uart_as.S
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

                        .section uart_as.cog, "ax"
                        .compress off

                        org 0

                        or dira, testLEDs               '' DEBUG: LEDs used for debugging purposes, can be removed later
                        wrlong negOne, par              '' Inform parent cog that initialization has begun

                        // Begin by retrieving all parameters...
                        call #READ_CMD                  '' Read in the pin mask TX
                        mov txMask, mailbox
                        call #READ_CMD                  '' Read in the pin number for TX
                        mov txPinNum, mailbox
                        call #READ_CMD                  '' Read in the pin mask RX
                        mov rxMask, mailbox
                        call #READ_CMD                  '' Read in the pin number for RX
                        mov rxPinNum, mailbox
                        call #READ_CMD                  '' Read in the number of data bits
                        mov dataWidth, mailbox
                        call #READ_CMD                  '' Read in the parity settings
                        mov parity, mailbox
                        call #READ_CMD                  '' Read in the number of stop bits
                        mov stopBits, mailbox

                        // Followed by setting TX and RX
                        or dira, txMask
                        andn dira, rxMask

/*** MAIN LOOP ***/
LOOP                    // Retrieve a command
                        call #READ_CMD
                        mov temp, mailbox
                        and temp, uartFuncBits          '' Mask away all bits except the function descriptor

                        // Using "jmp" instead of "call" because all functions return by "jmp #LOOP" instead of "ret"
                        // If command is "Send"
                        cmd temp, #UAR_FUNC_SEND wz
                        jmp #SEND

                        // Default: Return to loop
                        jmp #LOOP

/* FUNCTION: PropWare::UART::send() */
SEND                    // Interpret

SEND_ret                ret

/******************************
 *** Beginning of variables ***
 ******************************/
mailbox                 res     1                       '' Address in hub memory used for communication with another cog
temp                    res     1                       '' Working register
bitCount                res     1                       '' Keep track of how many bits are being used to send data
sendDelay               res     1                       '' Delay between bits of data being transmitted
