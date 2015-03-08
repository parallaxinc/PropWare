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
            NO_ERROR = 0,
            BEG_ERROR = Filesystem::BEG_ERROR + 1,
            EOF_ERROR,
            END_ERROR
        } ErrorCode;

        typedef enum {
            /** beginning of the stream */       BEG,
            /** current position in the stream */CUR,
            /** end of the stream */             END
        } SeekDir;

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

        uint32_t get_length () const {
            return this->m_length;
        }

    protected:
        /**
         * Files can only be created by their respective Filesystems
         */
        File (Filesystem &fs, const char name[], BlockStorage::Buffer *buffer = NULL, const Printer &logger = pwOut)
                : m_logger(&logger),
                  m_driver(fs.get_driver()),
                  m_id(fs.next_file_id()),
                  m_mod(false),
                  m_error(NO_ERROR) {
            strcpy(this->m_name, name);
            Utility::to_upper(this->m_name);

            if (NULL == buffer)
                this->m_buf = fs.get_buffer();
            else
                this->m_buf = buffer;
        }

        PropWare::ErrorCode seek (int32_t &ptr, const int32_t pos, const SeekDir way) {
            int32_t absolute;
            switch (way) {
                case BEG:
                    if (pos > this->m_length || 0 > pos)
                        return EOF_ERROR;
                    else {
                        ptr = pos;
                        break;
                    }
                case CUR:
                    absolute = pos + ptr;
                    if (0 > pos || pos > this->m_length)
                        return EOF_ERROR;
                    else {
                        ptr = (uint32_t) absolute;
                        break;
                    }
                case END:
                    absolute = this->m_length - pos;
                    if (0 > pos || pos > this->m_length)
                        return EOF_ERROR;
                    else {
                        ptr = (uint32_t) absolute;
                        break;
                    }
            }
            return NO_ERROR;
        }

        void print_status (const char classStr[] = "File", const bool printBlocks = false) const {
            this->m_logger->printf("File Status - PropWare::%s@0x%08X\n", classStr, (unsigned int) this);
            this->m_logger->println("=========================================");
            this->m_logger->println("Common");
            this->m_logger->println("------");
            this->m_logger->printf("\tFile name: %s\n", this->m_name);
            this->m_logger->printf("\tLogger: 0x%08X\n", (unsigned int) this->m_logger);
            this->m_logger->printf("\tDriver: 0x%08X\n", (unsigned int) this->m_driver);
            this->m_logger->printf("\tBuffer: 0x%08X\n", (unsigned int) this->m_buf);
            this->m_logger->printf("\tModified: %s\n", Utility::to_string(this->m_mod));
            this->m_logger->printf("\tFile ID: %u\n", this->m_id);
            this->m_logger->printf("\tLength: 0x%08X/%u\n", this->m_length, this->m_length);

            this->m_logger->println("Buffer");
            this->m_logger->println("------");
            if (this->m_buf->buf == NULL)
                this->m_logger->println("\tEmpty");
            else {
                this->m_logger->printf("\tID: %d\n", this->m_buf->id);
                this->m_logger->printf("\tModdified: %s\n", Utility::to_string(this->m_buf->mod));
                this->m_logger->printf("\tCur. cluster's start sector: 0x%08X/%u\n", this->m_buf->curTier2StartAddr,
                                       this->m_buf->curTier2StartAddr);
                this->m_logger->printf("\tCur. sector offset from cluster start: %u\n", this->m_buf->curTier1Offset);
                this->m_logger->printf("\tCurrent allocation unit: 0x%08X/%u\n", this->m_buf->curTier3,
                                       this->m_buf->curTier3);
                this->m_logger->printf("\tNext allocation unit: 0x%08X/%u\n", this->m_buf->nextTier3,
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

        /** When the length of a file is changed, this variable will be set, otherwise cleared */
        bool    m_mod;
        int32_t m_length;

        PropWare::ErrorCode m_error;
};

}
