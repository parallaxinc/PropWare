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
    static volatile PropWare::GPIO::Pin pin[] = {
            PropWare::GPIO::P16,
            PropWare::GPIO::P17,
            PropWare::GPIO::P18,
            PropWare::GPIO::P19,
            PropWare::GPIO::P20,
            PropWare::GPIO::P21,
            PropWare::GPIO::P22,
            PropWare::GPIO::P23 };
    uint32_t nextcnt;

    wait_time = 50 * MILLISECOND;

    syncstart = 0;

    for (n = 1; n < COGS; n++) {
        cog = _start_cog_thread(cog_stack[n] + STACK_SIZE, do_toggle,
                (void *) &pin[n], &thread_data);
        printf("Toggle COG %d Started\n", cog);
    }

    PropWare::GPIO::set_dir(pin[0], PropWare::GPIO::OUT);

    startcnt = CNT;
    syncstart = 1;
    nextcnt = wait_time + startcnt;
    while (1) {
        PropWare::GPIO::pin_set(pin[0]);
        nextcnt = waitcnt2(nextcnt, wait_time);
        PropWare::GPIO::pin_clear(pin[0]);
        nextcnt = waitcnt2(nextcnt, wait_time);
    }
    return 0;
}

void do_toggle (void *arg) {
    PropWare::GPIO::Pin *pin = (PropWare::GPIO::Pin *) arg;
    uint32_t nextcnt;

    PropWare::GPIO::set_dir(*pin, PropWare::GPIO::OUT);

    // wait for start signal from main cog
    while (syncstart == 0)
        ;

    nextcnt = wait_time + startcnt;
    while (1) {
        PropWare::GPIO::pin_toggle(*pin);
        nextcnt = waitcnt2(nextcnt, wait_time);
    }
}

