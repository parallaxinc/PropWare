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
#include <PropWare/scancapable.h>
#include <PropWare/filesystem/filesystem.h>

namespace PropWare {

class File : public ScanCapable {
    friend class Filesystem;
    friend class FatFS;

    public:
        typedef enum {
            ERROR,
            READ,
            WRITE,
            APPEND,
            R_UPDATE,
            W_UPDATE,
            A_UPDATE
        } Mode;

        typedef enum {
            NO_ERROR = 0
        } ErrorCode;

        // Signal that the contents of a buffer are a directory
        static const int8_t FOLDER_ID = -1;

    public:
        static Mode to_mode (const char modeStr[]) {
            Mode retVal = ERROR;

            if (strstr(modeStr, "r"))
                retVal = READ;
            else if (strstr(modeStr, "w"))
                retVal = WRITE;
            else if (strstr(modeStr, "a"))
                retVal = APPEND;

            if (ERROR != retVal && strstr(modeStr, "+"))
                retVal = (Mode) ((int) retVal + 3);

            return retVal;
        }
    static const char* to_string (const Mode mode) {
        switch (mode) {
            case ERROR:
                return "error";
            case READ:
                return "r";
            case WRITE:
                return "w";
            case APPEND:
                return "a";
            case R_UPDATE:
                return "r+";
            case W_UPDATE:
                return "w+";
            case A_UPDATE:
                return "a+";
            default:
                return "?";
        }
    }

    public:
        virtual PropWare::ErrorCode flush () = 0;

        void set_logger (const Printer *logger) {
            this->m_logger = logger;
        }

    protected:
        /**
         * Files can only be created by their respective Filesystems
         */
        File (const BlockStorage *driver, BlockStorage::Buffer *buffer, const int id, const Mode mode,
              const Printer *logger = &pwOut) : m_logger(logger),
                                                m_driver(driver),
                                                m_buf(buffer),
                                                m_mod(false),
                                                m_id(id),
                                                m_rPtr(0),
                                                m_wPtr(0),
                                                m_mode(mode) {
            this->m_buf->id = id;
        }

        PropWare::ErrorCode close () {
            return this->flush();
        }

        void print_status () {
            this->print_status("File");
        }

        void print_status (const char classStr[]) {
            this->m_logger->printf("File Status - PropWare::%s@0x%08X" CRLF, classStr, (unsigned int) this);
            this->m_logger->println("Common");
            this->m_logger->println("======");
            this->m_logger->printf("\tFile name: %s" CRLF, this->m_name);
            this->m_logger->printf("\tDriver: 0x%08X" CRLF, (unsigned int) this->m_driver);
            this->m_logger->printf("\tBuffer: 0x%08X" CRLF, (unsigned int) this->m_buf);
            this->m_logger->printf("\tModified: %s" CRLF, Utility::to_string(this->m_mod));
            this->m_logger->printf("\tFile ID: %u" CRLF, this->m_id);
            this->m_logger->printf("\tRead pointer: 0x%08X/%u" CRLF, this->m_rPtr, this->m_rPtr);
            this->m_logger->printf("\tWrite pointer: 0x%08X/%u" CRLF, this->m_wPtr, this->m_wPtr);
            this->m_logger->printf("\tFile mode: %s" CRLF, File::to_string(this->m_mode));
            this->m_logger->printf("\tLength: 0x%08X/%u" CRLF, this->m_length, this->m_length);
        }

    protected:
        char                 m_name[13];
        const Printer        *m_logger;
        const BlockStorage   *m_driver;
        BlockStorage::Buffer *m_buf;

        /** When the length of a file is changed, this variable will be set, otherwise cleared */
        bool       m_mod;
        /** determine if the buffer is owned by this file */
        uint8_t    m_id;
        uint32_t   m_rPtr;
        uint32_t   m_wPtr;
        File::Mode m_mode;
        uint32_t   m_length;
};

}
