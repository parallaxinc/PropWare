/**
 * @file    I2CSlave_Demo.cpp
 *
 * @author  Markus Ebner
 *
 * @copyright
 * The MIT License (MIT)<br>
 * <br>Copyright (c) 2013 David Zemon<br>
 * <br>Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
 * to permit persons to whom the Software is furnished to do so, subject to the following conditions:<br>
 * <br>The above copyright notice and this permission notice shall be included in all copies or substantial portions
 * of the Software.<br>
 * <br>THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 * TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <PropWare/hmi/output/printer.h>
#include <PropWare/serial/i2c/i2cslave.h>
#include <PropWare/serial/i2c/i2cmaster.h>
#include <PropWare/utility/collection/queue.h>
#include <PropWare/gpio/simpleport.h>

using PropWare::Queue;
using PropWare::I2CMaster;
using PropWare::I2CSlave;
using PropWare::SimplePort;
using PropWare::Port;
using PropWare::Pin;
using PropWare::Runnable;

class MyI2CSlave: public I2CSlave {
    public:
        static const int ADDRESS         = 42;
        static const int SHIFTED_ADDRESS = ADDRESS << 1;

    public:
        template<size_t I2C_BUFFER_SIZE, size_t STACK_SIZE, size_t QUEUE_BUFFER_SIZE>
        MyI2CSlave (uint8_t (&buffer)[I2C_BUFFER_SIZE], const uint32_t (&stack)[STACK_SIZE],
                    uint8_t (&queueBuffer)[QUEUE_BUFFER_SIZE])
            : I2CSlave(ADDRESS, buffer, stack),
              m_queue(queueBuffer),
              m_sum(0) {
        }

    private:
        /**
         * @brief	Callback fired, when the master requests data from us (the slave).
         *
         * In this method, you should answer the master through the slave's write() method
         */
        void on_request () {
            while (!this->m_queue.is_empty())
                this->m_sum += this->m_queue.dequeue();
            this->write(this->m_sum);
        }

        /**
         * @brief	Callback fired, when the master sent us some nice packet of fresh data.
         *
         * In this method, you should take the data received from the master out of the receive buffer.
         */
        void on_receive () {
            int result;
            while (-1 != (result = this->read()))
                this->m_queue.insert(static_cast<uint8_t>(result));
        }

    private:
        Queue<uint8_t> m_queue;
        uint8_t        m_sum;
};

/**
 * @example     I2CSlave_Demo.cpp
 *
 * Use an easy I2CSlave class to imitate an I2C slave with address 42
 *
 * @include PropWare_I2CSlave/CMakeLists.txt
 */
int main () {
    uint8_t    buffer[32];
    uint32_t   stack[128];
    uint8_t    queueBuffer[32];
    MyI2CSlave slave(buffer, stack, queueBuffer);

    // Start a new cog to monitor the I2C bus and respond to events
    Runnable::invoke(slave);

    // Run the master
    I2CMaster master;

    // Set the master to a very low frequency. The on_request() and on_receive() methods in this sample are not
    // optimized for high speed communications, and extreme optimizations may be necessary in order to run a synchronous
    // serial bus like I2C in slave mode. Those types of optimizations are beyond the scope of this demo, so we will
    // simply set the frequency low enough that they are unnecessary.
    master.set_frequency(1000);

    if (master.ping(MyI2CSlave::SHIFTED_ADDRESS)) {
        pwOut << "ACK received!\n";

        pwOut << "Expecting  0: " << master.get(MyI2CSlave::SHIFTED_ADDRESS, static_cast<uint8_t>(0)) << '\n';
        pwOut << "Expecting  1: " << master.get(MyI2CSlave::SHIFTED_ADDRESS, static_cast<uint8_t>(1)) << '\n';
        pwOut << "Expecting  3: " << master.get(MyI2CSlave::SHIFTED_ADDRESS, static_cast<uint8_t>(2)) << '\n';
        pwOut << "Expecting  6: " << master.get(MyI2CSlave::SHIFTED_ADDRESS, static_cast<uint8_t>(3)) << '\n';
        pwOut << "Expecting 10: " << master.get(MyI2CSlave::SHIFTED_ADDRESS, static_cast<uint8_t>(4)) << '\n';
        pwOut << "Expecting 80: " << master.get(MyI2CSlave::SHIFTED_ADDRESS, static_cast<uint16_t>(0x1234)) << '\n';
    } else
        pwOut << "No ack! :(\n I guess we're done.\n";
}
