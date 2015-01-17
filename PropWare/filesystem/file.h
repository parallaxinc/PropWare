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
    friend class Filesystem;

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

        // Signal that the contents of a buffer are a directory
        static const int8_t FOLDER_ID = -1;
    public:
        static Mode get_mode (const char mode[]) {
            Mode retVal = ERROR;

            if (strstr(mode, "r"))
                retVal = READ;
            else if (strstr(mode, "w"))
                retVal = WRITE;
            else if (strstr(mode, "a"))
                retVal = APPEND;

            if (ERROR != retVal && strstr(mode, "+"))
                retVal = (Mode) ((int) retVal + 3);

            return retVal;
        }

    protected:
        /** determine if the buffer is owned by this file */
        uint8_t    id;
        uint32_t   wPtr;
        uint32_t   rPtr;
        File::Mode mode;
        uint32_t   length;
};

}
