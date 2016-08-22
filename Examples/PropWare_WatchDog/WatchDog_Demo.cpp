/**
 * @file    WatchDog_Demo.cpp
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
#include <PropWare/concurrent/watchdog.h>
#include <PropWare/hmi/output/printer.h>

static uint32_t watchDogStack[96];

/**
 * @example     WatchDog_Demo.cpp
 *
 * Feed the dog a few times to show that the Propeller doesn't reset when fed correctly. Then stop feeding the dog
 * and demonstrate how the Propeller will do a hard reset when the timer runs out
 *
 * @include PropWare_WatchDog/CMakeLists.txt
 */
int main () {
    PropWare::WatchDog watchDog(watchDogStack, 1 * SECOND);
    PropWare::Runnable::invoke(watchDog);

    for (int i = 0; i < 5; ++i) {
        pwOut.printf("Resetting timer: %d\n", i);
        watchDog.reset();
        waitcnt(250*MILLISECOND + CNT);
    }

    int j = 0;
    while (true) {
        pwOut.printf("Resetting soon... %d\n", j++);
        waitcnt(400*MILLISECOND + CNT);
    }
}
