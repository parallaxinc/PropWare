/**
 * @file        PropWare/utility/charqueue.h
 *
 * @author      David Zemon
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

#pragma once

#include <PropWare/utility/queue.h>
#include <PropWare/hmi/input/scancapable.h>
#include <PropWare/hmi/output/printcapable.h>

#ifdef __PROPELLER_COG__
#define virtual
#endif

namespace PropWare {

/**
 * @brief   Provide a communication buffer for character data between cogs
 *
 * Typically used for buffered UART implementations. Note that the put_char and get_char methods are blocking to
 * ensure that put_char does not attempt to write to a full buffer and get_char does not attempt to read from an
 * empty buffer. For this reason, you should be careful about using the enqueue and dequeue methods directly when
 * using a CharQueue object because they will allow you to write to a full queue and read from an empty one.
 */
class CharQueue : public Queue<char>,
                  public ScanCapable,
                  public PrintCapable {
    public:
        template<size_t N>
        CharQueue (char (&array)[N], const int lockNumber = locknew())
                : Queue(array, lockNumber) {
        }

        CharQueue (char *array, const size_t length, const int lockNumber)
                : Queue(array, length, lockNumber) {
        }

        virtual char get_char () {
            while (this->is_empty());
            return this->dequeue();
        }

        virtual void put_char (const char c) {
            while (this->is_full());
            this->enqueue(c);
        }

        virtual void puts (const char *string) {
            char *c = const_cast<char *>(string);
            while (*c) {
                this->put_char(*c);
                c++;
            }
        }
};

}

#ifdef __PROPELLER_COG__
#undef virtual
#endif
