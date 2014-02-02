/**
 * @file    SD_Demo.h
 */
/**
 * @brief   Demonstrate various functions including writing to files and the
 *          Shell
 *
 * @project SD_Demo
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

#ifndef SD_DEMO_H_
#define SD_DEMO_H_

/**
 * @defgroup    _propware_example_sd    SD Card Demo
 * @ingroup     _propware_examples
 * @{
 */

#define DEBUG
//#define LOW_RAM_MODE
#define TEST_WRITE
#define TEST_SHELL

// Includes
#include <propeller.h>
#include <PropWare.h>
#include <sd.h>

#ifdef DEBUG
#include <stdio.h>
#endif

#if (!(defined SD_OPTION_VERBOSE) && !(defined SD_OPTION_VERBOSE_BLOCKS))
#define printf      __simple_printf
#endif

/** Pin number for MOSI (master out - slave in) */
#define MOSI                BIT_0
/** Pin number for MISO (master in - slave out) */
#define MISO                BIT_1
/** Pin number for the clock signal */
#define SCLK                BIT_2
/** Pin number for chip select */
#define CS                  BIT_4

#define OLD_FILE    "STUFF.TXT"
#define NEW_FILE    "TEST.TXT"

void error (const uint8_t err);

/**@}*/

#endif /* SD_DEMO_H_ */
