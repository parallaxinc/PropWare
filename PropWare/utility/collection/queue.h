/**
 * @file        PropWare/utility/collection/queue.h
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
#include <propeller.h>

// Need to include this since PropWare.h is not imported
#ifdef __PROPELLER_COG__
#define PropWare PropWare_cog
#define virtual
#endif

namespace PropWare {

/**
 * @brief   A basic first-in, first-out queue implementation. The queue will overwrite itself when the maximum size
 *          is reached
 */
template<typename T>
class Queue {
    public:
        /**
         * @brief   Construct a queue using the given statically-allocated array
         *
         * @param[in]   array   Statically allocated instance of an array, NOT a pointer
         */
        template<size_t N>
        Queue (T (&array)[N], const int lockNumber = locknew())
                : m_array(array),
                  m_arrayLength(N),
                  m_lockNumber(lockNumber),
                  m_size(0),
                  m_head(0),
                  m_tail(0) {
            lockclr(this->m_lockNumber);
        }

        /**
         * @brief   Construct a queue using the given dynamically allocated array (i.e., with `new` or `malloc`)
         *
         * This constructor is not recommended unless dynamic allocation is used. When using statically allocated
         * arrays, use the single-parameter constructor
         *
         * @param[in]   array   Address where the array begins
         * @param[in]   length  Number of elements allocated for the array
         */
        Queue (T *array, const size_t length, const int lockNumber)
                : m_array(array),
                  m_arrayLength(length),
                  m_lockNumber(locknew()),
                  m_size(0),
                  m_head(0),
                  m_tail(0) {
            lockclr(this->m_lockNumber);
        }

        ~Queue () {
            lockclr(this->m_lockNumber);
            lockret(this->m_lockNumber);
        }

        /**
         * @brief       Obtain the number of elements in the queue
         *
         * @returns     Number of elements in the queue
         */
        size_t size () const {
            return this->m_size;
        }

        /**
         * @brief       Determine if any elements exist
         *
         * @return      True if there is one or more elements, false otherwise
         */
        bool is_empty () const {
            return 0 == this->size();
        }

        /**
         * @brief       Determine if inserting another element would overwrite data
         *
         * @returns     True if the queue can not fit any more data without loosing old data, false otherwise
         */
        bool is_full () const {
            return this->m_arrayLength == this->size();
        }

        /**
         * @brief   Remove all data from the queue
         */
        void clear () {
            this->m_size = 0;
        }

        /**
         * @brief       Insert an element to the buffer
         *
         * @param[in]   value   Value to be inserted at the end of the buffer
         *
         * @post        If the buffer is already full, the oldest value will be overwritten with the `value` parameter
         *
         * @return      In order to allow chained calls to `PropWare::Queue::enqueue`, the Queue instance is returned
         */
        virtual Queue &enqueue (const T &value) {
            // Lock the state and save off these volatile variables into local memory
            while (lockset(this->m_lockNumber));
            unsigned int head = this->m_head;
            unsigned int tail = this->m_tail;
            size_t       size = this->m_size;

            // Move the head pointer
            if (0 == size)
                head = tail = 0;
            else {
                ++head;
                // If the head has reached the end of the memory block, rollover
                if (head == this->m_arrayLength)
                    head = 0;
            }

            this->m_array[head] = value;

            // If the buffer wasn't full, increment the size
            if (size == this->m_arrayLength) {
                // Move the tail forward and roll over if necessary
                ++tail;
                if (tail == this->m_arrayLength)
                    tail = 0;
            } else {
                ++size;
            }

            // Unlock the state and upload these variables back to volatile memory
            this->m_head = head;
            this->m_tail = tail;
            this->m_size = size;
            lockclr(this->m_lockNumber);

            return *this;
        }

        /**
         * @see PropWare::CircularQueue::enqueue(const T &value)
         */
        Queue &insert (const T &value) {
            return this->enqueue(value);
        }

        /**
         * @brief   Return and remove the oldest value in the buffer
         *
         * @pre     Buffer must not be empty - no checks are performed to ensure the buffer contains data
         *
         * @return  Oldest value in the buffer
         */
        virtual T dequeue () {
            // Lock the state and save off these volatile variables into local memory
            while (lockset(this->m_lockNumber));
            unsigned int head = this->m_head;
            unsigned int tail = this->m_tail;
            size_t       size = this->m_size;

            T *retVal;
            if (size) {
                retVal = &this->m_array[tail];

                --size;

                if (size) {
                    // Move the tail forward and roll over if necessary
                    ++tail;
                    if (tail == this->m_arrayLength)
                        tail = 0;
                }
            } else
                retVal = NULL;

            // Unlock the state and upload these variables back to volatile memory
            this->m_head = head;
            this->m_tail = tail;
            this->m_size = size;
            lockclr(this->m_lockNumber);

            return *retVal;
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
         * this method if you want to ensure values are valid prior to using them
         *
         * @param[in]   value   A value returned by PropWare::Queue::peek() or PropWare::Queue::dequeue()
         *
         * @return      Whether or not the value is valid
         */
        bool check (const T &value) const {
            const bool valid = &value == NULL;
            return valid;
        }

    protected:
        T            *m_array;
        const size_t m_arrayLength;
        const int    m_lockNumber;

        volatile size_t       m_size;
        volatile unsigned int m_head;
        volatile unsigned int m_tail;
};

}

#ifdef __PROPELLER_COG__
#undef virtual
#endif
