/**
 * @file        PropWare_Demo.h
 */
/**
 * @brief       Blink an LED on each of the 8 Propeller cogs
 *
 * @detailed    This file is nearly a direct copy of SimpleIDE's blinkcogs.c.
 *              Some changes were made to highlight the helpfulness of PropWare.
 *
 *              Make all propeller cogs blink assigned pins at exactly the same
 *              rate and time to demonstrate the precision of the
 *              _start_cog_thread method. This program and method uses 8 LMM C
 *              program COG "threads" of execution simultaneously.
 *
 *              This program should be compiled with the LMM memory model.
 *
 * @project     PropWare_Demo
 *
 * @author      Modified by David Zemon
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

#ifndef PROPWARE_DEMO_H_
#define PROPWARE_DEMO_H_

/**
 * @defgroup    _propware_example_propware  PropWare Basics
 * @ingroup     _propware_examples
 * @{
 */

#include <tinyio.h>
#include <sys/thread.h>

// Note the lack of an include for propeller.h; This is because PropWare.h will
// include propeller.h for you
#include <PropWare/PropWare.h>
#include <PropWare/pin.h>

#define COGS            8
#define STACK_SIZE      16

/**
 * @brief       Toggle thread function gets started in an LMM COG.
 *
 * @param[in]   *arg    pin number to toggle
 */
void do_toggle (void *arg);

/*@}*/

#endif /* PROPWARE_DEMO_H_ */
