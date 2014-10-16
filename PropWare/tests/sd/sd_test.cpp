/**
 * @file    pin_test.cpp
 *
 * @author  David Zemon
 *
 * Hardware:
 *      SD card connected with the following pins:
 *          - MOSI = P0
 *          - MISO = P1
 *          - SCLK = P2
 *          - CS   = P4
 *      SD card with following files:
 *          /test1.txt
 *          /child1/
 *              ./test2.txt
 *              ./grandson/
 *              ./grandson/test3.txt
 *          /child2/
 *              ./test4.txt
 *              ./granddaughter/                     // Inoperable
 *              ./granddaughter/longFilename.txt     // Inoperable
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

#include <PropWare/sd.h>
#include <PropWare/spi.h>
#include "../PropWareTests.h"

PropWare::SD *testable;
bool         didInit;

const PropWare::Pin::Mask MOSI = PropWare::Pin::P0;
const PropWare::Pin::Mask MISO = PropWare::Pin::P1;
const PropWare::Pin::Mask SCLK = PropWare::Pin::P2;
const PropWare::Pin::Mask CS   = PropWare::Pin::P4;

void setUp (const bool init = true) {
    PropWare::SPI *spi = PropWare::SPI::get_instance();

    testable = new PropWare::SD(spi);

    if (init) {
        testable->start(MOSI, MISO, SCLK, CS);
        testable->mount();
    }

    didInit = init;
}

TEARDOWN {
    if (didInit)
        testable->unmount();

    PropWare::SPI::get_instance()->stop();
    delete testable;
}

TEST(Start) {
    setUp(false);

    MSG_IF_FAIL(1, ASSERT_FALSE(testable->start(MOSI, MISO, SCLK, CS)),
                "Failed to start %s", ":(");

    tearDown();
}

TEST(Mount) {
    setUp(false);

    ASSERT_FALSE(testable->start(MOSI, MISO, SCLK, CS));
    ASSERT_FALSE(testable->mount());

    tearDown();
}

int main () {
    START(SDTest);

    RUN_TEST(Start);
    RUN_TEST(Mount);

    COMPLETE();
}
