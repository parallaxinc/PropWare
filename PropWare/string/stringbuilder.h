/**
 * @file    PropWare/string/stringbuilder.h
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
#include <PropWare/string/printer/printcapable.h>
#include <cstdlib>

namespace PropWare {

/**
 * @brief   Build a dynamically-sized string in RAM using the `PropWare::Printer` interface
 */
class StringBuilder : public PrintCapable {
    public:
        static const uint16_t DEFAULT_SPACE_ALLOCATED = 64;

    public:
        /**
         * @brief       Initialize with a given size to start with. Picking the correct size can increase performance.
         *
         * @param[in]   initialSize     Number of bytes that should be (dynamically) allocated for the string buffer
         */
        StringBuilder (const size_t initialSize = DEFAULT_SPACE_ALLOCATED)
                : m_minimumSpace(initialSize),
                  m_currentSpace(initialSize),
                  m_size(0) {
            this->m_string = (char *) malloc(initialSize);
            this->m_string[0] = '\0';
        }

        /**
         * @brief   Free all memory allocated for the string buffer
         */
        ~StringBuilder () {
            free(this->m_string);
        }

        void put_char (const char c) {
            // Don't try and save characters if the buffer doesn't exist
            if (NULL != this->m_string)
                this->insert_char(c);
            this->m_string[this->m_size] = '\0';
        }

        void puts (const char string[]) {
            // Don't try and save characters if the buffer doesn't exist
            if (NULL != this->m_string) {
                for (const char *s = string; *s; ++s)
                    this->insert_char(*s);
                this->m_string[this->m_size] = '\0';
            }
        }

        /**
         * @brief   Retrieve the address of the string buffer
         */
        const char *to_string () const {
            return this->m_string;
        }

        /**
         * @brief   Determine the length of the string, not including the null terminator
         */
        uint16_t get_size () const {
            return m_size;
        }

        /**
         * @brief   Remove all characters from the string and reallocate to the original size (if needed)
         */
        void clear () {
            if (this->m_size) {
                if (this->m_minimumSpace != this->m_currentSpace) {
                    if (NULL != this->m_string)
                        // It shouldn't ever be NULL, but this is an easy way to ensure bugs don't sneak in
                        free(this->m_string);
                    this->m_string       = (char *) malloc(this->m_minimumSpace);
                    this->m_currentSpace = this->m_minimumSpace;
                }
                this->m_string[0] = '\0';
                this->m_size = 0;
            }
        }

    private:
        void insert_char (const char c) {
            m_string[m_size++] = c;
            check_buffer_size();
        }

        void check_buffer_size () {
            if (m_size + 1 == m_currentSpace)
                this->expand();
        }

        void expand () {
            this->m_currentSpace <<= 1;
            char *temp = (char *) malloc((size_t) this->m_currentSpace);
            strcpy(temp, this->m_string);
            free(this->m_string);
            this->m_string = temp;
        }

    private:
        const uint16_t m_minimumSpace;
        uint16_t       m_currentSpace;
        uint16_t       m_size;
        char           *m_string;
};

}
