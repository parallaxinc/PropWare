/**
 * @file    Runnable_Demo.cpp
 *
 * @author  Modified by David Zemon
 */

#include <PropWare/concurrent/runnable.h>
#include <PropWare/PropWare.h>
#include <PropWare/hmi/output/synchronousprinter.h>
#include <PropWare/gpio/pin.h>

using PropWare::Runnable;
using PropWare::Pin;

class TalkingThread: public Runnable {
    public:
        template<size_t N>
        TalkingThread(const uint32_t (&stack)[N])
            : Runnable(stack) {
        }

        void run() {
            while (1) {
                pwSyncOut.printf("Hello from cog %u (0x%08X)! %u\n", cogid(), (unsigned int) this, CNT);
                waitcnt(250 * MILLISECOND + CNT);
            }
        }
};

class BlinkingThread: public Runnable {
    public:
        template<size_t N>
        BlinkingThread(const uint32_t (&stack)[N], const Pin::Mask mask)
            : Runnable(stack),
              m_mask(mask) {
        }

        void run() {
            const Pin pin(this->m_mask, Pin::Dir::OUT);
            while (1) {
                pin.toggle();
                waitcnt(250 * MILLISECOND + CNT);
            }
        }

    private:
        const Pin::Mask m_mask;
};

/**
 * @example     Runnable_Demo.cpp
 *
 * Run code in a total of four cogs. Two of them will simply blink LEDs. The other two demonstrate a thread-safe way to
 * use a serial terminal simultaneously from two different cogs.
 *
 * @include PropWare_Runnable/CMakeLists.txt
 */
int main(int argc, char *argv[]) {
    uint32_t       stack[3][70];
    TalkingThread  talkingThread(stack[0]);
    BlinkingThread blink16(stack[1], Pin::Mask::P16);
    BlinkingThread blink17(stack[2], Pin::Mask::P17);

    int8_t cog = Runnable::invoke(talkingThread);
    pwSyncOut.printf("Talking thread (0x%08X) started in cog %d\n", (unsigned int) &talkingThread, cog);

    cog = Runnable::invoke(blink16);
    pwSyncOut.printf("Blink16 thread (0x%08X) started in cog %d\n", (unsigned int) &blink16, cog);

    cog = Runnable::invoke(blink17);
    pwSyncOut.printf("Blink17 thread (0x%08X) started in cog %d\n", (unsigned int) &blink17, cog);

    while (1) {
        pwSyncOut.printf("Hello from cog %u! %u\n", cogid(), CNT);
        waitcnt(250 * MILLISECOND + CNT);
    }
}
