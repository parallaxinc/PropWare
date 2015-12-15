/**
 * @file    PropWare/comparator.h
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

namespace PropWare {

/**
 * @brief   Provide a way for a `PropWare::Scanner` to sanitize user input
 */
template<typename T>
class Comparator {
    public:
        /**
         * @brief   Determines if the given argument is valid
         */
        virtual bool valid (const T *lhs) const = 0;
};

/**
 * @brief   Also known as whole numbers, this class will only allow numbers that are zero or greater with no
 *          fractional part
 *
 * Though you are welcome to construct your own instance, a global instance is available for use in the PropWare
 * namespace: `PropWare::NON_NEGATIVE_COMP`.
 */
class NonNegativeIntegerComparator : public Comparator<int> {
    public:
        /**
         * @brief   Required default constructor
         */
        NonNegativeIntegerComparator () {
        }

        bool valid (const int *lhs) const {
            return 0 <= *lhs;
        }
};

}

extern const PropWare::NonNegativeIntegerComparator NON_NEGATIVE_COMP;
