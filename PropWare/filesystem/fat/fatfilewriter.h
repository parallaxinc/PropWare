/**
 * @file        fatfilewriter.h
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

#include <PropWare/filesystem/filewriter.h>
#include <PropWare/filesystem/fat/fatfile.h>

namespace PropWare {

class FatFileWriter : public virtual FatFile, public virtual FileWriter {

public:
    FatFileWriter (FatFS &fs, char const name[], Printer const *logger)
            : File(fs, name, logger),
              FatFile(fs, name, logger),
              FileWriter(fs, name, logger) {
    }

    PropWare::ErrorCode flush () {
        PropWare::ErrorCode err;

        // If the currently loaded sector has been modified, save the changes
        check_errors(this->m_driver->flush(this->m_buf));

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
            check_errors(this->m_driver->flush(this->m_buf));
        }

        return NO_ERROR;
    }

};

}
