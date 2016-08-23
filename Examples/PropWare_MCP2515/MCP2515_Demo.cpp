/**
 * @file    MCP2515_Demo.cpp
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
#include <PropWare/hmi/output/printer.h>
#include <PropWare/serial/can/mcp2515.h>

using namespace PropWare;

const Port::Mask MOSI         = Port::Mask::P0;
const Port::Mask MISO         = Port::Mask::P1;
const Port::Mask SCLK         = Port::Mask::P2;
const Port::Mask CS           = Port::Mask::P7;
const Port::Mask CLOCK_MASK   = Port::Mask::P8;

const uint8_t messages[][6] = {
        "Hello",
        "David",
        "Katie"
};

void handle (const ErrorCode err) {
    if (err) {
        pwOut.printf("ERROR!!! %d\n", err);
        while (1)
            waitcnt(0);
    }
}

void read (MCP2515 &can, const MCP2515::BufferNumber bufferNumber) {
    if (can.check_receive_buffer(bufferNumber)) {
        uint8_t buffer[MCP2515::MAX_DATA_BYTES];
        uint8_t bytesRead;
        can.read_message(bufferNumber, &bytesRead, buffer);
        pwOut << "Message: `" << (const char *) buffer << "`\n";
    } else {
        pwOut << "No message\n";
    }
}

/**
 * @example     MCP2515_Demo.cpp
 *
 * @include     PropWare_MCP2515/CMakeLists.txt
 */
int main () {
    // Instantiate the SPI bus and CAN controller instances
    const SPI spi(MOSI, MISO, SCLK);
    MCP2515   can(spi, CS);

    // We'll use the Propeller's hardware counters as a clock source for the MCP2515
    const Pin clock(CLOCK_MASK, Pin::Dir::OUT);
    clock.start_hardware_pwm(8000000);

    // Start the MCP2515 running a 1 Mbaud and in "loopback" mode. This means that all messages "sent" will be
    // immediately looped back into the receive buffers. This is great for testing your configuration while still at
    // your desk, to ensure that the filters and masks have been correctly configured, along with any other
    // configuration. Once ready, remove the optional `mode` parameter and connect to your live system.
    handle(can.start(MCP2515::BaudRate::BAUD_1000KBPS, MCP2515::Mode::LOOPBACK));

    // Set up the filters and masks so that only message ID 2 is allowed through
    handle(can.set_mask(MCP2515::BufferNumber::BUFFER_0, WORD_0));
    handle(can.set_filter(MCP2515::FilterNumber::FILTER_0, 2));

    pwOut << "Expected message received:\n";
    handle(can.send_message(2, 6, messages[0]));
    read(can, MCP2515::BufferNumber::BUFFER_0);

    pwOut << "Message should _not_ be received!\n";
    handle(can.send_message(3, 6, messages[1]));
    read(can, MCP2515::BufferNumber::BUFFER_0);

    pwOut << "Expected message received:\n";
    handle(can.send_message(2, 6, messages[2]));
    read(can, MCP2515::BufferNumber::BUFFER_0);
}
