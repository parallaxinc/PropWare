/**
 * @file    PropWare_Demo.c
 */
/**
 * @project PropWare_Demo
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

#include "PropWare_Demo.h"

static int cog_stack[STACK_SIZE][8];
static _thread_state_t thread_data;

volatile unsigned int wait_time;
volatile unsigned int startcnt;
volatile unsigned int pins;
volatile int syncstart;

int main (int argc, char* argv[]) {
    int n;
    int cog;
    int pin[] = {
            BIT_16,
            BIT_17,
            BIT_18,
            BIT_19,
            BIT_20,
            BIT_21,
            BIT_22,
            BIT_23 };
    unsigned int nextcnt;

    wait_time = 50 * MILLISECOND;

    syncstart = 0;

    for (n = 1; n < COGS; n++) {
        cog = _start_cog_thread(cog_stack[n] + STACK_SIZE, do_toggle,
                (void*) pin[n], &thread_data);
        printf("Toggle COG %d Started\n", cog);
    }

    startcnt = CNT;
    syncstart = 1;
    nextcnt = wait_time + startcnt;
    while (1) {
        GPIOPinToggle(pin[0]);
        nextcnt = waitcnt2(nextcnt, wait_time);
    }
    return 0;
}

void do_toggle (void *arg) {
    int pin = (int) arg;
    unsigned int nextcnt;

    while (syncstart == 0)
        ;  // wait for start signal from main cog

    nextcnt = wait_time + startcnt;
    while (1) {
        GPIOPinToggle(pin);
        nextcnt = waitcnt2(nextcnt, wait_time);
    }
}
