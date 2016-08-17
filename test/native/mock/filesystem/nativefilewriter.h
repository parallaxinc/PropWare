/**
 * @file        PropWare/filesystem/NativeFileWriter.h
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
#include <mock/filesystem/nativefilesystem.h>
#include <string>
#include <fstream>

namespace mock {

class NativeFileWriter : public PropWare::FileWriter {
    public:
        NativeFileWriter (NativeFilesystem &fs, const std::string &name)
                : FileWriter(fs, name.c_str()),
                  PropWare::File(fs, name.c_str()),
                  m_nameString(name) {
            this->m_file = new std::ofstream();
        }

        virtual PropWare::ErrorCode flush () override {
            this->m_file->flush();
            return NO_ERROR;
        }

        virtual PropWare::ErrorCode open (const int32_t offset, const SeekDir way) override {
            this->m_file->open(this->m_nameString);
            return NO_ERROR;
        }

        virtual PropWare::ErrorCode safe_put_char (const char c) override {
            this->m_file->put(c);
            return NO_ERROR;
        }

        virtual PropWare::ErrorCode trim () override {
            return NO_ERROR;
        }

    private:
        const std::string m_nameString;
        std::ofstream     *m_file;
};

}
