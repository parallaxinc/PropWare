/**
 * @file        PropWare/memory/blockstoragewriter.h
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
#include <PropWare/hmi/output/printer.h>
#include <PropWare/memory/blockstorage.h>

namespace PropWare {

class BlockStorageWriter : virtual public BlockStorage {
    public:
        /**
         * @brief       Write data to a storage device
         *
         * @param[in]   address     Block address on the storage device
         * @param[in]   dat         Array of data to be written
         *
         * @return      0 upon success, error code otherwise
         */
        virtual PropWare::ErrorCode write_data_block (uint32_t address, const uint8_t dat[]) const = 0;

        /**
         * @overload
         *
         * @param[in]   address     Block address on the storage device
         * @param[in]   *buffer     Address of the buffer that has data to be written
         *
         * @return      0 upon success, error code otherwise
         */
        PropWare::ErrorCode write_data_block (uint32_t address, const BlockStorage::Buffer *buffer) const {
            return this->write_data_block(address, buffer->buf);
        }

        /**
         * @brief       Flush the contents of a buffer and mark as unmodified
         *
         * @param[in]   buffer  Buffer to be written to the SD card - written only it was modified
         *
         * @return      0 upon success, error code otherwise
         */
        virtual PropWare::ErrorCode flush (Buffer *buffer) const {
            PropWare::ErrorCode err;
            if (buffer->meta && buffer->meta->mod) {
                check_errors(this->write_data_block(buffer->meta->curTier2Addr + buffer->meta->curTier1Offset,
                                                    buffer->buf));
                buffer->meta->mod = false;
            }
            return 0;
        }

        /**
         * @brief       Write a byte to a buffer
         *
         * @param[in]   offset  Address of the buffer to modify
         * @param[in]   buf[]   Buffer to modify
         * @param[in]   value   Value to be written in the buffer
         */
        void write_byte (const uint16_t offset, uint8_t buf[], const uint8_t value) const {
            buf[offset] = value;
        }

        /**
         * @brief       Write two bytes to a buffer
         *
         * Devices such as SD cards use reverse byte order compared with the Propeller - this method should be
         * implemented to ensure that the parameter is reversed if necessary. The user of this function should not
         * need to worry about reversing bytes
         *
         * @param[in]   offset  Address of the buffer to modify
         * @param[in]   buf[]   Buffer to modify
         * @param[in]   value   Value to be written in the buffer
         */
        virtual void write_short (const uint16_t offset, uint8_t buf[], const uint16_t value) const = 0;

        /**
         * @brief       Write four bytes to a buffer
         *
         * Devices such as SD cards use reverse byte order compared with the Propeller - this method should be
         * implemented to ensure that the parameter is reversed if necessary. The user of this function should not
         * need to worry about reversing bytes
         *
         * @param[in]   offset  Address of the buffer to modify
         * @param[in]   buf[]   Buffer to modify
         * @param[in]   value   Value to be written in the buffer
         */
        virtual void write_long (const uint16_t offset, uint8_t buf[], const uint32_t value) const = 0;
};

}
