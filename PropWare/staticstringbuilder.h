/**
 * @file    PropWare/staticstringbuilder.h
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
#include <PropWare/printcapable.h>

namespace PropWare {

class StaticStringBuilder : public PrintCapable {
    public:
        StaticStringBuilder (char buffer[])
                : m_string(buffer),
                  m_size(0) {
            this->m_string[0] = '\0';
        }

        void put_char (const char c) {
            this->m_string[this->m_size++] = c;
            this->m_string[this->m_size]   = '\0';
        }

        void puts (const char string[]) {
            for (const char *s = string; *s; ++s)
                this->m_string[this->m_size++] = *s;
            this->m_string[this->m_size]       = '\0';
        }

        const char *to_string () const {
            return this->m_string;
        }

        uint16_t get_size () const {
            return this->m_size;
        }

        void clear () {
            this->m_string[0] = '\0';
            this->m_size = 0;
        }

    private:
        char     *m_string;
        uint16_t m_size;
};

}
