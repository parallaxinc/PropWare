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

// Includes
#include <PropWare/PropWare.h>
#include <PropWare/uart/halfduplexuart.h>
#include <PropWare/synchronousprinter.h>

uint8_t init_main_cog (_thread_state_t *threadData,
                       PropWare::SimplexUART *speaker);

void init_listener_cog (char buffer[], PropWare::HalfDuplexUART *listener);

void listen_silently (void *arg);

void error (const PropWare::ErrorCode err);

// Create an easy-to-test number pattern - useful when testing with a logic
// analyzer
char g_numberPattern[] = {
        0x01,
        0x02,
        0x03,
        0x45,
        0xe5,
        0xaa,
        0xff,
        0x80,
        0x00};

// Create the test string - useful when testing with a terminal
const char                   TEST_STRING[] = "Hello, world!\n";
const uint32_t               BAUD_RATE     = 200;
const PropWare::Port::Mask   TX_PIN        = PropWare::Port::P12;
const PropWare::Port::Mask   RX_PIN        = PropWare::Port::P13;
const PropWare::UART::Parity PARITY        = PropWare::UART::NO_PARITY;
const uint16_t               STACK_SIZE    = 256;
uint32_t                     threadStack[STACK_SIZE];

/**
 * @brief   Write "Hello world!" out via UART protocol and receive an echo
 */
int main () {
    static _thread_state_t threadData;
    PropWare::SimplexUART  speaker(TX_PIN);

    // Start our new cog and initialize the speaking UART
    const uint8_t cog = init_main_cog(&threadData, &speaker);
//    pwSyncOut.printf("New cog %u. Ready to send!!!" CRLF, cog); // TODO

    while (1) {
        waitcnt(500 * MILLISECOND + CNT);
        speaker.puts(TEST_STRING);
    }
}

uint8_t init_main_cog (_thread_state_t *threadData,
        PropWare::SimplexUART *speaker) {
    speaker->set_baud_rate(BAUD_RATE);
    speaker->set_parity(PARITY);

    return (uint8_t) _start_cog_thread(threadStack + STACK_SIZE,
            listen_silently, (void *) NULL, threadData);
}

void listen_silently (void *arg) {
    PropWare::ErrorCode      err;
    char                     buffer[sizeof(TEST_STRING)];
    PropWare::HalfDuplexUART listener(RX_PIN);
    int32_t                  chars;

    // Initialize the listener UART and clear the buffer
    init_listener_cog(buffer, &listener);
//    pwSyncOut.puts("Ready to receive!" CRLF); // TODO

    while (1) {
        chars = 0;
        if ((err = listener.fgets(buffer, &chars)))
            error(err);

//        pwSyncOut.printf("Data (%d chars): \"%s\"" CRLF, chars, buffer); // TODO
    }
}

void init_listener_cog (char buffer[], PropWare::HalfDuplexUART *listener) {
    listener->set_baud_rate(BAUD_RATE);
    listener->set_parity(PARITY);
    memset(buffer, 0, sizeof(TEST_STRING));

    // A very short wait to ensure the main cog has finished printing its "I'm
    // ready" statement before we start printing ours
    waitcnt(10 * MILLISECOND + CNT);
}

void error (const PropWare::ErrorCode err) {
    PropWare::SimplePort debugLEDs(PropWare::Port::P16, 8, PropWare::Pin::OUT);

//    pwSyncOut.printf("Unknown error: %u" CRLF, err); // TODO

    while (1) {
        debugLEDs.write((uint32_t) err);
        waitcnt(100 * MILLISECOND);
        debugLEDs.write(0);
        waitcnt(100 * MILLISECOND);
    }
}
