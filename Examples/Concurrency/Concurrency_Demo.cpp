/**
 * @file    Concurrency_Demo.cpp
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

// Includes
#include <PropWare/PropWare.h>
#include <PropWare/pin.h>

void blinkAnLEDSome (void *arg);

volatile int lock = locknew();

// NOTE!!! The direction of a pin (or port) is not a member variable of the pin
// or port. Therefore, if you're going to declare a pin (or port) in one cog
// and use it in another, you MUST set the direction in the new port. Look at
// how g_pin2.set_dir() is called within blinkAnLEDSome() instead of the
// direction being set in the constructor
PropWare::Pin    g_pin1(PropWare::Port::P16, PropWare::Port::OUT);
PropWare::Pin    g_pin2(PropWare::Port::P17);
static const int g_someStackSpace = 64;

// Main function
int main () {
    static uint32_t        threadStack[g_someStackSpace];
    static _thread_state_t threadData;

    _start_cog_thread(threadStack + g_someStackSpace, blinkAnLEDSome,
            (void *) NULL, &threadData);

    waitcnt(20 * MICROSECOND + CNT);

    while (lockset(lock));

    for (int i = 0; i < 40; ++i) {
        waitcnt(50 * MILLISECOND + CNT);
        g_pin1.toggle();
    }

    lockclr(lock);

    return 0;
}

void blinkAnLEDSome (void *arg) {
    while (lockset(lock));

    g_pin2.set_dir(PropWare::Port::OUT);

    for (int i = 0; i < 40; ++i) {
        waitcnt(50 * MILLISECOND + CNT);
        g_pin2.toggle();
    }

    lockclr(lock);
}
