/**
 * @file    pwedit.h
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

#pragma once

#include <PropWare/hmi/output/printer.h>
#include <PropWare/hmi/input/scanner.h>
#include <PropWare/string/stringbuilder.h>
#include <PropWare/c++allocate.h>
#include <list>

namespace PropWare {

class PWEdit {
    public:
        static const char BACKSPACE = 0x08;
        static const char CURSOR    = '#';
        typedef enum {
            NO_ERROR,
            OUT_OF_MEMORY
        }                 ErrorCode;

    public:
        PWEdit (FileReader &file, Scanner &scanner, const Printer &printer = pwOut)
                : m_file(&file),
                  m_printer(&pwOut),
                  m_scanner(&scanner),
                  m_cols(0),
                  m_rows(1) {
        }

        ~PWEdit () {
            for(auto line : this->m_lines)
                delete line;
        }

        PropWare::ErrorCode run () {
            PropWare::ErrorCode err;

            this->calibrate();

            return NO_ERROR;
        }

    protected:
        void calibrate () {
            static const char calibrationString[]     = "Calibration...#";
            const size_t      calibrationStringLength = strlen(calibrationString);

            this->clear();
            *this->m_printer << calibrationString;
            this->m_cols = calibrationStringLength;
            this->m_rows = 1;

            char input;
            do {
                *this->m_scanner >> input;
                switch (input) {
                    case 'a':
                    case 'h':
                        // Move left
                        if (1 < this->m_cols) {
                            --this->m_cols;
                            *this->m_printer << BACKSPACE << ' ' << BACKSPACE << BACKSPACE << CURSOR;
                        }
                        break;
                    case 'w':
                    case 'k':
                        // Move up
                        if (1 < this->m_rows) {
                            --this->m_rows;
                            *this->m_printer << BACKSPACE << ' ' << BACKSPACE;
                            this->clear();
                            *this->m_printer << calibrationString;
                            *this->m_printer << BACKSPACE << ' ' << BACKSPACE;

                            // Handle columns
                            if (calibrationStringLength >= this->m_cols) {
                                const unsigned int charactersToDelete = calibrationStringLength - this->m_cols + 1;
                                for (unsigned int  i                  = 0; i < charactersToDelete; ++i) {
                                    *this->m_printer << BACKSPACE << ' ' << BACKSPACE;
                                }
                                *this->m_printer << CURSOR;
                            } else {
                                for (unsigned int i = calibrationStringLength; i < this->m_cols; ++i) {
                                    *this->m_printer << ' ';
                                }
                                *this->m_printer << CURSOR;
                            }

                            // Handle rows
                            for (unsigned int i = 1; i < this->m_rows; ++i) {
                                *this->m_printer << BACKSPACE << " \n";
                                for (unsigned int j = 0; j < (this->m_cols - 1); ++j)
                                    *this->m_printer << ' ';
                                *this->m_printer << CURSOR;
                            }
                        }
                        break;
                    case 's':
                    case 'j':
                        // Move down
                        ++this->m_rows;
                        *this->m_printer << BACKSPACE << " \n";
                        for (unsigned int i = 0; i < (this->m_cols - 1); ++i)
                            *this->m_printer << ' ';
                        *this->m_printer << CURSOR;
                        break;
                    case 'd':
                    case 'l':
                        // Move right
                        ++this->m_cols;
                        *this->m_printer << BACKSPACE << " #";
                        break;
                }
            } while ('\r' != input && '\n' != input && '\0' != input);
            this->clear();
            *this->m_printer << "Calibrated for a " << this->m_cols << 'x' << this->m_rows << " screen.";
        }

        void clear () {
            *this->m_printer << "\x1B\x5B\x01;\x01H";
        }

    protected:
        FileReader                 *m_file;
        const Printer              *m_printer;
        Scanner                    *m_scanner;
        std::list<StringBuilder *> m_lines;

        unsigned int m_cols;
        unsigned int m_rows;
};

}
