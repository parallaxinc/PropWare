/**
 * @file        PropWare/filesystem/nativefilereader.h
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

#include <PropWare/filesystem/filereader.h>
#include <mock/filesystem/nativefilesystem.h>
#include <string>
#include <fstream>

namespace mock {

class NativeFileReader : public PropWare::FileReader {
    public:
        NativeFileReader (NativeFilesystem &fs, const std::string &name)
                : PropWare::FileReader(fs, name.c_str()),
                  PropWare::File(fs, name.c_str()),
                  m_nameString(name) {
            this->m_file = new std::ifstream();
        }

        virtual PropWare::ErrorCode open (const int32_t offset = 0, const SeekDir way = BEG) override {
            this->m_file->open(this->m_nameString);
            return NO_ERROR;
        }

        virtual PropWare::ErrorCode safe_get_char (char &c) override {
            if (this->m_file->eof())
                return EOF_ERROR;
            else {
                c = (char) this->m_file->get();
                return NO_ERROR;
            }
        }

    private:
        const std::string m_nameString;
        std::ifstream     *m_file;
};

}
