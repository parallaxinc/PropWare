/**
 * @file    SynchronousPrinter_Demo.cpp
 *
 * @author  David Zemon
 */

#include <PropWare/runnable.h>
#include <PropWare/PropWare.h>
#include <PropWare/printer/synchronousprinter.h>
#include <PropWare/port.h>
#include <PropWare/pin.h>

const uint16_t EXTRA_COGS = 7;
const uint16_t STACK_SIZE = 128;

const uint32_t    wait_time = SECOND;
volatile bool     syncStart = false;
volatile uint32_t startCnt;

class ExtraPrintingCog: public PropWare::Runnable {
    public:
        template<size_t N>
        ExtraPrintingCog(const uint32_t (&stack)[N])
            : Runnable(stack) {
        }


        virtual void run() override {
            const PropWare::Port::Mask pinMaskOfCogId = (PropWare::Port::Mask) (1 << (cogid() + 16));
            uint32_t                   nextCnt;

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
};

/**
 * @example     SynchronousPrinter_Demo.cpp
 *
 * Demonstrate multiple cogs printing to the same serial terminal simultaneously
 *
 * @include PropWare_SynchronousPrinter/CMakeLists.txt
 */
int main(int argc, char *argv[]) {
    int8_t           n;
    int8_t           cog;
    uint32_t         nextCnt;
    const uint32_t   stacks[EXTRA_COGS][STACK_SIZE] = {{0}};
    ExtraPrintingCog extraCogs[]                        = {
        ExtraPrintingCog(stacks[0]),
        ExtraPrintingCog(stacks[1]),
        ExtraPrintingCog(stacks[2]),
        ExtraPrintingCog(stacks[3]),
        ExtraPrintingCog(stacks[4]),
        ExtraPrintingCog(stacks[5]),
        ExtraPrintingCog(stacks[6])
    };

    // If the comm port was not initialized successfully,
    // just sit here and complain
    if (!pwSyncOut.has_lock())
        while (1)
            PropWare::Port::flash_port(PropWare::BYTE_2, PropWare::BYTE_2);

    for (n = 0; n < EXTRA_COGS; n++) {
        cog = PropWare::Runnable::invoke(extraCogs[n]);
        pwSyncOut.printf("Toggle COG %d Started\n", cog);
    }

    startCnt  = CNT;
    syncStart = true;
    nextCnt   = wait_time + startCnt;
    while (1) {
        // Visual recognition that the cog is running
        PropWare::Pin::flash_pin((PropWare::Port::Mask) (1 << (cogid() + 16)), 3);

        pwSyncOut.printf("Hello from cog %d\n", cogid());
        nextCnt = waitcnt2(nextCnt, wait_time);
    }
}
