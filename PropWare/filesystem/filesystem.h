/**
 * @file        filesystem.h
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
#include <PropWare/filesystem/blockstorage.h>

namespace PropWare {

class Filesystem {
    friend class File;

    public:
        // TODO: Sort these out properly between FS errors and file errors
#define HD44780_MAX_ERROR    64
        typedef enum {
                                       NO_ERROR            = 0,
                                       BEG_ERROR           = HD44780_MAX_ERROR + 1,
            /** Filesystem Error  0 */ FILE_ALREADY_EXISTS = BEG_ERROR,
            /** Filesystem Error  2 */ ENTRY_NOT_FILE,
            /** Filesystem Error  3 */ ENTRY_NOT_DIR,
            /** Filesystem Error  4 */ FILENAME_NOT_FOUND,
            /** Filesystem Error  5 */ FILESYSTEM_ALREADY_MOUNTED,
            /** End Filesystem error */END_ERROR           = FILESYSTEM_ALREADY_MOUNTED
        } ErrorCode;

        // Signal that the contents of a buffer are a directory
        static const int FOLDER_ID = INT32_MAX;

    public:
        /**
         * @brief       Mount a filesystem
         *
         * @param[in]   partition   If multiple partitions are supported, the partition number can be specified here
         *
         * @return  Returns 0 upon success, error code otherwise
         */
        virtual PropWare::ErrorCode mount (const uint8_t partition = 0) = 0;

        /**
         * @brief   Unmount the filesystem by flushing all dirty buffers
         */
        virtual PropWare::ErrorCode unmount () = 0;

        const BlockStorage * get_driver () const {
            return this->m_driver;
        }

    public:
        static void print_error_str (const Printer &printer, const ErrorCode err) {
            switch (err) {
                case FILE_ALREADY_EXISTS:
                    printer.println("File already exists");
                    break;
                case ENTRY_NOT_FILE:
                    printer.println("Entry is not a file");
                    break;
                case ENTRY_NOT_DIR:
                    printer.println("Entry is not a directory");
                    break;
                case FILENAME_NOT_FOUND:
                    printer.println("Filename was not found");
                    break;
                case FILESYSTEM_ALREADY_MOUNTED:
                    printer.println("Filesystem is already mounted");
                    break;
                default:
                    printer.printf("Unknown error: %u\n", err);
            }
        }

    protected:
        Filesystem (const BlockStorage *driver, const Printer *logger = &pwOut)
                : m_logger(logger),
                  m_driver(driver),
                  m_sectorSize(driver->get_sector_size()),
                  m_mounted(false),
                  m_nextFileId(0) {
            this->m_buf.buf = NULL;
            this->m_buf.meta = &m_dirMeta;
        }

        virtual uint32_t compute_tier1_from_tier3 (uint32_t allocUnit) const = 0;

        int next_file_id () {
            return this->m_nextFileId++;
        }

        BlockStorage::Buffer * get_buffer () {
            return &this->m_buf;
        }

        uint8_t get_tier1s_per_tier2_shift () {
            return this->m_tier1sPerTier2Shift;
        }

    protected:
        const Printer       *m_logger;
        const BlockStorage  *m_driver;
        const uint16_t      m_sectorSize;
        uint8_t             m_tier1sPerTier2Shift;  // Used as a quick multiply/divide; Stores log_2(Sectors per Cluster)

        bool                   m_mounted;
        BlockStorage::Buffer   m_buf;
        BlockStorage::MetaData m_dirMeta;
        int                    m_nextFileId;
};

}
