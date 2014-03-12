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

volatile int lock = locknew();

const Pin g_pin1 = P23;
const Pin g_pin2 = P16;
const int g_someStackSpace = 64;

// Main function
int main () {

    cog_run((void (*) (void *)) &blinkAnLEDSome, g_someStackSpace);

    while(lockset(lock));

    set_dir(g_pin2, OUT);

    for (int i = 0; i < 40; ++i) {
        waitcnt(50 * MILLISECOND + CNT);
        pin_toggle(g_pin2);
    }

    lockclr(lock);

    return 0;
}

void blinkAnLEDSome (void) {
    while(lockset(lock));

    set_dir(g_pin1, OUT);

    for (int i = 0; i < 40; ++i) {
        waitcnt(50 * MILLISECOND + CNT);
        pin_toggle(g_pin1);
    }

    lockclr(lock);
}
