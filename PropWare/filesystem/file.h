/**
 * @file        file.h
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
#include <PropWare/filesystem/filesystem.h>

namespace PropWare {

class File {
    public:
        typedef enum {
            NO_ERROR = 0
        } ErrorCode;

    public:
        virtual ~File () {
            this->close();
        }

        /**
         * @brief       Open a file
         *
         * @param[in]   *buffer     Optional buffer can be used by the file. If no buffer is passed in, the
         *                          Filesystem's shared buffer will be used by the file. Passing a dedicated buffer
         *                          is only recommended when opening more than one simultaneously
         *
         * @return      0 upon success, error code otherwise
         */
        virtual PropWare::ErrorCode open () = 0;

        virtual PropWare::ErrorCode close () {
//            return this->flush();
            return NO_ERROR;
        }

        virtual PropWare::ErrorCode flush () = 0;

    protected:
        /**
         * Files can only be created by their respective Filesystems
         */
        File (Filesystem &fs, const char name[], BlockStorage::Buffer *buffer = NULL,
              const Printer &logger = pwOut)
                : m_logger(&logger),
                  m_driver(fs.get_driver()),
                  m_id(fs.next_file_id()),
                  m_mod(false) {
            strcpy(this->m_name, name);
            Utility::to_upper(this->m_name);

            if (NULL == buffer)
                this->m_buf = fs.get_buffer();
            else
                this->m_buf = buffer;
        }

        void print_status (const char classStr[] = "File", const bool printBlocks = false) const {
            this->m_logger->printf("File Status - PropWare::%s@0x%08X" CRLF, classStr, (unsigned int) this);
            this->m_logger->println("=========================================");
            this->m_logger->println("Common");
            this->m_logger->println("------");
            this->m_logger->printf("\tFile name: %s" CRLF, this->m_name);
            this->m_logger->printf("\tLogger: 0x%08X" CRLF, (unsigned int) this->m_logger);
            this->m_logger->printf("\tDriver: 0x%08X" CRLF, (unsigned int) this->m_driver);
            this->m_logger->printf("\tBuffer: 0x%08X" CRLF, (unsigned int) this->m_buf);
            this->m_logger->printf("\tModified: %s" CRLF, Utility::to_string(this->m_mod));
            this->m_logger->printf("\tFile ID: %u" CRLF, this->m_id);
            this->m_logger->printf("\tLength: 0x%08X/%u" CRLF, this->m_length, this->m_length);

            this->m_logger->println("Buffer");
            this->m_logger->println("------");
            if (this->m_buf->buf == NULL)
                this->m_logger->println("\tEmpty");
            else {
                this->m_logger->printf("\tID: %d" CRLF, this->m_buf->id);
                this->m_logger->printf("\tModdified: %s" CRLF, Utility::to_string(this->m_buf->mod));
                this->m_logger->printf("\tCur. cluster's start sector: 0x%08X/%u" CRLF, this->m_buf->curTier2StartAddr,
                                       this->m_buf->curTier2StartAddr);
                this->m_logger->printf("\tCur. sector offset from cluster start: %u" CRLF, this->m_buf->curTier1Offset);
                this->m_logger->printf("\tCurrent allocation unit: 0x%08X/%u" CRLF, this->m_buf->curTier3,
                                       this->m_buf->curTier3);
                this->m_logger->printf("\tNext allocation unit: 0x%08X/%u" CRLF, this->m_buf->nextTier3,
                                       this->m_buf->nextTier3);
                if (printBlocks)
                    BlockStorage::print_block(*this->m_logger, *this->m_buf);
            }
        }

    protected:
        char                 m_name[13];
        const Printer        *m_logger;
        const BlockStorage   *m_driver;
        BlockStorage::Buffer *m_buf;
        /** determine if the buffer is owned by this file */
        uint8_t              m_id;

        uint32_t   m_length;
        /** When the length of a file is changed, this variable will be set, otherwise cleared */
        bool       m_mod;
};

}
