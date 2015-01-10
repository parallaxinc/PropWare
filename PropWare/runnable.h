/**
 * @file        runnable.h
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

#include <cstdint>
#include <sys/thread.h>
#include <stdlib.h>

namespace PropWare {

class Runnable {
    public:
        template<class T>
        static int8_t invoke (T &runnable) {
            return (int8_t) _start_cog_thread(runnable.get_stack_top(), (void (*) (void *)) &T::run, (void *) &runnable,
                                              &runnable.m_threadData);
        }

    public:
        Runnable (const uint32_t *stack, const size_t stackSizeInBytes)
                : m_stack(stack), m_stackSizeInBytes(stackSizeInBytes) {
        }

        virtual void run () = 0;

    protected:
        const uint32_t *m_stack;
        size_t         m_stackSizeInBytes;

    private:
        void *get_stack_top () const {
            return (void *) (m_stack + (m_stackSizeInBytes >> 2) - 1);
        }

    private:
        _thread_state_t m_threadData;
};

}