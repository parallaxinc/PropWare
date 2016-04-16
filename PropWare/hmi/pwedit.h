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

/**
 * @brief   Basic terminal-style text editor
 *
 * Capable of running on any `Printer` which supports the following escape sequences:
 *
 *   * `CSI n ; m H`: Move the cursor to column `n` and row `m`, where `n` and `m` are 1-indexed.
 */
class PWEdit {
    public:
        static const char BACKSPACE = 0x08;
        static const char CURSOR    = '#';
        typedef enum {
                                  NO_ERROR,
            /** First error */    BEG_ERROR     = 128
        }                 ErrorCode;

    public:
        /**
         * @brief   Constructor
         *
         * @param[in]   file        Unopened file to be displayed/edited
         * @param[in]   scanner     Human input will be read from this scanner. `pwIn` can not be used because it is set
         *                          for echo mode on, which can not be used in an editor
         * @param[in]   printer     Where the contents of the editor should be printed
         */
        PWEdit (FileReader &file, Scanner &scanner, const Printer &printer = pwOut, Printer *debugger = NULL)
                : m_file(&file),
                  m_printer(&pwOut),
                  m_scanner(&scanner),
                  m_debugger(debugger),
                  m_cols(0),
                  m_rows(1) {
        }

        ~PWEdit () {
            for (auto line : this->m_lines)
                delete line;
        }

        PropWare::ErrorCode run () {
            PropWare::ErrorCode err;

            this->calibrate();
            check_errors(this->read_in_file());
            this->display_file();

            return NO_ERROR;
        }

    protected:
        void calibrate () {
            static const char calibrationString[]     = "Calibration...#";
            const uint8_t     calibrationStringLength = (uint8_t) strlen(calibrationString);

            this->clear(false);
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
                            this->clear(false);
                            *this->m_printer << calibrationString;
                            *this->m_printer << BACKSPACE << ' ' << BACKSPACE;

                            // Handle columns
                            if (calibrationStringLength >= this->m_cols) {
                                const uint8_t     charactersToDelete =
                                                          (uint8_t) (calibrationStringLength - this->m_cols + 1);
                                for (unsigned int i                  = 0; i < charactersToDelete; ++i) {
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
            *this->m_printer << this->m_cols << 'x' << this->m_rows << " ";
        }

        PropWare::ErrorCode read_in_file () {
            PropWare::ErrorCode err;

            *this->m_printer << "Reading file...\n";

            check_errors(this->m_file->open());
            while (!this->m_file->eof()) {
                // Read a line
                StringBuilder *line = new StringBuilder();
                char          c;
                do {
                    check_errors(this->m_file->safe_get_char(c));
                    if (32 <= c && c <= 127)
                        // Only accept printable characters
                        line->put_char(c);
                } while ('\r' != c && '\n' != c);

                // Munch the \n following a \r
                if ('\n' == this->m_file->peek()) {
                    check_errors(this->m_file->safe_get_char(c));
                }

                this->m_lines.insert(this->m_lines.end(), line);

                this->move_cursor(2, 1);
                *this->m_printer << "Line: " << this->m_lines.size();
            }
            return NO_ERROR;
        }

        void display_file () const {
            this->clear();
            auto lineIterator = this->m_lines.cbegin();

            for (uint8_t row = 1; row <= this->m_rows; ++row) {
                this->move_cursor(row, 1);
                for (uint8_t column = 0; column < this->m_cols && column < (*lineIterator)->get_size(); ++column)
                    *this->m_printer << (*lineIterator)->to_string()[column];
                ++lineIterator;
            }
        }

        void clear (const bool writeSpaces = true) const {
            if (writeSpaces) {
                for (uint8_t row = 1; row <= this->m_rows; ++row) {
                    this->move_cursor(row, 1);
                    for (uint8_t col = 0; col <= this->m_cols; ++col)
                        *this->m_printer << ' ';
                }
            }
            this->move_cursor(1, 1);
        }

        void move_cursor (const uint8_t row, const uint8_t column) const {
            *this->m_printer << '\x1B' << '\x5B' << row << ';' << column << 'H';
        }

    protected:
        FileReader                 *m_file;
        const Printer              *m_printer;
        Scanner                    *m_scanner;
        Printer                    *m_debugger;
        std::list<StringBuilder *> m_lines;

        uint8_t m_cols;
        uint8_t m_rows;
};

}
