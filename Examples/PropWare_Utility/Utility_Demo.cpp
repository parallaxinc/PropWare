/**
 * @file    Utility_Demo.cpp
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
#include <PropWare/printer.h>

void sampleCountBits ();
void sampleTimeMeasurement ();
void sampleFreeMemory ();

int main () {
    pwOut.printf("Hello! Let's explore some of PropWare's utility functions"
                         CRLF);

    sampleCountBits();
    sampleTimeMeasurement();
    sampleFreeMemory();

    return 0;
}

void sampleTimeMeasurement () {
    unsigned int start = CNT;
    pwOut.printf(CRLF "/*** Timing Events ***/" CRLF);
    pwOut.printf("But how long does it take to send each of these messages?"
                         CRLF);
    uint32_t timeInMicros = PropWare::Utility::measure_time_interval(start);
    pwOut.printf("Well that previous message took precisely %u microseconds."
                         CRLF, timeInMicros);
}

void sampleCountBits () {
    int bits = 0x42;
    pwOut.printf(CRLF "/*** Counting Bits in a Variable ***/" CRLF);
    pwOut.printf("How many bits are set in 0x%02X?" CRLF, bits);
    pwOut.printf("\tThe answer is... %u" CRLF,
                 PropWare::Utility::count_bits(bits));
}

void sampleFreeMemory () {
    pwOut.printf(CRLF "/*** Free Memory Determination ***/" CRLF);
    pwOut.printf("The largest contiguous block of free memory is %u bytes" CRLF,
                 PropWare::Utility::get_largest_free_block_size());
}
