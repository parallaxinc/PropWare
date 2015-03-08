/**
 * @file    Runnable_Demo.cpp
 *
 * @author  Modified by David Zemon
 */

#include <PropWare/PropWare.h>
#include <PropWare/runnable.h>
#include <PropWare/printer/synchronousprinter.h>
#include <PropWare/pin.h>

class TalkingThread : public PropWare::Runnable {
    public:
        TalkingThread (const uint32_t *stack, const size_t stackSizeInBytes)
                : Runnable(stack, stackSizeInBytes) {}

        void run () {
            while (1) {
                pwSyncOut.printf("Hello from cog %u (0x%08X)! %u\n", cogid(), (unsigned int) this, CNT);
                waitcnt(250 * MILLISECOND + CNT);
            }
        }
};

class BlinkingThread : public PropWare::Runnable {
    public:
        BlinkingThread (const uint32_t *stack, const size_t stackSizeInBytes, const PropWare::Pin::Mask mask)
                : Runnable(stack, stackSizeInBytes),
                  m_mask(mask) {}

        void run () {
            const PropWare::Pin pin(this->m_mask, PropWare::Pin::OUT);
            while (1) {
                pin.toggle();
                waitcnt(250 * MILLISECOND + CNT);
            }
        }

    private:
        const PropWare::Pin::Mask m_mask;
};

int main (int argc, char *argv[]) {
    uint32_t     stack[3][32];
    TalkingThread talkingThread(stack[0], sizeof(stack[0]));
    BlinkingThread blink16(stack[1], sizeof(stack[1]), PropWare::Pin::P16);
    BlinkingThread blink17(stack[2], sizeof(stack[2]), PropWare::Pin::P17);

    int8_t cog = PropWare::Runnable::invoke(talkingThread);
    pwSyncOut.printf("Talking thread (0x%08X) started in cog %d\n", (unsigned int) &talkingThread, cog);

    cog = PropWare::Runnable::invoke(blink16);
    pwSyncOut.printf("Blink16 thread (0x%08X) started in cog %d\n", (unsigned int) &blink16, cog);

    cog = PropWare::Runnable::invoke(blink17);
    pwSyncOut.printf("Blink17 thread (0x%08X) started in cog %d\n", (unsigned int) &blink17, cog);

    while (1) {
        pwSyncOut.printf("Hello from cog %u! %u\n", cogid(), CNT);
        waitcnt(250 * MILLISECOND + CNT);
    }
}
