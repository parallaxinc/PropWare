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
#include <PropWare/filesystem/file.h>
#include <PropWare/filesystem/blockstorage.h>

#define check_fs_error(x) if ((err = x)) {this->m_error = err;return NULL;}

namespace PropWare {

class Filesystem {
public:
#define HD44780_MAX_ERROR    64
    typedef enum {
                                   NO_ERROR            = 0,
                                   ERROR_BEG           = HD44780_MAX_ERROR + 1,
        /** Filesystem Error  0 */ FILE_ALREADY_EXISTS = ERROR_BEG,
        /** Filesystem Error  1 */ INVALID_FILE_MODE,
        /** Filesystem Error  2 */ ENTRY_NOT_FILE,
        /** Filesystem Error  3 */ ENTRY_NOT_DIR,
        /** Filesystem Error  4 */ FILENAME_NOT_FOUND,
        /** Filesystem error  5 */ BAD_FILE_MODE,
        /** Filesystem Error  5 */ FILESYSTEM_ALREADY_MOUNTED,
        /** End Filesystem error */END_ERROR           = FILESYSTEM_ALREADY_MOUNTED
    } ErrorCode;

public:
    /**
     * @brief       Mount a filesystem
     *
     * @param[in]   partition   If multiple partitions are supported, the
     *                          partition number can be specified here
     *
     * @return  Returns 0 upon success, error code otherwise
     */
    virtual PropWare::ErrorCode mount (const uint8_t partition = 0) = 0;

    /**
     * @brief   Unmount the filesystem. If none is mounted, immediately return
     *
     * @return  Returns 0 upon success, error code otherwise
     */
    virtual PropWare::ErrorCode unmount () = 0;

    File* fopen (const char *name, const char modeStr[], BlockStorage::Buffer *buffer = NULL) {
        File::Mode mode = File::get_mode(modeStr);
        if (File::Mode::ERROR == mode)
            this->m_error = BAD_FILE_MODE;
            return NULL;
        return fopen(name, mode, buffer);
    }

    /**
     * @brief       Open a file from the given filesystem
     *
     * @param[in]   *name       Path to file that should be opened
     * @param[in]   mode        Mode to open the file as
     * @param[in]   *buffer     Optional buffer can be used by the file. If no buffer is passed in, the Filesystem's
     *                          shared buffer will be used by the file. Passing a dedicated buffer is only
     *                          recommended when opening more than one file
     *
     * @return      The newly opened file pointer is returned if successful, otherwise NULL is returned and an error
     *              code is set internally (@see PropWare::Filesystem::get_error)
     */
    virtual File* fopen (const char *name, const File::Mode mode, BlockStorage::Buffer *buffer = NULL) = 0;

    /**
     * @brief   Determine what error (if any) occurred. Error code is reset
     *          after call
     *
     * @return  Returns 0 when no error has yet occurred, otherwise error code
     */
    PropWare::ErrorCode get_error () {
        PropWare::ErrorCode err = this->m_error;
        this->m_error = NO_ERROR;
        return err;
    }

protected:
    static inline uint8_t get_file_id (File *f) {
        return f->id;
    }

    static inline void set_file_id (File *f, const uint8_t id) {
        f->id = id;
    }

    static inline void set_file_rPtr (File *f, const uint32_t rPtr) {
        f->rPtr = rPtr;
    }

    static inline void set_file_wPtr (File *f, const uint32_t wPtr) {
        f->wPtr = wPtr;
    }

    static inline File::Mode get_file_mode (File *f) {
        return f->mode;
    }

    static inline void set_file_mode (File *f, const File::Mode mode) {
        f->mode = mode;
    }

    static inline uint32_t get_file_length (File *f) {
        return f->length;
    }

    static inline void set_file_length (File *f, const uint32_t length) {
        f->length = length;
    }

protected:
    PropWare::ErrorCode m_error;
};

}
