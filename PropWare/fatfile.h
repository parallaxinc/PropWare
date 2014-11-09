/**
* @file        fatfs.h
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

#include <PropWare/file.h>
#include <PropWare/filesystem.h>

namespace PropWare {

class FatFile : public File {
    friend class FatFS;

    protected:
        BlockStorage::Buffer *buf;
        /** Maximum number of sectors currently allocated to a file */
        uint32_t             maxSectors;
        /**
         * When the length of a file is changed, this variable will be
         * set, otherwise cleared
         */
        bool                 mod;
        /** File's starting allocation unit */
        uint32_t             firstAllocUnit;
        /**
         * like curSectorOffset, but does not reset upon loading a new
         * cluster
         */
        uint32_t             curSector;
        /** like curSector, but for allocation units */
        uint32_t             curCluster;
        /**
         * Which sector of the SD card contains this file's meta-data
         */
        uint32_t             dirSectorAddr;
        /** Address within the sector of this file's entry */
        uint16_t             fileEntryOffset;
};

}
