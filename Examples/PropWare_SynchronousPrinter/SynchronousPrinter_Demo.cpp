/**
 * @file    Hello_Demo.cpp
 *
 * @author  David Zemon
 */

// Includes
#include <PropWare/PropWare.h>
#include <PropWare/synchronousprinter.h>
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

// TODO: This should probably be fixed whenever SynchronousPrinter is complete
int main (int argc, char *argv[]) {
    int8_t   n;
    int8_t   cog;
    uint32_t nextCnt;

    // If the comm port was not initialized successfully,
    // just sit here and complain
    if (!pwSyncOut.hasLock())
        while (1)
            PropWare::Port::flash_port(PropWare::BYTE_2, PropWare::BYTE_2);

    for (n = 1; n < COGS; n++) {
        cog = (int8_t) _start_cog_thread(cog_stack[n] + sizeof(cog_stack[n]),
                                         run_cog, nullptr, &thread_data);
//        pwSyncOut.printf("Toggle COG %d Started" CRLF, cog);
    }

    startCnt = CNT;
    syncStart = true;
    nextCnt = wait_time + startCnt;
    while (1) {
        // Visual recognition that the cog is running
        PropWare::Pin::flash_pin(
                (PropWare::Port::Mask) (1 << (cogid() + 16)), 3);

//        pwSyncOut.printf("Hello from cog %d" CRLF, cogid());
        nextCnt = waitcnt2(nextCnt, wait_time);
    }
    return 0;
}

void run_cog (void *arg) {
    uint32_t      nextCnt;

    // wait for start signal from main cog
    while (!syncStart);

    nextCnt = wait_time + startCnt;
    while (1) {
        // Visual recognition that the cog is running
        PropWare::Pin::flash_pin(
                (PropWare::Port::Mask) (1 << (cogid() + 16)), 3);

//        pwSyncOut.printf("Hello from cog %d" CRLF, cogid());
        nextCnt = waitcnt2(nextCnt, wait_time);
    }
}
