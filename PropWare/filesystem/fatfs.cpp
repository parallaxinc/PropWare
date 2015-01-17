/**
 * @file    fatfs.cpp
 *
 * @author  David Zemon
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

#include <PropWare/filesystem/fatfs.h>

const uint8_t PropWare::FatFS::PARTITION_IDS[54] = {0x01, 0x04, 0x06, 0x07, 0x08, 0x0B, 0x0C, 0x0E, 0x11, 0x12, 0x14,
        0x16, 0x17, 0x1B, 0x1C, 0x1E, 0x24, 0x27, 0x28, 0x56, 0x84, 0x86, 0x8B, 0x8D, 0x90, 0x92, 0x97, 0x98, 0x9A,
        0xAA, 0xB6, 0xBB, 0xBC, 0xC0, 0xC1, 0xC6, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCE, 0xD0, 0xD1, 0xD4, 0xD6, 0xDB,
        0xDE, 0xE1, 0xE4, 0xE5, 0xEF, 0xF2, 0xFE};