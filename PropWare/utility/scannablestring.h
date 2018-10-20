/**
 * @file    scannablestring.h
 *
 * @author  David Zemon
 */

#pragma once

#include <PropWare/hmi/input/scancapable.h>

namespace PropWare {

class ScannableString: public PropWare::ScanCapable {
    public:
        ScannableString (const char *string)
            : m_string(string),
              m_index(0) {
        }

        virtual char get_char () {
            return this->m_string[this->m_index++];
        }

    protected:
        const char *m_string;
        size_t     m_index;
};

}
