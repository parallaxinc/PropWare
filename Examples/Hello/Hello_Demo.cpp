/**
 * @file    Concurrency_Demo.cpp
 *
 * @author  David Zemon
 */

#define TEST_PROPWARE
//#define TEST_SIMPLE

// Includes
#include <PropWare/PropWare.h>

#ifdef TEST_PROPWARE
#include <PropWare/uart/simplexuart.h>
#include <PropWare/printer.h>
#endif

#ifdef TEST_SIMPLE
#include <simpletext.h>
#endif

// Main function
int main () {
    uint32_t i = 0;

#ifdef TEST_PROPWARE
    const PropWare::SimplexUART uart(PropWare::UART::PARALLAX_STANDARD_TX);
    const PropWare::Printer printer(&uart);

    while (1) {
        printer.printf("Hello, world! %d" CRLF, i++);
        waitcnt(500 * MILLISECOND + CNT);
    }
#elif defined TEST_SIMPLE
    while (1) {
        print("Hello, world! %d" CRLF, i++);
        waitcnt(500 * MILLISECOND + CNT);
    }
#endif

    return 0;
}
