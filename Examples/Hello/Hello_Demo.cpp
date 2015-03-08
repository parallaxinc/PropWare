/**
 * @file    Hello_Demo.cpp
 *
 * @author  David Zemon
 */

#define TEST_PROPWARE
//#define TEST_SIMPLE
//#define TEST_TINYSTREAM
//#define TEST_TINYIO
//#define TEST_FDSERIAL

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
#endif

// Main function
int main () {
    uint32_t i = 0;

#ifdef TEST_FDSERIAL
    fdserial *serial = fdserial_open(31, 30, 0, 115200);
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
#endif
        i++;
        waitcnt(250 * MILLISECOND + CNT);
    }

    return 0;
}
