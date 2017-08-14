/**
 * @file    BNO055_Demo.cpp
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

#include <PropWare/sensor/fusion/bno055.h>
#include <Simple-Libraries/Learn/Simple Libraries/Protocol/libsimplei2c/simplei2c.h>

using PropWare::I2CMaster;
using PropWare::BNO055;
using PropWare::Pin;

uint8_t get_byte (i2c *bus) {
    i2c_start(bus);
    i2c_writeByte(bus, BNO055::DEVICE_ADDRESS_A);
    i2c_writeByte(bus, BNO055::convert(BNO055::Address::CHIP_ID));
    const uint8_t result = static_cast<uint8_t>(i2c_readByte(bus, 1));
    i2c_stop(bus);
    return result;
}

PropWare::ErrorCode begin (i2c *bus) {
    auto id = get_byte(bus);
    if (id != BNO055::ID) {
        waitcnt(SECOND + CNT); // hold on for boot
        id = get_byte(bus);
        if (id != BNO055::ID) {
            pwOut << "fail 0\n";
            return BNO055::FAILED_TO_BOOT;
        }
    }
    return 0;
}

int ping (i2c *bus) {
    return i2c_poll(bus, BNO055::DEVICE_ADDRESS_A);
}

/**
 * @example     BNO055_Demo.cpp
 *
 *
 * @include PropWare_BNO055/CMakeLists.txt
 */
int main () {
    PropWare::ErrorCode error;
    const Pin resetPin(PropWare::Port::P23, Pin::Dir::OUT);
    waitcnt(100 * MILLISECOND + CNT);

    i2c bus;
    i2c_open(&bus, Pin::from_mask(I2CMaster::DEFAULT_SCL_MASK), Pin::from_mask(I2CMaster::DEFAULT_SDA_MASK), 0);

    for (unsigned int   i = 0; i < 5; ++i) {
        resetPin.clear();
        waitcnt(MILLISECOND + CNT);
        resetPin.set();

        while (ping(&bus))
            waitcnt(10 * MILLISECOND + CNT);
        int ack = i2c_writeByte(&bus, 0);
        i2c_readByte(&bus, 0);
        i2c_stop(&bus);
        waitcnt(10 * MILLISECOND + CNT);

        error = begin(&bus);
        if (!error)
            break;
    }

    //BNO055 sensor;
    //for (int            i = 0; i < 5; ++i) {
    //    resetPin.clear();
    //    waitcnt(MILLISECOND + CNT);
    //    resetPin.set();
    //
    //    while (!pwI2c.ping(BNO055::DEVICE_ADDRESS_A))
    //        waitcnt(10 * MILLISECOND + CNT);
    //    pwI2c.send_byte(0);
    //    pwI2c.get(BNO055::DEVICE_ADDRESS_A, static_cast<uint8_t>(0));
    //    pwI2c.stop();
    //    waitcnt(10 * MILLISECOND + CNT);
    //
    //    error = sensor.begin();
    //    if (!error)
    //        break;
    //}

    if (error)
        pwOut << "Error: " << error << '\n';
    else
        while (1) {
            pwOut << "Temperature = " << /*sensor.get_temperature() <<*/ '\n';
            waitcnt(SECOND + CNT);
        }
}
