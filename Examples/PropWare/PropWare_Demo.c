/**
 * @file    PropWare_Demo.c
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
    uint32_t pin[] = {
            BIT_16,
            BIT_17,
            BIT_18,
            BIT_19,
            BIT_20,
            BIT_21,
            BIT_22,
            BIT_23 };
    uint32_t nextcnt;

    wait_time = 50 * MILLISECOND;

    syncstart = 0;

    for (n = 1; n < COGS; n++) {
        cog = _start_cog_thread(cog_stack[n] + STACK_SIZE, do_toggle,
                (void*) pin[n], &thread_data);
        printf("Toggle COG %d Started\n", cog);
    }

    gpio_set_dir(pin[0], GPIO_DIR_OUT);

    startcnt = CNT;
    syncstart = 1;
    nextcnt = wait_time + startcnt;
    while (1) {
        gpio_pin_set(pin[0]);
        nextcnt = waitcnt2(nextcnt, wait_time);
        gpio_pin_clear(pin[0]);
        nextcnt = waitcnt2(nextcnt, wait_time);
    }
    return 0;
}

void do_toggle (void *arg) {
    uint32_t pin = (uint32_t) arg;
    uint32_t nextcnt;

    gpio_set_dir(pin, GPIO_DIR_OUT);

    // wait for start signal from main cog
    while (syncstart == 0)
        ;

    nextcnt = wait_time + startcnt;
    while (1) {
        gpio_pin_toggle(pin);
        nextcnt = waitcnt2(nextcnt, wait_time);
    }
}

