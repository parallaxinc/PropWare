/**
 * @file    Hello_Demo.cpp
 *
 * @author  David Zemon
 */

#define TEST_PROPWARE
//#define TEST_SIMPLE

// Includes
#include <PropWare/PropWare.h>
#include <PropWare/printer.h>
#include <simpletext.h>

// Main function
int main () {
    uint32_t i = 0;
    const float f = 4.2;

#ifdef TEST_PROPWARE
    while (1) {
        pstw.printf("Hello, world! %03d 0x%02X %06.3f" CRLF, i, i, f);
        ++i;
        waitcnt(500 * MILLISECOND + CNT);
    }
#elif defined TEST_SIMPLE
    while (1) {
        print("Hello, world! %03d 0x%02x %06.3f" CRLF, i, i, f);
        i++;
        waitcnt(500 * MILLISECOND + CNT);
    }
#endif

    return 0;
}
