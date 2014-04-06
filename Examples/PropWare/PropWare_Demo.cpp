/**
 * @file    PropWare_Demo.cpp
 *
 * @project PropWare_Demo
 *
 * @author  Modified by David Zemon
 */

#include "PropWare_Demo.h"

static uint32_t cog_stack[STACK_SIZE][8];
static _thread_state_t thread_data;

volatile uint32_t wait_time;
volatile uint32_t startcnt;
volatile int8_t syncstart;

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
    uint32_t nextcnt;

    wait_time = 50 * MILLISECOND;

    syncstart = 0;

    for (n = 1; n < COGS; n++) {
        cog = _start_cog_thread(cog_stack[n] + STACK_SIZE, do_toggle,
                (void *) &pins[n], &thread_data);
        printf("Toggle COG %d Started\n", cog);
    }

    pin.set_mask(pins[0]);
    pin.set_dir(PropWare::Pin::OUT);

    startcnt = CNT;
    syncstart = 1;
    nextcnt = wait_time + startcnt;
    while (1) {
        pin.set();
        nextcnt = waitcnt2(nextcnt, wait_time);
        pin.clear();
        nextcnt = waitcnt2(nextcnt, wait_time);
    }
    return 0;
}

void do_toggle (void *arg) {
    PropWare::Pin pin;
    uint32_t nextcnt;

    pin.set_mask(*(PropWare::Pin::Mask *) arg);
    pin.set_dir(PropWare::Pin::OUT);

    // wait for start signal from main cog
    while (syncstart == 0)
        ;

    nextcnt = wait_time + startcnt;
    while (1) {
        pin.toggle();
        nextcnt = waitcnt2(nextcnt, wait_time);
    }
}

