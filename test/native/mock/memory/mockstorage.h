/**
 * @file        mock/memory/mockstorage.h
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

#include <PropWare/memory/blockstorage.h>

namespace mock {

class MockStorage : public PropWare::BlockStorage {
    public:
        typedef enum {
            NO_ERROR
        } ErrorCode;

    public:
        virtual PropWare::ErrorCode start () const {
            return NO_ERROR;
        }

        virtual PropWare::ErrorCode read_data_block (uint32_t address, uint8_t buf[]) const {
            return NO_ERROR;
        }

        virtual PropWare::ErrorCode write_data_block (uint32_t address, const uint8_t dat[]) const {
            return NO_ERROR;
        }

        virtual uint16_t get_short (const uint16_t offset, const uint8_t buf[]) const {
            return 0;
        }

        virtual uint32_t get_long (const uint16_t offset, const uint8_t buf[]) const {
            return 0;
        }

        virtual void write_short (const uint16_t offset, uint8_t buf[], const uint16_t value) const {
        }

        virtual void write_long (const uint16_t offset, uint8_t buf[], const uint32_t value) const {
        }

        virtual uint16_t get_sector_size () const {
            return 0;
        }

        virtual uint8_t get_sector_size_shift () const {
            return 0;
        }
};

}
