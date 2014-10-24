/**
 * @file    Blinky_Demo.cpp
 *
 * @author  David Zemon
 */

#include <PropWare/PropWare.h>
#include <PropWare/pin.h>

int main () {
    PropWare::Pin led(PropWare::Pin::P16, PropWare::Pin::OUT);

    while (1) {
        led.toggle();
        waitcnt(CLKFREQ / 4 + CNT);
    }
}

