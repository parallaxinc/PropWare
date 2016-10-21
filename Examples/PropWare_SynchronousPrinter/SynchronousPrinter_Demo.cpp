/**
 * @file    SynchronousPrinter_Demo.cpp
 *
 * @author  David Zemon
 */

#include <PropWare/concurrent/runnable.h>
#include <PropWare/PropWare.h>
#include <PropWare/hmi/output/synchronousprinter.h>
#include <PropWare/gpio/pin.h>

static const uint16_t     COGS             = 8;
static const uint16_t     STACK_SIZE       = 128;
static const unsigned int DELAY_IN_SECONDS = 2;
static const uint32_t     WAIT_TIME        = DELAY_IN_SECONDS * SECOND;

class SyncedPrinterCog: public PropWare::Runnable {
    public:
        template<size_t N>
        SyncedPrinterCog(const uint32_t (&stack)[N])
            : Runnable(stack) {
        }


        virtual void run() {
            const PropWare::Port::Mask pinMaskOfCogId = (PropWare::Port::Mask) (1 << (cogid() + 16));
            uint32_t                   nextCnt;

            nextCnt = WAIT_TIME + CNT;
            while (1) {
                // Visual recognition that the cog is running
                PropWare::Pin::flash_pin(pinMaskOfCogId, 3);

                pwSyncOut.printf("Hello from cog %d\n", cogid());
                nextCnt = waitcnt2(nextCnt, WAIT_TIME);
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
    const uint32_t   stacks[COGS][STACK_SIZE] = {{0}};
    SyncedPrinterCog syncedPrinterCogs[]      = {
        SyncedPrinterCog(stacks[0]),
        SyncedPrinterCog(stacks[1]),
        SyncedPrinterCog(stacks[2]),
        SyncedPrinterCog(stacks[3]),
        SyncedPrinterCog(stacks[4]),
        SyncedPrinterCog(stacks[5]),
        SyncedPrinterCog(stacks[6]),
        SyncedPrinterCog(stacks[7])
    };

    // If the comm port was not initialized successfully, just sit here and complain
    if (!pwSyncOut.has_lock())
        while (1)
            PropWare::Port::flash_port(PropWare::BYTE_2, PropWare::BYTE_2);

    for (uint8_t n = 1; n < COGS; n++)
        PropWare::Runnable::invoke(syncedPrinterCogs[n]);

    syncedPrinterCogs[0].run();
}
