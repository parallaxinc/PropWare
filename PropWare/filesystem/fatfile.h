/**
 * @file        fatfile.h
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

#include <PropWare/filesystem/file.h>
#include <PropWare/filesystem/filesystem.h>

namespace PropWare {

class FatFile : public File {
    friend class FatFS;

    public:
        char get_char () {
            return '\0';
        }

        PropWare::ErrorCode flush () {
            PropWare::ErrorCode err;

            if (this->m_mode == File::READ) {
                return NO_ERROR;
            } else {
                // If the currently loaded sector has been modified, save the changes
                check_errors(this->m_driver->flush_buffer(this->m_buf));

                // If we modified the length of the file...
                if (this->m_mod) {
                    // Then check if the directory sector is loaded...
                    if ((this->m_buf->curTier2StartAddr + this->m_buf->curTier1Offset) != this->m_dirTier1Addr)
                        // And load it if it isn't
                        check_errors(this->m_driver->read_data_block(this->m_dirTier1Addr, this->m_buf));

                    // Finally, edit the length of the file
                    this->m_driver->write_long(this->fileEntryOffset + FILE_LEN_OFFSET, this->m_buf->buf,
                                               this->m_length);
                    this->m_buf->mod = true;
                    check_errors(this->m_driver->flush_buffer(this->m_buf));
                }

                return NO_ERROR;
            }
        }

    private:
        static const uint8_t FILE_LEN_OFFSET = 0x1C;  // Length of a file in bytes

    private:
        FatFile (const BlockStorage *driver, BlockStorage::Buffer *buffer, const int id, const Mode mode,
                 const Printer *logger = &pwOut)
                : File(driver, buffer, id, mode, logger),
                  m_curTier1(0) {
        }

        ~FatFile () {
            this->close();
        }

        void print_status () {
            this->File::print_status("FatFile");

            this->m_logger->println("\tFAT-specific");
            this->m_logger->println("\t============");
            this->m_logger->printf("\t\tSectors allocated to file: 0x%08X/%u" CRLF, this->m_maxTier1s,
                                   this->m_maxTier1s);
            this->m_logger->printf("\t\tStarting allocation unit: 0x%08X/%u" CRLF, this->firstTier3, this->firstTier3);
            this->m_logger->printf("\t\tCurrent sector (counting from first in file): 0x%08X/%u" CRLF,
                                   this->m_curTier1, this->m_curTier1);
            this->m_logger->printf("\t\tCurrent cluster (counting from first in file): 0x%08X/%u" CRLF,
                                   this->m_curTier2, this->m_curTier2);
            this->m_logger->printf("\t\tDirectory address (sector): 0x%08X/%u" CRLF, this->m_dirTier1Addr,
                                   this->m_dirTier1Addr);
            this->m_logger->printf("\t\tFile entry offset: 0x%04X" CRLF, this->fileEntryOffset);

        }

    protected:
        /** Maximum number of sectors currently allocated to a file */
        uint32_t m_maxTier1s;
        /** File's starting allocation unit */
        uint32_t firstTier3;
        /** like curTier1Offset, but does not reset upon loading a new cluster */
        uint32_t m_curTier1;
        /** like curTier2, but for allocation units */
        uint32_t m_curTier2;
        /** Which sector of the storage device contains this file's meta-data */
        uint32_t m_dirTier1Addr;
        /** Address within the sector of this file's entry */
        uint16_t fileEntryOffset;
};

}
