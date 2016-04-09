/**
 * @file        PropWare/memory/blockstoragereader.h
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

class BlockStorageReader : virtual public BlockStorage {
    public:
        /**
         * @brief   Read a block of data from the device into RAM
         *
         * @param[in]   address     Address of the block on the storage device
         * @param[out]  buf[]       Location in memory to store the block
         *
         * @return      0 upon success, error code otherwise
         */
        virtual PropWare::ErrorCode read_data_block (uint32_t address, uint8_t buf[]) const = 0;

        /**
         * @overload
         *
         * @param[in]   address     Address of the block on the storage device
         * @param[out]  *buffer     Address of the buffer to store data in
         *
         * @return      0 upon success, error code otherwise
         */
        PropWare::ErrorCode read_data_block (uint32_t address, const BlockStorage::Buffer *buffer) const {
            return this->read_data_block(address, buffer->buf);
        }

        /**
         * @brief       Use a buffer's metadata to determine the address and read data from the storage device into
         *              memory
         *
         * @param[in]   *buffer     Address of a buffer
         *
         * @pre         Contents of the buffer will not be written to storage device prior to overwriting, so be sure
         *              it is flushed before invoking
         *
         * @return      0 if successful, error code otherwise
         */
        PropWare::ErrorCode reload_buffer (const BlockStorage::Buffer *buffer) const {
            return this->read_data_block(buffer->meta->curTier2Addr + buffer->meta->curTier1Offset, buffer);
        }

        /**
         * @brief       Read a byte from a buffer
         *
         * @param[in]   offset  Offset from the beginning of the buffer
         * @param[in]   *buf    Address of the buffer to read
         *
         * @return      Requested byte in the buffer
         */
        uint8_t get_byte (const uint16_t offset, const uint8_t *buf) const {
            return buf[offset];
        }

        /**
         * @brief       Read two bytes from a buffer
         *
         * Devices such as SD cards use reverse byte order compared with the Propeller - this method should be
         * implemented to ensure that the returned value is reversed if necessary. The user of this function should not
         * need to worry about reversing bytes
         *
         * @param[in]   offset  Where in the buffer should the value be retrieved
         * @param[in]   buf[]   Address of the buffer to read
         *
         * @return      Requested bytes from the buffer
         */
        virtual uint16_t get_short (const uint16_t offset, const uint8_t buf[]) const = 0;

        /**
         * @brief       Read four bytes from a buffer
         *
         * Devices such as SD cards use reverse byte order compared with the Propeller - this method should be
         * implemented to ensure that the returned value is reversed if necessary. The user of this function should not
         * need to worry about reversing bytes
         *
         * @param[in]   offset  Where in the buffer should the value be retrieved
         * @param[in]   buf[]   Address of the buffer to read
         *
         * @return      Requested bytes from the buffer
         */
        virtual uint32_t get_long (const uint16_t offset, const uint8_t buf[]) const = 0;
};

}
