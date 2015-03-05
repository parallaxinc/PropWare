/**
* @file        printer.h
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
#include <PropWare/scancapable.h>
#include <PropWare/printer/printer.h>
#include <PropWare/uart/halfduplexuart.h>

extern "C" {
extern const char *_scanf_getl (const char *str, int *dst, int base, unsigned width, int isSigned);
extern const char *_scanf_getf (const char *str, float *dst);
}

namespace PropWare {

/**
* @brief    Interface for all classes capable of scanning
*/
class Scanner {
    public:
        typedef enum {
            /** No error */               NO_ERROR  = 0,
            /** First Scanner error */    BEG_ERROR,
            /** Scanner Error  0 */       BAD_INPUT = BEG_ERROR,
            /** Last Scanner error code */END_ERROR = BAD_INPUT
        } ErrorCode;

    public:
        static const char DEFAULT_DELIMITER = '\n';

    public:
        Scanner (ScanCapable *scanCapable, const Printer *printer)
                : m_scanCapable(scanCapable), m_printer(printer) {
        }

        /**
         * @see PropWare::ScanCapable::get_char
         */
        char get_char () {
            const char c = this->m_scanCapable->get_char();
            this->m_printer->put_char(c);
            return c;
        }

        /**
         * @see PropWare::ScanCapable::fgets
         */
        ErrorCode gets (char string[], int32_t length, const char delimiter = DEFAULT_DELIMITER) {
            char *buf = string;
            while (0 < --length) {
                char ch = this->m_scanCapable->get_char();

                if (ch == 8 || ch == 127) {
                    if (buf > string) {
                        this->m_printer->puts("\010 \010");
                        ++length;
                        --buf;
                    }
                    length += 1;
                    continue;
                }

                this->m_printer->put_char(ch);

                if ('\r' == ch)
                    this->m_printer->put_char('\n');

                if ('\r' == ch || '\n' == ch)
                    break;
                else
                    *(buf++) = ch;
            }
            *buf = 0;

            return NO_ERROR;
        }

        const Scanner& operator>> (char &c) {
            this->get(c);
            return *this;
        }

        const Scanner& operator>> (uint32_t &x) {
            get(x);
            return *this;
        }

        const Scanner& operator>> (int32_t &x) {
            get(x);
            return *this;
        }

        const Scanner& operator>> (float &f) {
            return *this;
        }

        const ErrorCode get (char &c) {
            ErrorCode err;
            char userInput[2];
            check_errors(this->gets(userInput, sizeof(userInput)));
            if ('\0' == c)
                return BAD_INPUT;
            else {
                c = userInput[0];
                return NO_ERROR;
            }
        }

        const ErrorCode get(uint32_t &x) {
            ErrorCode  err;
            char userInput[32];
            check_errors(this->gets(userInput, sizeof(userInput)));
            if (0 == _scanf_getl(userInput, (int *) &x, 10, 11, false))
                return BAD_INPUT;
            else
                return NO_ERROR;
        }

        const ErrorCode get(int32_t &x) {
            ErrorCode  err;
            char userInput[32];
            check_errors(this->gets(userInput, sizeof(userInput)));
            if (0 == _scanf_getl(userInput, &x, 10, 11, false))
                return BAD_INPUT;
            else
                return NO_ERROR;
        }

        const ErrorCode get(float &f) {
            ErrorCode err;
            char userInput[32];
            check_errors(this->gets(userInput, sizeof(userInput)));
            if (0 == _scanf_getf(userInput, &f))
                return BAD_INPUT;
            else
                return NO_ERROR;
        }

        /**
         * @brief
         *
         * @param[in]   prompt[]            User prompt which will be displayed before each attempt to read the serial
         *                                  bus
         * @param[in]   failureResponse[]   Message to be displayed after each incorrect input
         * @param[out]  userInput[]         Buffer that can be used for storing the user's input
         * @param[in]   bufferLength        Size (in bytes) of the `userInput[]` buffer
         * @param[in]   comparator          Determines whether or not the received input was valid
         */
        void input_prompt (const char prompt[], const char failureResponse[], char userInput[],
                           const size_t bufferLength, const Comparator<char> &comparator) {
            do {
                this->m_printer->puts(prompt);
                this->gets(userInput, bufferLength);

                if (comparator.valid(userInput))
                    return;
                else
                    this->m_printer->puts(failureResponse);
            } while (1);
        }

        /**
         * @brief
         *
         * @param[in]   prompt[]            User prompt which will be displayed before each attempt to read the serial
         *                                  bus
         * @param[in]   failureResponse[]   Message to be displayed after each incorrect input
         * @param[out]  *userInput          Resulting value will be stored at this address
         * @param[in]   comparator          Determines whether or not the received input was valid
         */
        template<typename T>
        void input_prompt (const char prompt[], const char failureResponse[], T *userInput,
                           const Comparator<T> &comparator) {
            const T original = *userInput;
            ErrorCode err;
            do {
                this->m_printer->puts(prompt);
                err = this->get(*userInput);
                if (NO_ERROR == err && comparator.valid(userInput))
                    return;

                this->m_printer->puts(failureResponse);
                *userInput = original;
            } while (1);
        }

    private:
        ScanCapable *m_scanCapable;
        const Printer     *m_printer;
};

}

extern PropWare::Scanner pwIn;
