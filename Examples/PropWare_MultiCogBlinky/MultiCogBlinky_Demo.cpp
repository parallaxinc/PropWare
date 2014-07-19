/**
 * @file    MultiCogBlinky_Demo.cpp
 *
 * @author  Modified by David Zemon
 */

#include "MultiCogBlinky_Demo.h"

static uint32_t cog_stack[STACK_SIZE][8];
static _thread_state_t thread_data;

volatile uint32_t wait_time;
volatile uint32_t startCnt;
volatile int8_t syncStart;

int main (int argc, char* argv[]) {
    int8_t n;
    int8_t cog;
    PropWare::Pin pin;
    static volatile PropWare::Pin::Mask pins[] = {
            PropWare::Port::P16,
            PropWare::Port::P17,
            PropWare::Port::P18,
            PropWare::Port::P19,
            PropWare::Port::P20,
            PropWare::Port::P21,
            PropWare::Port::P22,
            PropWare::Port::P23 };
    uint32_t nextCnt;

    wait_time = (uint32_t) (50 * MILLISECOND);

    syncStart = 0;

    for (n = 1; n < COGS; n++) {
        cog = (int8_t) _start_cog_thread(cog_stack[n] + STACK_SIZE, do_toggle,
                        (void *) &pins[n], &thread_data);
        printf("Toggle COG %d Started\n", cog);
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
    uint32_t nextcnt;

    pin.set_mask(*(PropWare::Pin::Mask *) arg);
    pin.set_dir(PropWare::Pin::OUT);

    // wait for start signal from main cog
    while (syncStart == 0)
        ;

    nextcnt = wait_time + startCnt;
    while (1) {
        pin.toggle();
        nextcnt = waitcnt2(nextcnt, wait_time);
    }
}

