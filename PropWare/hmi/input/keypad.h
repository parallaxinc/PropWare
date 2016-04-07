/**
 * @file    PropWare/hmi/input/keypad.h
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

#pragma once

#include <PropWare/gpio/pin.h>
#include <PropWare/utility/utility.h>

namespace PropWare {

class Keypad {
    public:
        class Key {
            public:
                friend class Keypad;

            public:
                Key(const char character)
                    : character(character),
                      state(false) {
                }

                char get_character() const {
                    return this->character;
                }


                bool get_state() const {
                    return this->state;
                }

            protected:
                char character;
                bool state;
        };

    public:
        /**
         * MAPSIZE is the number of rows (times 16 columns)
         */
        static const unsigned int DEFAULT_BOUNCE_TIME_MS = 10;

    public:
        /**
         * Allows custom m_keymap, pin configuration, and keypad sizes
         */
        template<size_t ROW_SIZE, size_t COLUMN_SIZE>
        Keypad(Key keys[], const Pin (&rowPins)[ROW_SIZE], const Pin (&columnPins)[COLUMN_SIZE])
            : m_keys(keys),
              m_rowPins(rowPins),
              m_columnPins(columnPins),
              m_rows(ROW_SIZE),
              m_columns(COLUMN_SIZE),
              m_keyCount(this->m_rows * this->m_columns),
              m_startTime(CNT - DEFAULT_BOUNCE_TIME_MS * MILLISECOND) {
            this->set_debounce_time(Keypad::DEFAULT_BOUNCE_TIME_MS);
        }

        /**
         * Populate the key list
         */

        bool is_pressed(const char character) {
            this->get_keys();

            for (uint_fast8_t i = 0; i < this->m_keyCount; i++)
                if (character == this->m_keys[i].character) {
                    return this->m_keys[i].state;
                }
            return false;
        }

        bool get_keys() {
            bool keyActivity = false;

            // Limit how often the keypad is scanned. This makes the loop() run 10 times as fast.
            if ((CNT - this->m_startTime) > this->m_debounceTime) {
                this->scan_keys();
                this->m_startTime = CNT;
            }

            return keyActivity;
        }

        /**
         * Minimum debounceTime is 1 mS.
         */
        void set_debounce_time(const uint32_t debounceMs) {
            if (!debounceMs)
                this->m_debounceTime = MILLISECOND;
            else
                this->m_debounceTime = debounceMs * MILLISECOND;
        }

    private:

        /**
         * Hardware scan
         */
        void scan_keys() {
            for (uint_fast8_t row = 0; row < this->m_rows; row++) {
                this->m_rowPins[row].set_dir_in();
                this->m_rowPins[row].high();
            }

            // bitMap stores ALL the keys that are being pressed.
            for (uint_fast8_t column = 0; column < this->m_columns; ++column) {
                // Begin column pulse output.
                this->m_columnPins[column].low();
                this->m_columnPins[column].set_dir_out();

                for (uint_fast8_t row = 0; row < this->m_rows; ++row) {
                    const uint_fast8_t keyIndex = column * 4 + row;
                    this->m_keys[keyIndex].state = !this->m_rowPins[row].read();
                }

                // Set pin to high impedance input. Effectively ends column pulse.
                this->m_columnPins[column].high();
                this->m_columnPins[column].set_dir_in();
            }
        }

    private:
        Key                *m_keys;
        const Pin          *m_rowPins;
        const Pin          *m_columnPins;
        const uint_fast8_t m_rows;
        const uint_fast8_t m_columns;
        const uint_fast8_t m_keyCount;

        uint32_t          m_debounceTime;
        volatile uint32_t m_startTime;
};

}
