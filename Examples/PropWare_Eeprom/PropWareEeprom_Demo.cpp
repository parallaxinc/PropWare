/**
 * @file    PropWareEeprom_Demo.cpp
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

#include <PropWare/printer/printer.h>
#include <PropWare/eeprom.h>
#include <PropWare/scanner.h>

const uint8_t  MAGIC_ARRAY_1[] = "DCBA0";
const size_t   ARRAY_SIZE_1    = sizeof(MAGIC_ARRAY_1);
const char     MAGIC_ARRAY_2[] = "Hello, world!";
const size_t   ARRAY_SIZE_2    = sizeof(MAGIC_ARRAY_2);
const uint16_t TEST_ADDRESS    = 32 * 1024; // Place the data immediately above the first 32k of data

/**
 * @example     PropWareEeprom_Demo.cpp
 *
 * Read from and write to the EEPROM that comes with your Propeller
 *
 * @include PropWare_Eeprom/CMakeLists.txt
 */
int main() {
    PropWare::Eeprom eeprom;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Here we have some core access of the EEPROM, passing it the address with every call. This is great for non-ASCII
    // data, but it gets tedious.

    pwOut << "EEPROM ack = " << eeprom.ping() << '\n';

    bool success = eeprom.put(TEST_ADDRESS, MAGIC_ARRAY_1, ARRAY_SIZE_1);
    pwOut << "Put status: " << success << '\n';

    pwOut << "Received character: " << (char) eeprom.get(TEST_ADDRESS) << '\n';
    pwOut << "Received character: " << (char) eeprom.get(TEST_ADDRESS + 1) << '\n';
    pwOut << "Received character: " << (char) eeprom.get(TEST_ADDRESS + 2) << '\n';
    pwOut << "Received character: " << (char) eeprom.get(TEST_ADDRESS + 3) << '\n';
    pwOut << "Received character: " << (char) eeprom.get(TEST_ADDRESS + 4) << '\n';

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // This is great for ASCII data, because we have access to the Printer and Scanner classes.

    pwOut << "Notice that PropWare::Eeprom also implements the PropWare::PrintCapable \n"
        "and PropWare::ScanCapable interfaces. So we could also use the Propware::Printer \n"
        "and PropWare::Scanner objects for reading and writing.\n";


    PropWare::Printer eepromPrinter(&eeprom);
    PropWare::Scanner eepromScanner(&eeprom);

    // Reset the EEPROM address
    eeprom.set_memory_address(TEST_ADDRESS);
    // Note that the newline is required, or else the scanner won't know when to stop reading
    eepromPrinter << MAGIC_ARRAY_2 << '\n';

    char buffer[ARRAY_SIZE_2];
    // Reset the EEPROM address so that we can read what we just wrote
    eeprom.set_memory_address(TEST_ADDRESS);
    // Read from the EEPROM
    eepromScanner.gets(buffer, ARRAY_SIZE_2);
    pwOut << "Received \"" << buffer << "\" from the EEPROM!\n";

    return 0;
}
