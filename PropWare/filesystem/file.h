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
            /** Successful completion */NO_ERROR  = 0,
            /** First error code */     BEG_ERROR = Filesystem::BEG_ERROR + 1,
            /** End of file */          EOF_ERROR,
            /** Invalid file name */    INVALID_FILENAME,
            /** Final error code */     END_ERROR = EOF_ERROR
        }    ErrorCode;

        typedef enum {
            /** beginning of the stream */       BEG,
            /** current position in the stream */CUR,
            /** end of the stream */             END
        }    SeekDir;

        static const unsigned int MAX_FILENAME_LENGTH = 32;

    public:
        /**
         * Destructor
         */
        virtual ~File () { }

        /**
         * @brief       Open the file
         *
         * @return      0 upon success, error code otherwise
         */
        virtual PropWare::ErrorCode open () = 0;

        /**
         * @brief   Close a file - a required step in any workflow that includes opening a file
         *
         * @return      0 upon success, error code otherwise
         */
        virtual PropWare::ErrorCode close () {
            return this->flush();
        }

        /**
         * @brief   Flush any modified data back to the SD card
         *
         * @return      0 upon success, error code otherwise
         */
        virtual PropWare::ErrorCode flush () = 0;

        /**
         * @brief   Return the number of bytes (characters) in the file
         */
        int32_t get_length () const {
            return this->m_length;
        }

    protected:

        /**
         * Files can only be created by their respective Filesystems
         */
        File (Filesystem &fs, const char name[], BlockStorage::Buffer *buffer = NULL, const Printer &logger = pwOut)
                : m_logger(&logger),
                  m_driver(fs.get_driver()),
                  m_fsBufMeta(&fs.m_dirMeta),
                  m_length(-1),
                  m_ptr(0),
                  m_error(NO_ERROR) {
            if (NULL == buffer)
                this->m_buf = &fs.m_buf;
            else
                this->m_buf = buffer;

            this->m_contentMeta.name = name;
        }

        /**
         * @brief       Move the given pointer to a specified address
         *
         * @param[out]  &ptr    Pointer to be moved
         * @param[in]   pos     Scale that the pointer should be moved
         * @param[in]   way     Starting position for the movement
         *
         * @return      0 upon success, error code otherwise
         */
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

        /**
         * @brief       Print various data on a file - useful for debugging
         *
         * @param[in]   classStr[]      Used to help determine which concrete class called this method
         * @param[in]   printBlocks     Determine whether or not to print the content of the file's buffer
         */
        void print_status (const char classStr[] = "File", const bool printBlocks = false) const {
            this->m_logger->printf("File Status - PropWare::%s@0x%08X\n", classStr, (unsigned int) this);
            this->m_logger->println("=========================================");
            this->m_logger->println("Common");
            this->m_logger->println("------");
            this->m_logger->printf("\tFile name: %s\n", this->m_name);
            this->m_logger->printf("\tLogger: 0x%08X\n", (unsigned int) this->m_logger);
            this->m_logger->printf("\tDriver: 0x%08X\n", (unsigned int) this->m_driver);
            this->m_logger->printf("\tBuffer: 0x%08X\n", (unsigned int) this->m_buf);
            this->m_logger->printf("\tLength: 0x%08X/%d\n", this->m_length, this->m_length);

            if (NULL != this->m_buf) {
                this->m_logger->println("Buffer");
                this->m_logger->println("------");
                this->m_logger->printf("\tData address: 0x%08X\n", (unsigned int) this->m_buf->buf);
                this->m_logger->printf("\tMeta address: 0x%08X\n", (unsigned int) this->m_buf->meta);
                if (NULL != this->m_buf->buf && NULL != this->m_buf->meta) {
                    this->m_logger->printf("\tID: %d\n", this->m_buf->meta->id);
                    this->m_logger->printf("\tModdified: %s\n", Utility::to_string(this->m_buf->meta->mod));
                    this->m_logger->printf("\tCur. cluster's start sector: 0x%08X/%u\n",
                                           this->m_buf->meta->curTier2Addr,
                                           this->m_buf->meta->curTier2Addr);
                    this->m_logger->printf("\tCur. sector offset from cluster start: %u\n",
                                           this->m_buf->meta->curTier1Offset);
                    this->m_logger->printf("\tCurrent allocation unit: 0x%08X/%u\n", this->m_buf->meta->curTier2,
                                           this->m_buf->meta->curTier2);
                    this->m_logger->printf("\tNext allocation unit: 0x%08X/%u\n", this->m_buf->meta->nextTier2,
                                           this->m_buf->meta->nextTier2);
                    if (printBlocks) {
                        BlockStorage::print_block(*this->m_logger, *this->m_buf);
                    }
                }
            }
        }

    protected:
        char                   m_name[MAX_FILENAME_LENGTH];
        const Printer          *m_logger;
        const BlockStorage     *m_driver;
        BlockStorage::Buffer   *m_buf;
        /** Metadata for the file's content (location on the storage device) */
        BlockStorage::MetaData m_contentMeta;
        /** Metadata for the file's directory entry */
        BlockStorage::MetaData m_dirEntryMeta;
        /** Filesystem's buffer metadata (used to determine the current directory when opening the file) */
        BlockStorage::MetaData *m_fsBufMeta;

        int32_t m_length;
        int32_t m_ptr;

        PropWare::ErrorCode m_error;
};

}
