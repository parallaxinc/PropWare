/**
 * @file    HD44780_Demo.h
 */
/**
 * @brief   Display "Hello world" on LCD screen and demonstrate line-wrapping
 *
 * @project HD44780_Demo
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

#ifndef HD44780_DEMO_H_
#define HD44780_DEMO_H_

/**
 * @defgroup _propware_examples_hd44780 Character LCD Demo
 * @ingroup _propware_examples
 * @{
 */

#include <propeller.h>
#include <stdio.h>
#include <PropWare.h>
#include <hd44780.h>

#define RS                  BIT_14
#define RW                  BIT_12
#define EN                  BIT_10

#define DATA_H              BIT_26 | BIT_25 | BIT_24 | BIT_23
//#define DATA_L            BIT_22 | BIT_21 | BIT_20 | BIT_19

#ifdef DATA_L
#define BITMODE             HD44780_8BIT
#define DATA                DATA_H | DATA_L
#else
#define BITMODE             HD44780_4BIT
#define DATA                DATA_H
#endif
#define DIMENSIONS          HD44780_16x2

/**
 * @brief       Enter an infinite loop that blinks the error code on the
 *              QUICKSTART's 8 onboard LEDs
 *
 * @param[in]   err     Error code
 */
void error (const uint8_t err);

/**@}*/

/*@}*/

#endif /* HD44780_DEMO_H_ */
