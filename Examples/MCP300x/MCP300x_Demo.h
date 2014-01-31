/**
 * @file    MCP300x_Demo.h
 */
/**
 * @project MCP300x_Demo
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

#ifndef MCP300X_DEMO_H_
#define MCP300X_DEMO_H_

#include <propeller.h>
#include <stdio.h>
#include <stdlib.h>

#include <PropWare.h>
#include <mcp300x.h>

/** Pin number for MOSI (master out - slave in) */
#define MOSI                BIT_0
/** Pin number for the clock signal */
#define SCLK                BIT_1
/** Pin number for MISO (master in - slave out) */
#define MISO                BIT_2
/** Pin number for chip select */
#define CS                  BIT_3
#define FREQ            100000

// We're going to read from just channel 1 in this demo, but feel free to read
// from any that you like
#define CHANNEL         MCP_CHANNEL_1

#define DEBUG_LEDS      BYTE_2

/**
 * @brief       Report errors to the Debug LEDs for user interpretation
 *
 * @param[in]   err     Error value
 */
void error (int8_t err);

#endif /* MCP300X_DEMO_H_ */
