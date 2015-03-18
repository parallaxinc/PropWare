/**
 * @file    StringBuilder_Demo.cpp
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

#include <PropWare/PropWare.h>
#include <PropWare/stringbuilder.h>
#include <PropWare/staticstringbuilder.h>
#include <PropWare/printer/printer.h>
#include <PropWare/scanner.h>

using namespace PropWare;

unsigned int next_fibonacci (const bool clear = false);

unsigned int run_buffered (const int iterations);

unsigned int run_static_buffered (const int iterations);

unsigned int run_unbuffered (const int iterations);

int main () {
    unsigned int dynamicBuffer, staticBuffer, noBuffer;

    pwOut << "PropWare::StringBuilder Demo\n";
    pwOut << "============================\n";

    // Get iterations
    int iterations;
    pwIn.input_prompt("How many iterations of the fibonacci sequency would you like to see?\n>>> ",
                      "Please enter a non-negative whole-number\n", &iterations, NON_NEGATIVE_COMP);

    // Run with StringBuilder
    dynamicBuffer = run_buffered(iterations);
    pwOut << "Total runtime was " << dynamicBuffer << "us using the PropWare::StringBuilder class\n";

    // Reset the fibonacci function
    pwOut << "\nLet's try this again, but static initialization (no use of malloc).\n";
    next_fibonacci(true);

    // Run with StaticStringBuilder
    staticBuffer = run_static_buffered(iterations);
    pwOut << "Total runtime was " << staticBuffer << "us using the PropWare::StaticStringBuilder class\n";

    // Reset the fibonacci function
    pwOut << "\nFinally, let's try this without any buffering.\n";
    next_fibonacci(true);

    // Run unbuffered
    noBuffer = run_unbuffered(iterations);
    pwOut << "Total runtime was " << noBuffer << "us without buffering\n";

    // Exit
    pwOut.println("Runtime results:");
    pwOut << "\tDynamic buffer: " << dynamicBuffer << " us\n";
    pwOut << "\tStatic buffer:  " << staticBuffer << " us\n";
    pwOut << "\tUnbuffered:     " << noBuffer << " us\n";
    pwOut << "Take note that using a buffer won't help you increase your runtime performance.\n";

    return 0;
}

unsigned int run_buffered (const int iterations) {
    const volatile unsigned int timerStart = CNT;

    StringBuilder string;
    const Printer stringStream(&string);
    for (int i = 0; i < iterations; ++i) {
        stringStream << next_fibonacci();
        if (i != iterations - 1)
            stringStream << ", ";
    }

    pwOut << string.to_string() << '\n';
    return Utility::measure_time_interval(timerStart);
}

unsigned int run_static_buffered (const int iterations) {
    const volatile unsigned int timerStart = CNT;

    char buffer[4096];
    StaticStringBuilder string(buffer);
    const Printer stringStream(&string);
    for (int i = 0; i < iterations; ++i) {
        stringStream << next_fibonacci();
        if (i != iterations - 1)
            stringStream << ", ";
    }

    pwOut << string.to_string() << '\n';
    return Utility::measure_time_interval(timerStart);
}

unsigned int run_unbuffered (const int iterations) {
    const volatile unsigned int timerStart = CNT;
    for (int i = 0; i < iterations; ++i) {
        pwOut << next_fibonacci();
        if (i != iterations - 1)
            pwOut << ", ";
    }
    pwOut << '\n';
    return Utility::measure_time_interval(timerStart);
}

unsigned int next_fibonacci (const bool clear) {
    static unsigned int next     = 1;
    static unsigned     previous = 1;

    if (clear) {
        next = 1;
        previous = 1;
    } else {
        const unsigned int retVal = next;
        next += previous;
        previous                     = retVal;
        return retVal;
    }

    return 0;
}
