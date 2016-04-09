/**
 * @file        PropWare/memory/blockstorage.h
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

namespace PropWare {

/**
 * @brief   Any device that uses blocks as hardware level abstraction.
 *
 * Well suited for SD/MMC cards, floppy disk, CD drives, hard disk drives, solid state drives, and others. Ongoing
 * work may be needed to remove lingering assumptions about generic block storage devices that I made based on my
 * experience with SD cards.
 */
class BlockStorage {
    public:
        /**
         * @brief   Metadata for a block of data
         *
         * @todo    [FIXME] This struct exists on the generic BlockStorage class, but it contains FAT-specific variables
         */
        struct MetaData {
            /** Human-readable name */
            const char   *name;
            /** Buffer ID - determine who owns the current information */
            int          id;
            /** Store the current sector offset from the beginning of the cluster */
            unsigned int curTier1Offset;
            /** Store the current cluster's starting sector number */
            uint32_t     curTier2Addr;
            /** Store the current allocation unit */
            uint32_t     curTier2;
            /** Look-ahead at the next FAT entry */
            uint32_t     nextTier2;
            /** When set, the currently loaded sector has been modified since it was read from the SD card */
            bool         mod;

            MetaData () {
                this->name = "";
            }
        };

        /**
         * @brief   In-memory buffer containing a single block of data
         */
        struct Buffer {
            uint8_t  *buf;
            MetaData *meta;
        };

        typedef enum {
            NO_ERROR = 0
        } ErrorCode;

    public:
        /**
         * @brief   Print the formatted contents of a buffer
         *
         * @param[in]   printer         Output console
         * @param[in]   buffer          Data in this container will be printed
         * @param[in]   words           Bytes in the buffer that should be printed
         * @param[in]   wordsPerLine    Bytes to be printed before a newline
         */
        static void print_block (const Printer &printer, const Buffer &buffer, const size_t words = 512,
                                 const uint8_t wordsPerLine = 16) {
            if (!Utility::empty(buffer.meta->name)) {
                printer.printf("Name = %s\n", buffer.meta->name);
            }
            print_block(printer, buffer.buf, words, wordsPerLine);
        }

        /**
         * @overload
         *
         * @param[in]   printer         Output console
         * @param[in]   data[]          Array with data to be printed
         * @param[in]   words           Bytes in the buffer that should be printed
         * @param[in]   wordsPerLine    Bytes to be printed before a newline
         */
        static void print_block (const Printer &printer, const uint8_t data[], const size_t words = 512,
                                 const uint8_t wordsPerLine = 16) {
            uint_fast8_t lines = words / wordsPerLine;
            if (words % wordsPerLine)
                ++lines;

            // Printer header row
            printer << "         0 ";
            for (uint_fast8_t i = 1; i < wordsPerLine; ++i) {
                if (8 == i)
                    printer.print("  ");
                printer.printf("%2X ", i);
            }
            printer << '\n';

            for (uint_fast16_t line = 0; line < lines; ++line) {
                const uint_fast16_t baseAddress = line * wordsPerLine;
                printer.printf("0x%04X: ", baseAddress);

                // Print hex values
                for (uint_fast8_t offset = 0; offset < wordsPerLine; ++offset) {
                    if (8 == offset)
                        printer.print("- ");
                    printer.printf("%02X ", (unsigned int) data[baseAddress + offset]);
                }

                // Print ASCII values
                for (uint_fast8_t offset = 0; offset < wordsPerLine; ++offset) {
                    const char nextChar = data[baseAddress + offset];
                    if (32 <= nextChar && nextChar <= 126)
                        printer << nextChar;
                    else
                        printer << '.';
                }

                printer << '\n';
            }
        }

    public:
        /**
         * @brief   Initialize and power up a storage device
         *
         * @return  0 upon success, error code otherwise
         */
        virtual PropWare::ErrorCode start () const = 0;

        virtual PropWare::ErrorCode flush (Buffer *buffer) const {
            return BlockStorage::NO_ERROR;
        }

        /**
         * @brief   Return the size of a sector (also known as a "block") for the given storage device
         *
         * @return  Bytes in a single sector
         */
        virtual uint16_t get_sector_size () const = 0;

        /**
         * @brief   Determine the number of shifts required to multiply or divide a number by the sector size
         *
         * Because the Propeller does not have a hardware multiply/divide instruction, having the log_2(`SECTOR_SIZE`)
         * can be helpful when you need to multiply or divide a number the sector size. Simply invoke this function and
         * then shift left or right by the return value.
         *
         * @return  log_2(`SECTOR_SIZE`)
         */
        virtual uint8_t get_sector_size_shift () const = 0;
};

}
