/**
 * @file    MultiCogBlinky_Demo.cpp
 *
 * @author  Modified by David Zemon
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

// Note the lack of an include for propeller.h; This is because PropWare.h will
// include propeller.h for you
#include <PropWare/PropWare.h>
#include <PropWare/gpio/pin.h>
#include <PropWare/hmi/output/printer.h>

using PropWare::Pin;

/**
* @brief       Toggle thread function gets started in an LMM COG.
*
* @param[in]   arg     pin number to toggle
*/
static void run_cog(void *arg);

static const uint16_t         COGS       = 8;
static const uint16_t         STACK_SIZE = 16;
static uint32_t        cog_stack[STACK_SIZE][COGS];
static _thread_state_t thread_data;

static volatile uint32_t wait_time;
static volatile uint32_t startCnt;
static volatile int8_t   syncStart;

/**
 * @example     MultiCogBlinky_Demo.cpp
 *
 * Use PropGCC's native interface to blink an LED from each of the Propeller's 8 cogs. Each cog will be invoking
 * instructions out of HUB memory, not COG memory. For a simpler and more object-oriented way to start new cogs,
 * check out PropWare::Runnable
 *
 * @include PropWare_MultiCogBlinky/CMakeLists.txt
 */
int main(int argc, char *argv[]) {
    int8_t              n;
    int8_t              cog;
    Pin       pin;
    uint32_t            nextCnt;
    const Pin::Mask pins[] = {
        Pin::P16,
        Pin::P17,
        Pin::P18,
        Pin::P19,
        Pin::P20,
        Pin::P21,
        Pin::P22,
        Pin::P23
    };

    wait_time = 50 * MILLISECOND;

    syncStart = 0;

    for (n = 1; n < COGS; n++) {
        cog = (int8_t) _start_cog_thread(cog_stack[n] + STACK_SIZE, run_cog, (void *) &pins[n], &thread_data);
        pwOut.printf("Toggle COG %d Started\n", cog);
    }

    pin.set_mask(pins[0]);
    pin.set_dir_out();

    startCnt  = CNT;
    syncStart = 1;
    nextCnt   = wait_time + startCnt;
    while (1) {
        pin.set();
        nextCnt = waitcnt2(nextCnt, wait_time);
        pin.clear();
        nextCnt = waitcnt2(nextCnt, wait_time);
    }
}

void run_cog(void *arg) {
    Pin pin;
    uint32_t      nextCnt;

    pin.set_mask(*(Pin::Mask *) arg);
    pin.set_dir_out();

    // wait for start signal from main cog
    while (syncStart == 0);

    nextCnt = wait_time + startCnt;
    while (1) {
        pin.toggle();
        nextCnt = waitcnt2(nextCnt, wait_time);
    }
}

