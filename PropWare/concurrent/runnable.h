/**
 * @file        PropWare/concurrent/runnable.h
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

#include <type_traits>
#include <propeller.h>
#include <cstdint>
#include <stdlib.h>

namespace PropWare {

/**
 * @brief   Helper class for creating easy parallel applications
 *
 * To create a Runnable instance which will blink an LED, try the simple example below:
 *
 * @code
 * #include <PropWare/PropWare.h>
 * #include <PropWare/concurrent/runnable.h>
 * #include <PropWare/gpio/pin.h>
 * #include <PropWare/hmi/output/printer.h>
 *
 * class BlinkingThread : public PropWare::Runnable {
 *     public:
 *         template<size_t N>
 *         BlinkingThread (const uint32_t (&stack)[N], const PropWare::Pin::Mask mask)
 *                 : Runnable(stack),
 *                   m_mask(mask) {}
 *
 *         void run () {
 *             const PropWare::Pin pin(this->m_mask, PropWare::Pin::OUT);
 *             while (1) {
 *                 pin.toggle();
 *                 waitcnt(250 * MILLISECOND + CNT);
 *             }
 *         }
 *
 *     private:
 *         const PropWare::Pin::Mask m_mask;
 * };
 *
 * int main () {
 *     uint32_t       stack[64];
 *     BlinkingThread blinkyThread(stack, PropWare::Pin::P16);
 *
 *     int8_t cog = PropWare::Runnable::invoke(blinkyThread);
 *     pwOut << "Blink thread started in cog " << cog << "\n";
 *     while(1);
 * }
 * @endcode
 */
class Runnable {
    public:
        /**
         * @brief       Start a new cog running the given object
         *
         * @param[in]   runnable    Object that should be invoked in a new cog
         *
         * @returns     If the cog was successfully started, the new cog ID is returned. Otherwise, -1 is returned
         */
        template<class T>
        static int8_t invoke(T &runnable) {
            static_assert(std::is_base_of<Runnable, T>::value,
                          "Only PropWare::Runnable and its children can be invoked");
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpmf-conversions"
            return (int8_t) cogstart((void (*)(void *)) &T::run, (void *) &runnable,
                                     (void *) (runnable.m_stackPointer), runnable.m_stackSizeInBytes);
#pragma GCC diagnostic pop
        }

    public:
        /**
         * @brief   Invoked in the new cog, this method should be the root of the business logic
         */
        virtual void run() = 0;

    protected:
        /**
         * @brief       Construct a new instance that runs on the given stack
         *
         * @param[in]   stack[]     Statically-allocated array which will be used for stack memory by the new cog
         */
        template<size_t N>
        Runnable(const uint32_t (&stack)[N])
            : m_stackPointer(stack),
              m_stackSizeInBytes(N * sizeof(uint32_t)) {
        }

        /**
         * @brief       Construct an instance based on a dynamically allocated stack
         *
         * The first constructor is recommended because it calculates the size of the stack at compile time. If your
         * stack is allocated during runtime (i.e., with `malloc` or `new`), then you'll have to use this two-parameter
         * constructor.
         *
         * @warning     Use the single-parameter constructor whenever possible!
         *
         * @warning     The second parameter requests the length of the stack, not the size in bytes!
         *
         * @param[in]   stack           Address where the stack begins
         * @param[in]   stackLength     Number of elements in the stack. For instance, an array initialized as
         *                              `uint32_t *myStack = (uint32_t) malloc(16*sizeof(uint32_t));` would have a
         *                              length of 16, not 64.
         */
        Runnable(const uint32_t *stack, const size_t stackLength)
            : m_stackPointer(stack),
              m_stackSizeInBytes(stackLength * sizeof(uint32_t)) {
        }

    protected:
        const uint32_t *m_stackPointer;
        size_t         m_stackSizeInBytes;
};

}
