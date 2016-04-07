/**
 * @file    Keypad_Demo.cpp
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

#include <PropWare/hmi/input/keypad.h>
#include <PropWare/hmi/output/printer.h>

using namespace PropWare;

/**
 * @example     PropWare_Keypad/Keypad_Demo.cpp
 *
 * Read keys from a common 4x4 keypad to interact with a user
 *
 * @include PropWare_Keypad/CMakeLists.txt
 */
int main() {
    Keypad::Key keys[] = {
        Keypad::Key('1'), Keypad::Key('2'), Keypad::Key('3'), Keypad::Key('A'),
        Keypad::Key('4'), Keypad::Key('5'), Keypad::Key('6'), Keypad::Key('B'),
        Keypad::Key('7'), Keypad::Key('8'), Keypad::Key('9'), Keypad::Key('C'),
        Keypad::Key('*'), Keypad::Key('0'), Keypad::Key('#'), Keypad::Key('D')
    };
    const Pin   rowPins[]       = {
        Pin(Pin::P19),
        Pin(Pin::P20),
        Pin(Pin::P21),
        Pin(Pin::P22),
    };
    const Pin   columnPins[]    = {
        Pin(Pin::P26),
        Pin(Pin::P25),
        Pin(Pin::P24),
        Pin(Pin::P23),
    };

    Keypad keypad(keys, rowPins, columnPins);

    while (1) {
        //for (auto key : keys) {
        //    if (keypad.is_pressed(key.get_character()))
        //        pwOut << key.get_character() << '\n';
        //}

        keypad.get_keys();
        for (size_t i = 0; i < Utility::size_of_array(keys); ++i) {
            if (0 == i % 4)
                pwOut << '\n';
            pwOut << (keys[i].get_state() ? keys[i].get_character() : ' ') << ' ';
        }

        waitcnt(100*MILLISECOND + CNT);
    }
}
