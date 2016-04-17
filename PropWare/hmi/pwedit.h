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
        typedef enum {
                                  NO_ERROR,
            /** First error */    BEG_ERROR = 128
        } ErrorCode;

        typedef enum {
            UP,
            DOWN,
            LEFT,
            RIGHT
        } Direction;

    public:
        static const char BELL      = 0x07;
        static const char BACKSPACE = 0x08;
        static const char CURSOR    = '#';
        static const char EXIT_CHAR = 'x';

        static const uint8_t PADDING = 3;

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
                  m_columns(0),
                  m_rows(1) {
        }

        ~PWEdit () {
            for (auto line : this->m_lines)
                delete line;
        }

        PropWare::ErrorCode run () {
            PropWare::ErrorCode err;

            this->calibrate();
            err = this->read_in_file();
            if (err) {
                this->m_file->close();
                return err;
            } else {
                this->display_file();

                char c;
                do {
                    c = this->m_scanner->get_char();
                    switch (c) {
                        case 'a':
                        case 'h':
                            this->move_selection(LEFT);
                            break;
                        case 's':
                        case 'j':
                            this->move_selection(DOWN);
                            break;
                        case 'd':
                        case 'l':
                            this->move_selection(RIGHT);
                            break;
                        case 'w':
                        case 'k':
                            this->move_selection(UP);
                            break;
                        case 'g':
                            this->to_file_start();
                            break;
                        case 'G':
                            this->to_file_end();
                            break;
                    }
                } while (EXIT_CHAR != c);
                this->m_file->close();

                return NO_ERROR;
            }
        }

    protected:
        void calibrate () {
            static const char calibrationString[]     = "Calibration...#";
            const uint8_t     calibrationStringLength = (uint8_t) strlen(calibrationString);

            this->clear(false);
            *this->m_printer << calibrationString;
            this->m_columns = calibrationStringLength;
            this->m_rows    = 1;

            char input;
            do {
                input = this->m_scanner->get_char();
                switch (input) {
                    case 'a':
                    case 'h':
                        // Move left
                        if (1 < this->m_columns) {
                            --this->m_columns;
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
                            if (calibrationStringLength >= this->m_columns) {
                                const uint8_t     charactersToDelete =
                                                          (uint8_t) (calibrationStringLength - this->m_columns + 1);
                                for (unsigned int i                  = 0; i < charactersToDelete; ++i) {
                                    *this->m_printer << BACKSPACE << ' ' << BACKSPACE;
                                }
                                *this->m_printer << CURSOR;
                            } else {
                                for (unsigned int i = calibrationStringLength; i < this->m_columns; ++i) {
                                    *this->m_printer << ' ';
                                }
                                *this->m_printer << CURSOR;
                            }

                            // Handle rows
                            for (unsigned int i = 1; i < this->m_rows; ++i) {
                                *this->m_printer << BACKSPACE << " \n";
                                for (unsigned int j = 0; j < (this->m_columns - 1); ++j)
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
                        for (unsigned int i = 0; i < (this->m_columns - 1); ++i)
                            *this->m_printer << ' ';
                        *this->m_printer << CURSOR;
                        break;
                    case 'd':
                    case 'l':
                        // Move right
                        ++this->m_columns;
                        *this->m_printer << BACKSPACE << " #";
                        break;
                }
            } while ('\r' != input && '\n' != input && '\0' != input);
            this->clear();
            *this->m_printer << this->m_columns << 'x' << this->m_rows << " ";
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

                this->m_lines.push_back(line);

                this->move_cursor(2, 1);
                *this->m_printer << "Line: " << this->m_lines.size();
            }
            return NO_ERROR;
        }

        void display_file () {
            this->display_file_from(0, 0);

            this->move_cursor(1, 1);
            this->m_termRow            = 1;
            this->m_termColumn         = 1;
            this->m_selectedLineNumber = this->m_firstLineDisplayed;
            this->m_selectedLine       = this->m_lines.begin();
        }

        void display_file_from (const unsigned int startingLineNumber, const unsigned int startingColumnNumber) {
            auto lineIterator = this->m_lines.cbegin();

            unsigned int i = startingLineNumber;
            while (i--)
                ++lineIterator;

            for (uint8_t row = 1; row <= this->m_rows; ++row) {
                const uint16_t charactersInLine = (*lineIterator)->get_size();

                this->move_cursor(row, 1);
                uint8_t column;
                for (column = 0;
                     column < this->m_columns && (column + startingColumnNumber) < charactersInLine;
                     ++column)
                    *this->m_printer << (*lineIterator)->to_string()[column + startingColumnNumber];
                while (column++ < this->m_columns)
                    *this->m_printer << ' ';
                ++lineIterator;
            }

            this->m_firstLineDisplayed = startingLineNumber;
        }

        void clear (const bool writeSpaces = true) const {
            if (writeSpaces) {
                for (uint8_t row = 1; row <= this->m_rows; ++row) {
                    this->move_cursor(row, 1);
                    for (uint8_t col = 0; col <= this->m_columns; ++col)
                        *this->m_printer << ' ';
                }
            }
            this->move_cursor(1, 1);
        }

        void move_cursor (const uint8_t row, const uint8_t column) const {
            *this->m_printer << '\x1B' << '\x5B' << row << ';' << column << 'H';
        }

        void move_selection (const Direction direction) {
            switch (direction) {
                case DOWN:
                    this->move_down();
                    break;
                case UP:
                    this->move_up();
                    break;
                case RIGHT:
                    this->move_right();
                    break;
                case LEFT:
                    this->move_left();
                    break;
            }
        }

        void move_down () {
            if ((this->m_lines.size() - 1) <= this->m_selectedLineNumber)
                *this->m_printer << BELL;
            else {
                const bool         redrawNecessary = this->trim_column_selection_to_fit(DOWN);
                const unsigned int lastLineInFile  = this->m_firstLineDisplayed + this->m_rows;
                if (PADDING > (this->m_rows - this->m_termRow)
                        && this->m_lines.size() > lastLineInFile) {
                    this->display_file_from(++this->m_firstLineDisplayed, this->m_firstColumnDisplayed);
                    this->move_cursor(this->m_termRow, this->m_termColumn);
                } else {
                    if (redrawNecessary)
                        this->display_file_from(this->m_firstLineDisplayed, this->m_firstColumnDisplayed);
                    this->move_cursor(++this->m_termRow, this->m_termColumn);
                }
                ++this->m_selectedLineNumber;
                ++this->m_selectedLine;
            }
        }

        void move_up () {
            if (!this->m_selectedLineNumber)
                *this->m_printer << BELL;
            else {
                const bool redrawNecessary = this->trim_column_selection_to_fit(UP);
                if (PADDING >= this->m_termRow && this->m_firstLineDisplayed) {
                    this->display_file_from(--this->m_firstLineDisplayed, this->m_firstColumnDisplayed);
                    this->move_cursor(this->m_termRow, this->m_termColumn);
                } else {
                    if (redrawNecessary)
                        this->display_file_from(this->m_firstLineDisplayed, this->m_firstColumnDisplayed);
                    this->move_cursor(--this->m_termRow, this->m_termColumn);
                }
                --this->m_selectedLineNumber;
                --this->m_selectedLine;
            }
        }

        void move_right () {
            if (((*this->m_selectedLine)->get_size() - 1) <= this->m_selectedColumnInLine) {
                *this->m_printer << BELL;
            } else {
                const unsigned int lastColumnInLine = this->m_firstColumnDisplayed + this->m_columns;
                if (PADDING > (this->m_columns - this->m_termColumn)
                        && (*this->m_selectedLine)->get_size() > lastColumnInLine) {
                    this->display_file_from(this->m_firstLineDisplayed, ++this->m_firstColumnDisplayed);
                    this->move_cursor(this->m_termRow, this->m_termColumn);
                } else
                    this->move_cursor(this->m_termRow, ++this->m_termColumn);
                ++this->m_selectedColumnInLine;
            }
        }

        void move_left () {
            if (!this->m_selectedColumnInLine)
                *this->m_printer << BELL;
            else {
                if (PADDING >= this->m_termColumn && this->m_firstColumnDisplayed) {
                    this->display_file_from(this->m_firstLineDisplayed, --this->m_firstColumnDisplayed);
                    this->move_cursor(this->m_termRow, this->m_termColumn);
                } else
                    this->move_cursor(this->m_termRow, --this->m_termColumn);
                --this->m_selectedColumnInLine;
            }
        }

        bool trim_column_selection_to_fit (const Direction direction) {
            auto tempIterator = m_selectedLine;
            switch (direction) {
                case UP:
                    --tempIterator;
                    break;
                case DOWN:
                    ++tempIterator;
                    break;
                default:
                    return false;
            }

            bool               redrawNecessary = false;
            const unsigned int lineLength      = (*tempIterator)->get_size();

            const bool lineShorterThanSelection = lineLength <= this->m_selectedColumnInLine;
            if (lineShorterThanSelection) {
                this->m_selectedColumnInLine = lineLength - 1;
                if (this->m_firstColumnDisplayed > this->m_selectedColumnInLine) {
                    this->m_firstColumnDisplayed = this->m_selectedColumnInLine;
                    this->m_termColumn           = 1;
                    redrawNecessary = true;
                } else
                    this->m_termColumn = (uint8_t) (this->m_selectedColumnInLine - this->m_firstColumnDisplayed + 1);
            }

            return redrawNecessary;
        }

        void to_file_start () {
            this->m_firstLineDisplayed   = 0;
            this->m_firstColumnDisplayed = 0;
            this->m_selectedLineNumber   = 0;
            this->m_selectedLine         = this->m_lines.cbegin();
            this->m_selectedColumnInLine = 0;
            this->m_termRow              = 1;
            this->m_termColumn           = 1;
            this->display_file_from(0, 0);
            this->move_cursor(this->m_termRow, this->m_termColumn);
        }

        void to_file_end () {
            this->m_firstLineDisplayed = this->m_lines.size() - this->m_rows;
            uint8_t lastColumn;
            if (this->m_lines.back()->get_size() > this->m_columns) {
                this->m_firstColumnDisplayed = this->m_lines.back()->get_size() - this->m_columns;
                lastColumn = (uint8_t) (this->m_columns - 1);
            } else {
                this->m_firstColumnDisplayed = 0;
                lastColumn = (uint8_t) (this->m_lines.back()->get_size());
            }
            this->m_selectedLine         = this->m_lines.cend();
            --this->m_selectedLine; // Need to point to the last line, not the "end" which is one after the last
            this->m_selectedLineNumber = this->m_lines.size() - 1;
            this->m_selectedColumnInLine = (unsigned int) ((*this->m_selectedLine)->get_size() - 1);
            this->m_termRow            = this->m_rows;
            this->m_termColumn         = lastColumn;
            this->display_file_from(this->m_firstLineDisplayed, this->m_firstColumnDisplayed);
            this->move_cursor(this->m_termRow, this->m_termColumn);
        }

    protected:
        FileReader                 *m_file;
        const Printer              *m_printer;
        Scanner                    *m_scanner;
        Printer                    *m_debugger;
        std::list<StringBuilder *> m_lines;

        /** Total columns on screen */
        uint8_t m_columns;
        /** Total rows on screen */
        uint8_t m_rows;

        /** Current cursor row (1-indexed) */
        uint8_t m_termRow;
        /** Current cursor column (1-indexed) */
        uint8_t m_termColumn;

        std::list<StringBuilder *>::const_iterator m_selectedLine;

        /** Index of currently selected line in the file (0-indexed) */
        unsigned int m_selectedLineNumber;
        /** Index of currently selected column in the line (0-indexed) */
        unsigned int m_selectedColumnInLine;

        /** First visible line of the file (0-indexed) */
        unsigned int m_firstLineDisplayed;
        /** First visible column of the line displayed (0-indexed) */
        unsigned int m_firstColumnDisplayed;
};

}
