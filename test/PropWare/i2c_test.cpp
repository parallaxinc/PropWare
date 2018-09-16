/**
 * @file    i2c_test.cpp
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

#include <PropWare/utility/collection/queue.h>
#include "PropWareTests.h"
#include <PropWare/serial/i2c/i2cmaster.h>
#include <PropWare/serial/i2c/i2cslave.h>

using PropWare::I2CMaster;
using PropWare::I2CSlave;
using PropWare::Pin;
using PropWare::Queue;
using PropWare::Runnable;

uint8_t  slaveBuffer[32];
uint8_t  queueBuffer[32];
uint32_t slaveStack[128];

class I2CSlaveTester: public I2CSlave {
    public:
        I2CSlaveTester (const uint8_t address)
            : I2CSlave(address, slaveBuffer, slaveStack),
              m_queue(queueBuffer),
              m_sum(0) {
        }

        void on_request () {
            while (!this->m_queue.is_empty())
                this->m_sum += this->m_queue.dequeue();
            this->write(this->m_sum);
        }

        void on_receive () {
            int result;
            while (-1 != (result = this->read()))
                this->m_queue.insert(static_cast<uint8_t>(result));
        }

    private:
        Queue<uint8_t> m_queue;
        uint8_t        m_sum;
};

void setUp() {
    memset(slaveBuffer, 32, 0);
    memset(queueBuffer, 32, 0);
    memset(slaveStack, 128*sizeof(slaveBuffer[0]), 0);
}

TEST(Master_Constructor_shouldSetDefaults) {
    setUp();

    I2CMaster testable;

    ASSERT_EQ(Pin::Mask::P28, testable.m_scl.get_mask());
    ASSERT_EQ(Pin::Mask::P29, testable.m_sda.get_mask());

    ASSERT_EQ(Pin::Dir::IN, testable.m_scl.get_dir());
    ASSERT_EQ(Pin::Dir::IN, testable.m_sda.get_dir());
}

TEST(Slave_Constructor_shouldSetDefaults) {
    setUp();

    const uint8_t expectedSlaveAddress = 0x12;

    I2CSlaveTester testable(expectedSlaveAddress);

    ASSERT_EQ(Pin::Mask::P28, testable.m_scl.get_mask());
    ASSERT_EQ(Pin::Mask::P29, testable.m_sda.get_mask());

    ASSERT_EQ(Pin::Dir::IN, testable.m_scl.get_dir());
    ASSERT_EQ(Pin::Dir::IN, testable.m_sda.get_dir());

    ASSERT_EQ(expectedSlaveAddress, testable.m_slaveAddress);
    ASSERT_EQ(slaveBuffer, testable.m_buffer);
    ASSERT_EQ(31, testable.m_bufferUpperBound)
}

TEST(MasterSlaveCommunication) {
    setUp();

    const uint8_t slaveAddress = 0x12;
    const uint8_t shiftedSlaveAddress = slaveAddress << 1;
    I2CSlaveTester  slave(slaveAddress);
    Runnable::invoke(slave);

    I2CMaster master;
    master.set_frequency(1000); // Low frequency is necessary for the slave to keep up

    ASSERT_TRUE(master.ping(shiftedSlaveAddress));

    ASSERT_EQ_MSG(0, master.get(shiftedSlaveAddress, static_cast<uint8_t>(0)));
    ASSERT_EQ_MSG(1, master.get(shiftedSlaveAddress, static_cast<uint8_t>(1)));
    ASSERT_EQ_MSG(3, master.get(shiftedSlaveAddress, static_cast<uint8_t>(2)));
    ASSERT_EQ_MSG(6, master.get(shiftedSlaveAddress, static_cast<uint8_t>(3)));
    ASSERT_EQ_MSG(10, master.get(shiftedSlaveAddress, static_cast<uint8_t>(4)));
    ASSERT_EQ_MSG(80, master.get(shiftedSlaveAddress, static_cast<uint16_t>(0x1234)));
}

int main () {
    START(I2CTest);

    RUN_TEST(Master_Constructor_shouldSetDefaults);
    RUN_TEST(Slave_Constructor_shouldSetDefaults);
    RUN_TEST(MasterSlaveCommunication);

    COMPLETE();
}
