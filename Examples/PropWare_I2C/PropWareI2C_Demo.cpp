/**
 * @file    PropWareI2C_Demo.cpp
 *
 * @author  David Zemon
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

#include <PropWare/i2c.h>
#include <PropWare/printer/printer.h>
#include <simpletools.h>

const uint8_t MAGIC_ARRAY[] = "DCBA0";
const size_t  ARRAY_SIZE    = sizeof(MAGIC_ARRAY);

const uint8_t  SHIFTED_DEVICE_ADDR = EEPROM_ADDR << 1;
const uint16_t TEST_ADDRESS        = 32 * 1024; // Place the data immediately above the first 32k of data

/**
 * @example     PropWareI2C_Demo.cpp
 *
 * Use an easy I2C class to communicate with an EEPROM chip
 *
 * @include PropWare_I2C/CMakeLists.txt
 */
int main () {
    const PropWare::I2C pwI2C;
    pwOut << "EEPROM ack = " << pwI2C.ping(SHIFTED_DEVICE_ADDR) << '\n';

    bool success = pwI2C.put(SHIFTED_DEVICE_ADDR, TEST_ADDRESS, MAGIC_ARRAY, ARRAY_SIZE);
    pwOut << "Put status: " << success << '\n';

    // Wait for write to finish
    while (!pwI2C.ping(SHIFTED_DEVICE_ADDR));

    uint8_t buffer[ARRAY_SIZE];
    success &= pwI2C.get(SHIFTED_DEVICE_ADDR, TEST_ADDRESS, buffer, ARRAY_SIZE);
    pwOut << "Get status: " << success << '\n';

    pwOut << "Returned string = '" << (char *) buffer << "'\n";

    return 0;
}
