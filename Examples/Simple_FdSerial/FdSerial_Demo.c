/**
 * @file FdSerial_Demo.c
 *
 * @author  David Zemon
 *
 * Please note that FdSerial support requires the inclusion of the file
 * `pst.spin` as a source file for this project. Because the file is no longer
 * included as part of the Simple libraries you must copy it from this project
 * to your own before attempting to compile.
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

#include <fdserial.h>

/**
 * @example     FdSerial_Demo.c
 *
 * Print to the terminal using the FullDuplexSerial driver common in the Spin world
 *
 * @include     Simple_FdSerial/CMakeLists.txt
 * @include     Simple_FdSerial/pst.spin
 */
int main () {

    fdserial *serial = fdserial_open(31, 30, 0, 115200);

    while (1) {
        dprint(serial, "Hello, world!\n");
        waitcnt(CLKFREQ / 4 + CNT);
    }

    return 0;
}
