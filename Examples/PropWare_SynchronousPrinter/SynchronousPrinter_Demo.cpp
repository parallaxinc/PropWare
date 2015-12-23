/**
 * @file    SynchronousPrinter_Demo.cpp
 *
 * @author  David Zemon
 */

#include <PropWare/PropWare.h>
#include <PropWare/printer/synchronousprinter.h>
#include <PropWare/port.h>
#include <PropWare/pin.h>

void run_cog (void *arg);

const uint16_t         COGS       = 8;
const uint16_t         STACK_SIZE = 128;
static uint32_t        cog_stack[STACK_SIZE][COGS];
static _thread_state_t thread_data;

volatile uint32_t wait_time = SECOND;
volatile bool     syncStart = false;
volatile uint32_t startCnt;

/**
 * @example     SynchronousPrinter_Demo.cpp
 *
 * Demonstrate multiple cogs printing to the same serial terminal simultaneously
 *
 * @include PropWare_SynchronousPrinter/CMakeLists.txt
 */
int main (int argc, char *argv[]) {
    int8_t   n;
    int8_t   cog;
    uint32_t nextCnt;

    // If the comm port was not initialized successfully,
    // just sit here and complain
    if (!pwSyncOut.has_lock())
        while (1)
            PropWare::Port::flash_port(PropWare::BYTE_2, PropWare::BYTE_2);

    for (n = 1; n < COGS; n++) {
        cog = (int8_t) _start_cog_thread(cog_stack[n] + sizeof(cog_stack[n]), run_cog, NULL, &thread_data);
        pwSyncOut.printf("Toggle COG %d Started\n", cog);
    }

    startCnt = CNT;
    syncStart = true;
    nextCnt = wait_time + startCnt;
    while (1) {
        // Visual recognition that the cog is running
        PropWare::Pin::flash_pin((PropWare::Port::Mask) (1 << (cogid() + 16)), 3);

        pwSyncOut.printf("Hello from cog %d\n", cogid());
        nextCnt = waitcnt2(nextCnt, wait_time);
    }
    return 0;
}

void run_cog (void *arg) {
    uint32_t      nextCnt;
    const PropWare::Port::Mask pinMaskOfCogId = (PropWare::Port::Mask) (1 << (cogid() + 16));

    // wait for start signal from main cog
    while (!syncStart);

    nextCnt = wait_time + startCnt;
    while (1) {
        // Visual recognition that the cog is running
        PropWare::Pin::flash_pin(pinMaskOfCogId, 3);

        pwSyncOut.printf("Hello from cog %d\n", cogid());
        nextCnt = waitcnt2(nextCnt, wait_time);
    }
}
