/**
 * @file    Hello_Demo.cpp
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

#define TEST_PROPWARE
//#define TEST_SIMPLE
//#define TEST_TINYSTREAM
//#define TEST_TINYIO
//#define TEST_FDSERIAL
//#define TEST_LIBPROPELLER

// Includes
#include <PropWare/PropWare.h>

#ifdef TEST_PROPWARE
#include <PropWare/printer/printer.h>
#elif defined TEST_SIMPLE
#include <simpletext.h>
#elif defined TEST_TINYSTREAM
#include <tinystream>
namespace std {
std::stream cout;
}
#elif defined TEST_TINYIO
#include <tinyio.h>
#elif defined TEST_FDSERIAL
#include <fdserial.h>
#elif defined TEST_LIBPROPELLER
#include <libpropeller/serial/serial.h>

#endif

#ifndef TEST_PROPWARE
int _cfg_rxpin    = -1;
int _cfg_txpin    = -1;
int _cfg_baudrate = -1;
#endif

// Main function
int main () {
    uint32_t i = 0;

#ifdef TEST_FDSERIAL
    fdserial *serial = fdserial_open(_cfg_rxpin, _cfg_txpin, 0, _cfg_baudrate);
#elif defined TEST_LIBPROPELLER
    libpropeller::Serial serial;
    serial.Start(_cfg_rxpin, _cfg_txpin, _cfg_baudrate);
#endif

    while (1) {
#ifdef TEST_PROPWARE
        pwOut.printf("Hello, world! %03d 0x%02X\n", i, i);
#elif defined TEST_SIMPLE
        printi("Hello, world! %03d 0x%02x\n", i, i);
#elif defined TEST_TINYSTREAM
        std::cout << "Hello, world! " << i << ' ' << i << std::endl;
#elif defined TEST_TINYIO
        printf("Hello, world! %03d 0x%02x\n", i, i);
#elif defined TEST_FDSERIAL
        // Please note that FdSerial support requires the inclusion of the file
        // `pst.dat` as a source file for this project. Because the file is no
        // longer included as part of the Simple libraries you must copy it from
        // this project to your own before attempting to compile.
        dprinti(serial, "Hello, world! %03d 0x%02x\n", i, i);
#elif defined TEST_LIBPROPELLER
        serial.PutFormatted("Hello, world! %03d 0x%02X\r\n", i, i);
#endif
        i++;
        waitcnt(250 * MILLISECOND + CNT);
    }

    return 0;
}
