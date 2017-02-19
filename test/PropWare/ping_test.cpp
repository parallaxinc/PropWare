/**
 * @file    ping_test.cpp
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

#include "PropWareTests.h"
#include <PropWare/sensor/distance/ping.h>

using PropWare::Ping;
using PropWare::Pin;

unsigned int g_clockTicks;

class MockPing: public Ping {
    public:
        MockPing()
            : Ping(Pin::Mask::NULL_PIN) {
        }

        virtual unsigned int get_clock_ticks() const {
            return g_clockTicks;
        }
};

MockPing testable;

TEARDOWN {
}

TEST(GetMicroseconds) {
    g_clockTicks = 10*MICROSECOND;

    ASSERT_EQ_MSG(10, testable.get_microseconds());

    tearDown();
}

TEST(GetMillimeters) {
    g_clockTicks = MICROSECOND;

    ASSERT_EQ_MSG(17, testable.get_millimeters());

    tearDown();
}

TEST(GetCentimeters) {
    g_clockTicks = 1000*MICROSECOND;

    ASSERT_EQ_MSG(17, testable.get_centimeters());

    tearDown();
}

TEST(GetInches) {
    g_clockTicks = 1480*MICROSECOND;

    ASSERT_EQ_MSG(10, testable.get_inches());

    tearDown();
}

int main() {
    START(PingTest);

    RUN_TEST(GetMicroseconds);
    RUN_TEST(GetMillimeters);
    RUN_TEST(GetCentimeters);
    RUN_TEST(GetInches);

    COMPLETE();
}
