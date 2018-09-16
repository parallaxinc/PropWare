/**
 * @file    queue_test.cpp
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

#include "PropWareTests.h"
#include <PropWare/utility/collection/queue.h>

using PropWare::Queue;

static const size_t SIZE = 8;

class QueueTest {
    public:
        QueueTest () {
            testable = new Queue<int>(this->array);
        }

    public:
        int        array[SIZE];
        Queue<int> *testable;
};

TEST_F(QueueTest, Size_whenEmpty) {
    ASSERT_EQ_MSG(0, testable->size());
}

TEST_F(QueueTest, IsEmpty) {
    ASSERT_TRUE(testable->is_empty());
    testable->enqueue(1);
    ASSERT_FALSE(testable->is_empty());
}

TEST_F(QueueTest, enqueue_firstElement) {
    const int value = 42;

    testable->enqueue(value);
    ASSERT_EQ_MSG(1, testable->size());
    ASSERT_EQ_MSG(value, testable->peek());
}

TEST_F(QueueTest, Clear_whenEmpty) {
    testable->clear();
}

TEST_F(QueueTest, Clear_withOneElement) {
    testable->insert(42);
    testable->clear();
    ASSERT_TRUE(testable->is_empty());
}

TEST_F(QueueTest, Peek_doesNotRemoveElement) {
    const int value = 42;

    testable->enqueue(value);
    ASSERT_EQ_MSG(1, testable->size());
    ASSERT_EQ_MSG(value, testable->peek());
    ASSERT_EQ_MSG(1, testable->size());
    ASSERT_EQ_MSG(value, testable->peek());
}

TEST_F(QueueTest, enqueue_twoElements) {
    const int first  = 42;
    const int second = 13;

    testable->enqueue(first);
    ASSERT_EQ_MSG(1, testable->size());
    ASSERT_EQ_MSG(first, testable->peek());

    testable->enqueue(second);
    ASSERT_EQ_MSG(2, testable->size());
    ASSERT_EQ_MSG(second, testable->m_array[1]);
}

TEST_F(QueueTest, Deque_singleElement) {
    const int value = 42;

    testable->enqueue(value);

    ASSERT_EQ_MSG(1, testable->size());

    const int actual = testable->dequeue();
    ASSERT_EQ_MSG(value, actual);

    ASSERT_EQ_MSG(0, testable->size());
}

TEST_F(QueueTest, Deque_twoElements) {
    const int first  = 42;
    const int second = 13;

    testable->enqueue(first);
    testable->enqueue(second);

    ASSERT_EQ_MSG(2, testable->size());
    ASSERT_EQ_MSG(first, testable->dequeue());

    ASSERT_EQ_MSG(1, testable->size());
    ASSERT_EQ_MSG(second, testable->dequeue());

    ASSERT_EQ_MSG(0, testable->size());
}

TEST_F(QueueTest, Deque_multipleElements) {
    for (int i = 0; i < 4; ++i)
        testable->enqueue(i);

    ASSERT_EQ_MSG(4, testable->size());

    // Make sure peek points to the first element
    ASSERT_EQ_MSG(0, testable->peek());

    for (int i = 0; i < 4; ++i)
        ASSERT_EQ_MSG(i, testable->dequeue());
}

TEST_F(QueueTest, ManyElements) {
    // Insert many elements
    const size_t      TEST_SIZE = SIZE * 2 + 1;
    for (unsigned int i         = 0; i < TEST_SIZE; ++i) {
        testable->enqueue(i);
        if (SIZE > i) {
            ASSERT_EQ_MSG(i + 1, testable->size());
        } else {
            ASSERT_EQ_MSG(SIZE, testable->size());
        }
        ASSERT_EQ_MSG((int) i, testable->m_array[testable->m_head]);
    }

    // Dequeue many elements
    const size_t      DEQUEUE_LOOP_START = TEST_SIZE - SIZE;
    for (unsigned int i                  = DEQUEUE_LOOP_START; i < TEST_SIZE; ++i) {
        ASSERT_EQ_MSG((int) i, testable->dequeue());
    }
}

int main () {
    START(CircularBuffer);

    RUN_TEST_F(QueueTest, Size_whenEmpty);
    RUN_TEST_F(QueueTest, IsEmpty);
    RUN_TEST_F(QueueTest, enqueue_firstElement);
    RUN_TEST_F(QueueTest, Clear_whenEmpty);
    RUN_TEST_F(QueueTest, Clear_withOneElement);
    RUN_TEST_F(QueueTest, Peek_doesNotRemoveElement);
    RUN_TEST_F(QueueTest, enqueue_twoElements);
    RUN_TEST_F(QueueTest, Deque_singleElement);
    RUN_TEST_F(QueueTest, Deque_twoElements);
    RUN_TEST_F(QueueTest, Deque_multipleElements);
    RUN_TEST_F(QueueTest, ManyElements);

    COMPLETE();
}
