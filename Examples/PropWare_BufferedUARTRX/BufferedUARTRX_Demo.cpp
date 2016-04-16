/**
 * @file    BufferedUARTTX_Demo.cpp
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

#include <PropWare/hmi/output/printer.h>
#include <PropWare/hmi/input/scanner.h>
#include <PropWare/utility/charqueue.h>

/**
 * @example     BufferedUARTRX_Demo.cpp
 *
 * Write "Hello world!" out via UART protocol and receive an echo
 *
 * @include PropWare_UARTRX/CMakeLists.txt
 */
int main () {
    //extern unsigned int _load_start_buffereduartrx_cog[];
    char buffer[256];
    //PropWare::CharQueue queue(buffer);
    //cognew(_load_start_buffereduartrx_ecog, &queue);

    pwOut << "Hello! I'm going to say some things. I want you to answer them on your keyboard.\n";
    pwOut << "You won't see anything as you type - but don't worry, I'm catching it all.\n";
    pwOut << '\n';
    pwOut << "What's your name?\n";
    waitcnt(SECOND *2 + CNT);

    //PropWare::Scanner scanner(queue);
    //char input[32];
    //scanner >> input;
    //pwOut << "Your name is: " << input << "\n";
}
