/**
 * @file        queue.h
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

#include <cstddef>

namespace PropWare {

/**
 * @brief   A basic first-in, first-out queue implementation. The queue will overwrite itself when the maximum size
 *          is reached
 */
template<typename T>
class Queue {
    public:
        Queue (T array[], const size_t arrayLength)
                : m_array(array), m_arrayLength(arrayLength), m_size(0), m_head(-1), m_tail(-1) {
        }

        size_t size () const {
            return this->m_size;
        }

        bool is_empty () const {
            return 0 == this->size();
        }

        bool is_full () const {
            return this->m_arrayLength == this->size();
        }

        void clear () {
            this->m_size = 0;
        }

        /**
         * @brief       Insert an element to the buffer
         *
         * @param[in]   &value  Value to be inserted at the end of the buffer
         *
         * @post        If the buffer is already full, the oldest value will be overwritten with the `value` parameter
         *
         * @return      In order to allow chained calls to `PropWare::CircularQueue::enqueue`, the buffer isntance is
         *              returned
         */
        Queue & enqueue (const T &value) {
            // Move the head pointer
            if (this->is_empty())
                this->m_head = 0;
            else {
                ++this->m_head;
                // If the head has reached the end of the memory block, rollover
                if (this->m_arrayLength == (unsigned int) this->m_head)
                    this->m_head = 0;
            }

            this->m_array[this->m_head] = value;

            // If the buffer was previously empty, assign the tail pointer
            if (this->is_empty())
                this->m_tail = this->m_head;

            // If the buffer wasn't full, increment the size
            if (this->size() == this->m_arrayLength) {
                // Move the tail forward and roll over if necessary
                ++this->m_tail;
                if ((unsigned int) this->m_tail == this->m_arrayLength)
                    this->m_tail = 0;
            } else
                ++this->m_size;

            return *this;
        }

        /**
         * @see PropWare::CircularQueue::enqueue(const T &value)
         */
        Queue & insert (const T &value) {
            return this->enqueue(value);
        }

        /**
         * @brief   Return and remove the oldest value in the buffer
         *
         * @pre     Buffer must not be empty - no checks are performed to ensure the buffer contains data
         *
         * @return  Oldest value in the buffer
         */
        T dequeue () {
            if (this->size()) {
                T *retVal = &this->m_array[this->m_tail];

                --this->m_size;

                if (this->size()) {
                    // Move the tail forward and roll over if necessary
                    ++this->m_tail;
                    if ((unsigned int) this->m_tail == this->m_arrayLength)
                        this->m_tail = 0;
                } else {
                    this->m_tail = -1;
                    this->m_head = -1;
                }

                return *retVal;
            } else
                return * (T *) NULL;
        }

        /**
         * @brief   Return the oldest value in the buffer without removing it from the buffer
         *
         * @pre     Buffer must not be empty - no checks are performed to ensure the buffer contains data
         *
         * @return  Oldest value in the buffer
         */
        T peek () const {
            return this->m_array[this->m_tail];
        }

        /**
         * @brief   Determine if a value is valid
         *
         * If the queue is read (deque or peek) when the size is 0 then a value at address 0 is returned. A better
         * implementation would throw an exception when this occurs, but that isn't feasible on the Propeller. Use
         * this method to ensure values are valid prior to using them
         *
         * @param[in]   value   A value returned by PropWare::Queue::peek() or PropWare::Queue::dequeue()
         *
         * @return      Whether or not the value is valid
         */
        bool check(const T &value) const {
            const bool valid = &value == NULL;
            return valid;
        }

    private:
        T *m_array;
        const size_t m_arrayLength;

        volatile size_t m_size;
        volatile int m_head;
        volatile int m_tail;
};

}
