/**
 * @file    MultiCogBlinky_Demo.cpp
 *
 * @author  Modified by David Zemon
 */

#include "MultiCogBlinky_Demo.h"

const uint16_t         COGS       = 8;
const uint16_t         STACK_SIZE = 16;
static uint32_t        cog_stack[STACK_SIZE][COGS];
static _thread_state_t thread_data;

volatile uint32_t wait_time;
volatile uint32_t startCnt;
volatile int8_t   syncStart;

int main (int argc, char* argv[]) {
    int8_t                              n;
    int8_t                              cog;
    PropWare::Pin                       pin;
    uint32_t                            nextCnt;
    static volatile PropWare::Pin::Mask pins[] = {
            PropWare::Port::P16,
            PropWare::Port::P17,
            PropWare::Port::P18,
            PropWare::Port::P19,
            PropWare::Port::P20,
            PropWare::Port::P21,
            PropWare::Port::P22,
            PropWare::Port::P23};

    wait_time = 50 * MILLISECOND;

    syncStart = 0;

    for (n = 1; n < COGS; n++) {
        cog = (int8_t) _start_cog_thread(cog_stack[n] + STACK_SIZE, do_toggle,
                        (void *) &pins[n], &thread_data);
        print("Toggle COG %d Started" CRLF, cog);
    }

    pin.set_mask(pins[0]);
    pin.set_dir(PropWare::Pin::OUT);

    startCnt = CNT;
    syncStart = 1;
    nextCnt = wait_time + startCnt;
    while (1) {
        pin.set();
        nextCnt = waitcnt2(nextCnt, wait_time);
        pin.clear();
        nextCnt = waitcnt2(nextCnt, wait_time);
    }
    return 0;
}

void do_toggle (void *arg) {
    PropWare::Pin pin;
    uint32_t      nextCnt;

    pin.set_mask(*(PropWare::Pin::Mask *) arg);
    pin.set_dir(PropWare::Pin::OUT);

    // wait for start signal from main cog
    while (syncStart == 0)
        ;

    nextCnt = wait_time + startCnt;
    while (1) {
        pin.toggle();
        nextCnt = waitcnt2(nextCnt, wait_time);
    }
}

