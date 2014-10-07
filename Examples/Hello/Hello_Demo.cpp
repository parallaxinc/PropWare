/**
 * @file    Hello_Demo.cpp
 *
 * @author  David Zemon
 */

#define TEST_PROPWARE
//#define TEST_SIMPLE
//#define TEST_TINYSTREAM
//#define TEST_TINYIO

// Includes
#include <PropWare/PropWare.h>

#ifdef TEST_PROPWARE
#include <PropWare/printer.h>
#elif defined TEST_SIMPLE
#include <simpletext.h>
#elif defined TEST_TINYSTREAM
#include <tinystream>
namespace std {
std::stream cout;
}
#elif defined TEST_TINYIO
#include <tinyio.h>
#endif

// Main function
int main () {
    uint32_t i = 0;

    while (1) {
#ifdef TEST_PROPWARE
        pwOut.printf("Hello, world! %03d 0x%02X" CRLF, i, i);
#elif defined TEST_SIMPLE
        print("Hello, world! %03d 0x%02x" CRLF, i, i);
#elif defined TEST_TINYSTREAM
        std::cout << "Hello, world! " << i << ' ' << i << std::endl;
#elif defined TEST_TINYIO
        printf("Hello, world! %03d 0x%02x" CRLF, i, i);
#endif
        i++;
        waitcnt(500 * MILLISECOND + CNT);
    }

    return 0;
}
