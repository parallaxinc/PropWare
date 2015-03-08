/**
 * @file    DuplexUART_Demo.cpp
 *
 * @author  David Zemon
 *
 * @copyright
 * The MIT License (MIT)<br>
 * <br>Copyright (c) 2013 David Zemon<br>
 * <br>Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:<br>
 * <br>The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.<br>
 * <br>THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <PropWare/PropWare.h>
#include <PropWare/uart/halfduplexuart.h>
#include <PropWare/printer/synchronousprinter.h>
#include <PropWare/runnable.h>

// Create the test string - useful when testing with a terminal
const char                   TEST_STRING[] = "Hello, world!\n";
const uint32_t               BAUD_RATE     = 200;
const PropWare::Port::Mask   TX_PIN        = PropWare::Port::P12;
const PropWare::Port::Mask   RX_PIN        = PropWare::Port::P13;
const PropWare::UART::Parity PARITY        = PropWare::UART::NO_PARITY;
uint32_t                     threadStack[256];

class Listener : public PropWare::Runnable {
    public:
        Listener(uint32_t const *stack, size_t const stackSizeInBytes)
                : Runnable(stack, stackSizeInBytes) {
        }

        void run();

        void init();

    private:
        PropWare::HalfDuplexUART m_listener;
        char buffer[sizeof(TEST_STRING)];
};

void error (const PropWare::ErrorCode err);

/**
 * @brief   Write "Hello world!" out via UART protocol and receive an echo
 */
int main () {
    Listener listener(threadStack, sizeof(threadStack));
    PropWare::SimplexUART  speaker(TX_PIN);

    // Start our new cog and initialize the speaking UART
    speaker.set_baud_rate(BAUD_RATE);
    speaker.set_parity(PARITY);
    pwSyncOut.printf("New cog ID: %d. Ready to send!!!\n", PropWare::Runnable::invoke(listener));

    while (1) {
        waitcnt(5 * MILLISECOND + CNT);
        speaker.puts(TEST_STRING);
    }
}

void error (const PropWare::ErrorCode err) {
    PropWare::SimplePort debugLEDs(PropWare::Port::P16, 8, PropWare::Pin::OUT);

    pwSyncOut.printf("Unknown error: %u\n", err);

    while (1) {
        debugLEDs.write((uint32_t) err);
        waitcnt(100 * MILLISECOND);
        debugLEDs.write(0);
        waitcnt(100 * MILLISECOND);
    }
}

void Listener::run() {
    PropWare::ErrorCode err;
    int32_t receivedLength;

    this->init();
    pwSyncOut.printf("Ready to receive!\n");

    while (1) {
        receivedLength = 0;
        if ((err = this->m_listener.fgets(buffer, &receivedLength)))
            error(err);

        pwSyncOut.printf("Data (%d chars): \"%s\"\n", receivedLength, buffer);
    }
}

void Listener::init() {
    this->m_listener.set_rx_mask(RX_PIN);
    this->m_listener.set_baud_rate(BAUD_RATE);
    this->m_listener.set_parity(PARITY);
    memset(buffer, 0, sizeof(buffer));

    // A very short wait to ensure the main cog has finished printing its "I'm ready" statement before we start
    // printing ours
    waitcnt(10 * MILLISECOND + CNT);
}
