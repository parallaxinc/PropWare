/**
 * @file    utility.h
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

#pragma once

#include <PropWare/PropWare.h>

namespace PropWare {

/**
 * Basic, static, convenience methods
 */
class Utility {
    public:
        /**
         * @brief       Count the number of set bits in a parameter
         *
         * @param[in]   par     Parameter whose bits should be counted
         *
         * @return      Number of bits that are non-zero in par
         */
        static uint8_t count_bits (uint32_t par) {
            // Brian Kernighan's method for counting set bits in a variable
            uint8_t totalBits = 0;

            while (par) {
                par &= par - 1; // clear the least significant bit set
                ++totalBits;
            }

            return totalBits;
        }

        /**
         * @brief       Count the number of set bits in a parameter
         *
         * @param[in]   par     Parameter whose bits should be counted
         *
         * @return      Number of bits that are non-zero in par
         */
        static uint8_t count_bits (int32_t par) {
            return count_bits((uint32_t) par);
        }

        /**
         * @brief       Determine the number of microseconds passed since a starting point
         *
         * GCC is very clever about optimizations - sometimes too clever. Be sure that your start variable is declared
         * as `volatile`. For instance, the following code is likely to be optimized by GCC in such a way that this
         * function returns 0:
         *
         * @code
         * const uint32_t start = CNT;
         * foo();
         * pwOut << "Runtime was " << Utility::measure_time_interval(start) << " milliseconds\n";
         * @endcode
         *
         * This is because GCC sees that `start` is never modified by the call to `foo()`, so it optimizes the above
         * code into the following:
         *
         * @code
         * foo();
         * pwOut << "Runtime was " << Utility::measure_time_interval(CNT) << " milliseconds\n";
         * @endcode
         *
         * Not too helpful :(
         *
         * Instead, simply add the keyword `volatile` to your variable, and all will be good:
         *
         * @code
         * const volatile uint32_t start = CNT;
         * foo();
         * pwOut << "Runtime was " << Utility::measure_time_interval(start) << " milliseconds\n";
         * @endcode
         *
         * @param[in]   start   A value from the system counter (CNT)
         *
         * @return      Microseconds since start
         */
        static inline uint32_t measure_time_interval (const register uint32_t start) {
            return (CNT - start) / MICROSECOND;
        }

        /**
         * @overload
         */
        static inline uint32_t measure_time_interval (const register int32_t start) {
            return measure_time_interval((uint32_t) start);
        }

        /**
         * @brief       Determine the size of the largest block of free memory
         *
         * `malloc` is used to find free memory. Be aware that the execution time of `malloc` is not predictable and
         * it is called repeatedly - so this method can, potentially, take a long time to execute
         *
         * @param[in]   precision   The precision (in bytes) with which the method will be executed. Result can be
         *                          off by +/- `precision` bytes. Lower values will increase execution time
         *
         * @return      Returns the size of the largest free block of memory
         */
        static size_t get_largest_free_block_size (const uint8_t precision = 32) {
            size_t largestSuccess  = 0;
            size_t smallestFailure = 32 * 1024;
            size_t nextAttempt     = 32 * 1024;

            uint8_t *ptr = NULL;

            do {
                ptr = (uint8_t *) malloc(nextAttempt);

                // If the allocation succeeded, free the memory as quickly as
                // possible
                if (NULL != ptr) {
                    free(ptr);
                    largestSuccess = nextAttempt;
                } else
                    // If the allocation fails, try the next smallest
                    smallestFailure = nextAttempt;


                nextAttempt = (smallestFailure - largestSuccess) / 2 +
                        largestSuccess;
            } while (precision < (smallestFailure - largestSuccess));

            return largestSuccess;
        }

        /**
         * @brief       Convert each alphabetical character in a null-terminated character array to lowercase letters
         *
         * @param[out]  string[]    Characters array to be converted
         */
        static void to_lower (char string[]) {
            for (size_t i = 0; i < strlen(string); ++i)
                string[i] = tolower(string[i]);
        }

        /**
         * @brief       Convert each alphabetical character in a null-terminated character array to uppercase letters
         *
         * @param[out]  string[]    Characters array to be converted
         */
        static void to_upper (char string[]) {
            for (size_t i = 0; i < strlen(string); ++i)
                string[i] = toupper(string[i]);
        }

        /**
         * @brief       Convert a boolean to the string-literal either `"true"` or `"false"`
         *
         * @param[in]   b   Boolean to be checked
         */
        static const char *to_string (const bool b) {
            return b ? "true" : "false";
        }

        /**
         * @brief       Compute the mathematical expression log<sub>2</sub>(x). Result is in fixed-point format (16
         *              digits to the left and right of the decimal point
         *
         * Contributed by Dave Hein
         *
         * @param[in]   x   Input to log function
         *
         * @return      Result of log function
         */
        static int rom_log (int x) {
            int            exp;
            unsigned short *ptr;

            if (!x) return 0;

            for (exp = 31; x > 0; exp--) x <<= 1;
            ptr = (unsigned short *) ((((unsigned int) x) >> 19) + 0xb000);
            return (exp << 16) | *ptr;
        }

        /**
         * @brief       Reverse some of the bits in `x`
         *
         * See the `rev` assembly instruction
         *
         * @param[in]   x       Value to be reversed
         * @param[in]   bits    The least significant `32-bits` bits will reversed
         *
         * @return      A bitwise flip of some of the bits in `x`
         */
        inline static unsigned int reverse (unsigned int x, unsigned int bits = 0) {
            return __builtin_propeller_rev(x, bits);
        }

        static bool empty (const char string[]) {
            return '\0' == string[0];
        }

        /**
         * @brief       Determine the size of an array
         *
         * As recommended by cplusplus.com in the FAQ:
         * http://www.cplusplus.com/faq/sequences/arrays/sizeof-array/#cpp
         *
         * @param[in]   array   Statically defined array (no malloc/new allowed)
         *
         * @return      Number of elements in the array
         */
        template <typename T, size_t N>
        static inline size_t size_of_array (const T(&array)[N]) {
            return N;
        }

        /**
         * @brief   Perform hard reboot
         *
         * Also known as reset or power down/up, this will restart the entire chip as if power was just applied. Note
         * that this will reboot __all 8 cogs__, not just one.
         */
        static inline void reboot () {
            __builtin_propeller_clkset(0x80);
        }

    private:
        /**
         * @brief   Static Utility class should never be instantiated. Call methods with code such as
         *          `uint8_t bits = PropWare::Utility::count_bits(0x03);`
         */
        Utility () { }
};

}
