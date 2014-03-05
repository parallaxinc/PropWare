/**
 * @file    Concurrency_Demo.cpp
 *
 * @project PropWare_Demo
 *
 * @author  David Zemon
 */

// Includes
#include "Concurrency_Demo.h"

using namespace PropWare::GPIO;

volatile int lock;

const Pin g_pin = P23;

// Main function
int main () {

    lock = locknew();

    cog_run((void (*) (void *)) &blinkAnLEDSome, 64);

    while(lockset(lock));

    set_dir(P16, OUT);

    for (int i = 0; i < 40; ++i) {
        waitcnt(50 * MILLISECOND + CNT);
        pin_toggle(P16);
    }

    lockclr(lock);

    return 0;
}

void blinkAnLEDSome (void) {
    while(lockset(lock));

    set_dir(g_pin, OUT);

    for (int i = 0; i < 40; ++i) {
        waitcnt(50 * MILLISECOND + CNT);
        pin_toggle(g_pin);
    }

    lockclr(lock);
}
