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
#include <PropWare/printer/printer.h>

namespace PropWare {
class BlockStorage {
    public:
        struct Buffer {
            /**  Buffer data */
            uint8_t  *buf;
            /** Buffer ID - determine who owns the current information */
            int8_t   id;
            /** When set, the currently loaded sector has been modified since it was read from the SD card */
            bool     mod;
            /** Store the current cluster's starting sector number */
            uint32_t curTier2StartAddr;
            /** Store the current sector offset from the beginning of the cluster */
            uint8_t  curTier1Offset;
            /** Store the current allocation unit */
            uint32_t curTier3;
            /** Look-ahead at the next FAT entry */
            uint32_t nextTier3;
        };

    public:
        static void print_block (const Printer &printer, const Buffer &buffer, const size_t words = 512,
                                 const uint8_t wordsPerLine = 16) {
            print_block(printer, buffer.buf, words, wordsPerLine);
        }

        static void print_block (const Printer &printer, const uint8_t data[], const size_t words = 512,
                                 const uint8_t wordsPerLine = 16) {
            uint8_t lines = words / wordsPerLine;
            if (words % wordsPerLine)
                ++lines;

            // Printer header row
            printer.printf("         0 ");
            for (uint8_t i = 1; i < wordsPerLine; ++i)
                printer.printf("%2X ", i);
            printer.print(CRLF);

            for (uint16_t line = 0; line < lines; ++line) {
                const uint16_t baseAddress = line * wordsPerLine;
                printer.printf("0x%04X: ", baseAddress);

                // Print hex values
                for (uint8_t offset = 0; offset < wordsPerLine; ++offset)
                    printer.printf("%02X ", (unsigned int) data[baseAddress + offset]);

                // Print ASCII values
                for (uint8_t offset = 0; offset < wordsPerLine; ++offset) {
                    const char nextChar = data[baseAddress + offset];
                    if (32 <= nextChar && nextChar <= 126)
                        printer.print(nextChar);
                    else
                        printer.print('.');
                }

                printer.print(CRLF);
            }
        }

    public:
        virtual ErrorCode start() const = 0;

        virtual ErrorCode read_data_block (uint32_t address, uint8_t buf[]) const = 0;

        ErrorCode read_data_block (uint32_t address, const BlockStorage::Buffer *buffer) const {
            return this->read_data_block(address, buffer->buf);
        }

        virtual ErrorCode write_data_block (uint32_t address, const uint8_t dat[]) const = 0;

        ErrorCode write_data_block (uint32_t address, const BlockStorage::Buffer *buffer) const {
            return this->write_data_block(address, buffer->buf);
        }

        /**
         * @brief       Flush the contents of a buffer
         *
         * @param[in]   buffer  Buffer to be written to the SD card - written only it was modified
         */
        ErrorCode flush (Buffer *buffer) const {
            PropWare::ErrorCode err;
            if (buffer->mod) {
                check_errors(this->write_data_block(buffer->curTier2StartAddr + buffer->curTier1Offset, buffer->buf));
                buffer->mod = false;
            }
            return 0;
        }

        uint8_t get_byte (const uint16_t offset, const uint8_t *buf) const {
            return buf[offset];
        }

        virtual uint16_t get_short (const uint16_t offset, const uint8_t buf[]) const = 0;

        virtual uint32_t get_long (const uint16_t offset, const uint8_t buf[]) const = 0;

        void write_byte (const uint16_t offset, uint8_t buf[], const uint8_t value) const {
            buf[offset] = value;
        }

        virtual void write_short (const uint16_t offset, uint8_t buf[], const uint16_t value) const = 0;

        virtual void write_long (const uint16_t offset, uint8_t buf[], const uint32_t value) const = 0;

        virtual uint16_t get_sector_size () const = 0;

        virtual uint8_t get_sector_size_shift () const = 0;
};

}
