/**
 * @file        PropWare/hmi/input/stdin.h
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

#include <PropWare/hmi/input/scancapable.h>
#include <iostream>

#ifndef WIN32
#include <termios.h>
#include <unistd.h>
#endif

namespace mock {

class Stdin : public PropWare::ScanCapable {
    public:
        Stdin (bool echo = true) {
            // From http://stackoverflow.com/questions/1413445/read-a-password-from-stdcin
#ifdef WIN32
            HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
            DWORD mode;
            GetConsoleMode(hStdin, &mode);

            if( !echo )
                mode &= ~ENABLE_ECHO_INPUT;
            else
                mode |= ENABLE_ECHO_INPUT;

            SetConsoleMode(hStdin, mode );
#else
            termios tty;
            tcgetattr(STDIN_FILENO, &tty);
            if (echo)
                tty.c_lflag |= ECHO;
            else
                tty.c_lflag &= ~ECHO;
            tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif
        }

    public:
        virtual char get_char () {
            char c;
            std::cin >> c;
            return c;
        }
};

}
