/**
 * @file    PropWareI2CSlave_Demo.cpp
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

#include <PropWare/serial/i2c/i2c.h>
#include <PropWare/hmi/output/printer.h>
#include <simpletools.h>

static const uint8_t MAGIC_ARRAY_1[] = "DCBA0";
static const size_t  ARRAY_SIZE_1    = sizeof(MAGIC_ARRAY_1);

static const uint8_t  SHIFTED_DEVICE_ADDR = EEPROM_ADDR << 1;
static const uint16_t TEST_ADDRESS        = 32 * 1024; // Place the data immediately above the first 32k of data





/**
 * @brief	Callback fired, when the master requests data from us (the slave).
 * 			In this method, you should answer the master through the slave's write() method
 */
void onRequest(PropWare::I2CSlave* slave) {
	slave->write('h');
	slave->write('e');
	slave->write('l');
	slave->write('l');
	slave->write('o');
	slave->write('!');
}

/**
 * @brief	Callback fired, when the master sent us some nice packet of fresh data.
 * 			In this method, you should take the data received from the master out of the receive buffer.
 */
void onReceive(PropWare::I2CSlave* slave) {
	while(slave->available()) {
		char c = slave->read();
		//do stuff
	}
}



/**
 * @example     PropWareI2CSlave_Demo.cpp
 *
 * Use an easy I2CSlave class to immitate an i2c slave with address 42
 *
 * @include PropWare_I2CSlave/CMakeLists.txt
 */
int main () {
	// create a new i2cslave with address 42 and a receive buffer taking a maximum of 64bytes from pins (scl = P0), (sda = P1)
	PropWare::I2CSlave slave(42, 64, PropWare::Pin::Mask::P0, PropWare::Pin::Mask::P1);

	// register the bus callbacks that will be fired on incomming requests or data
	slave.setOnRequest(onRequest);
	slave.setOnReceive(onReceive);

	//start the main loop and wait for events
	slave.run();
}
