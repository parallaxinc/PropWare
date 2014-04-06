/**
 * @file    Concurrency_Demo.cpp
 *
 * @project PropWare_Demo
 *
 * @author  David Zemon
 */

// Includes
#include "Concurrency_Demo.h"

volatile int lock = locknew();

const PropWare::Pin g_pin1(PropWare::Port::P23, PropWare::Pin::OUT);
const PropWare::Pin g_pin2(PropWare::Port::P16, PropWare::Pin::OUT);
const int g_someStackSpace = 64;

// Main function
int main () {

    cog_run((void (*) (void *)) &blinkAnLEDSome, g_someStackSpace);

    while(lockset(lock));

    for (int i = 0; i < 40; ++i) {
        waitcnt(50 * MILLISECOND + CNT);
        g_pin2.toggle();
    }

    lockclr(lock);

    return 0;
}

void blinkAnLEDSome (void) {
    while(lockset(lock));

    for (int i = 0; i < 40; ++i) {
        waitcnt(50 * MILLISECOND + CNT);
        g_pin1.toggle();
    }

    lockclr(lock);
}
