/**
 * @file    Hello_Demo.cpp
 *
 * @author  David Zemon
 */

// Includes
#include <PropWare/PropWare.h>
#include <PropWare/synchronousprinter.h>

void run_cog (void *arg);

const uint16_t         COGS       = 8;
const uint16_t         STACK_SIZE = 16;
static uint32_t        cog_stack[STACK_SIZE][COGS];
static _thread_state_t thread_data;

volatile uint32_t wait_time;
volatile uint32_t startCnt;
volatile bool   syncStart = false;
volatile uint8_t cogNum[] = {0, 1, 2, 3, 4, 5, 6, 7};

int main (int argc, char* argv[]) {
    int8_t   n;
    int8_t   cog;
    uint32_t nextCnt;

    wait_time = 500 * MILLISECOND;

    for (n = 1; n < COGS; n++) {
        cog = (int8_t) _start_cog_thread(cog_stack[n] + STACK_SIZE, run_cog,
                                         (void *) &cogNum[n], &thread_data);
        pwSyncOut.printf("Toggle COG %d Started" CRLF, cog);
    }

    startCnt = CNT;
    syncStart = true;
    nextCnt = wait_time + startCnt;
    while (1) {
        pwSyncOut.printf("Hello from cog %u" CRLF, cogNum[0]);
        nextCnt = waitcnt2(nextCnt, wait_time);
    }
    return 0;
}

void run_cog (void *arg) {
    const uint8_t cog = *(uint8_t *) arg;
    uint32_t      nextCnt;

    // wait for start signal from main cog
    while (!syncStart);

    nextCnt = wait_time + startCnt;
    while (1) {
        pwSyncOut.printf("Hello from cog %u" CRLF, cog);
        nextCnt = waitcnt2(nextCnt, wait_time);
    }
}
