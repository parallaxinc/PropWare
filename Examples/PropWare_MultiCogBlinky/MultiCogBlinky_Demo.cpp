/**
 * @file    MultiCogBlinky_Demo.cpp
 *
 * @author  Modified by David Zemon
 */

// Note the lack of an include for propeller.h; This is because PropWare.h will
// include propeller.h for you
#include <PropWare/PropWare.h>
#include <PropWare/pin.h>
#include <PropWare/uart/simplexuart.h>
#include <PropWare/printer/printer.h>

/**
* @brief       Toggle thread function gets started in an LMM COG.
*
* @param[in]   *arg    pin number to toggle
*/
void run_cog(void *arg);

const uint16_t         COGS       = 8;
const uint16_t         STACK_SIZE = 16;
static uint32_t        cog_stack[STACK_SIZE][COGS];
static _thread_state_t thread_data;

volatile uint32_t wait_time;
volatile uint32_t startCnt;
volatile int8_t   syncStart;

/**
 * @example     MultiCogBlinky_Demo.cpp
 *
 * Use PropGCC's native interface to blink an LED from each of the Propeller's 8 cogs. Each cog will be invoking
 * instructions out of HUB memory, not COG memory. For a simpler and more object-oriented way to start new cogs,
 * check out PropWare::Runnable
 *
 * @include PropWare_MultiCogBlinky/CMakeLists.txt
 */
int main(int argc, char *argv[]) {
    int8_t              n;
    int8_t              cog;
    PropWare::Pin       pin;
    uint32_t            nextCnt;
    PropWare::Pin::Mask pins[] = {
        PropWare::Port::P16,
        PropWare::Port::P17,
        PropWare::Port::P18,
        PropWare::Port::P19,
        PropWare::Port::P20,
        PropWare::Port::P21,
        PropWare::Port::P22,
        PropWare::Port::P23
    };

    wait_time = 50 * MILLISECOND;

    syncStart = 0;

    for (n = 1; n < COGS; n++) {
        cog = (int8_t) _start_cog_thread(cog_stack[n] + STACK_SIZE, run_cog, (void *) &pins[n], &thread_data);
        pwOut.printf("Toggle COG %d Started\n", cog);
    }

    pin.set_mask(pins[0]);
    pin.set_dir(PropWare::Pin::OUT);

    startCnt  = CNT;
    syncStart = 1;
    nextCnt   = wait_time + startCnt;
    while (1) {
        pin.set();
        nextCnt = waitcnt2(nextCnt, wait_time);
        pin.clear();
        nextCnt = waitcnt2(nextCnt, wait_time);
    }
}

void run_cog(void *arg) {
    PropWare::Pin pin;
    uint32_t      nextCnt;

    pin.set_mask(*(PropWare::Pin::Mask *) arg);
    pin.set_dir(PropWare::Pin::OUT);

    // wait for start signal from main cog
    while (syncStart == 0);

    nextCnt = wait_time + startCnt;
    while (1) {
        pin.toggle();
        nextCnt = waitcnt2(nextCnt, wait_time);
    }
}

