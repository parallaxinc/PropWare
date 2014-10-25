/**
 * @file        ram.h
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

#include <PropWare/PropWare.h>

namespace PropWare {
class BlockStorage {
    public:
        class Buffer {
            public:
                /**  Buffer data buffer */
                uint8_t *buf;
                /** Buffer ID - determine who owns the current information */
                int8_t  id;
#ifdef SD_OPTION_FILE_WRITE
                /**
                 * When set, the currently loaded sector has been modified since
                 * it was read from the SD card
                 */
                bool mod;
#endif
        };

    public:
        virtual ErrorCode start() = 0;

        virtual ErrorCode read_data_block (uint32_t address, uint8_t *buf) = 0;

        ErrorCode read_data_block (uint32_t address,
                const BlockStorage::Buffer *buffer) {
            return this->read_data_block(address, buffer->buf);
        }

        virtual ErrorCode write_data_block (uint32_t address, uint8_t *dat) = 0;

        ErrorCode write_data_block (uint32_t address,
                const BlockStorage::Buffer *buffer) {
            return this->write_data_block(address, buffer->buf);
        }

        uint8_t get_byte (const uint16_t offset, const uint8_t *buf) const {
            return buf[offset];
        }

        virtual uint16_t get_short (const uint16_t offset,
                const uint8_t *buf) const = 0;

        virtual uint32_t get_long (const uint16_t offset,
                const uint8_t *buf) const = 0;

        uint8_t get_byte (const uint16_t offset,
                const BlockStorage::Buffer *buf) const {
            return get_byte(offset, buf->buf);
        }

        uint16_t get_short (const uint16_t offset,
                const BlockStorage::Buffer *buf) const {
            return get_short(offset, buf->buf);
        }

        uint32_t get_long (const uint16_t offset,
                const BlockStorage::Buffer *buf) const {
            return get_long(offset, buf->buf);
        }

        virtual uint16_t get_sector_size () const = 0;

        virtual uint8_t get_sector_size_shift () const = 0;
};
}
