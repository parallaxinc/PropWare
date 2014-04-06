/**
 * @file    MCP3000_Demo.h
 */
/**
 * @brief   Display the value of an analog channel on stdout and as a bar graph
 *          over the 8 LEDs of the QUICKSTART board
 *
 * @project MCP3000_Demo
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

#ifndef MCP3000_DEMO_H_
#define MCP3000_DEMO_H_

/**
 * @defgroup    _propware_example_mcp300x   MCP3000 Demo
 * @ingroup     _propware_examples
 * @{
 */

#include <propeller.h>
#include <tinyio.h>
#include <stdlib.h>

#include <PropWare/PropWare.h>
#include <PropWare/mcp3000.h>
#include <PropWare/spi.h>
#include <PropWare/pin.h>
#include <PropWare/port.h>

/** Used for determining the bit-width of the ADC channel (10, 12, or 13 bit) */
#define PART_NUMBER     PropWare::MCP3000::MCP300x

/** Pin number for MOSI (master out - slave in) */
#define MOSI            PropWare::Port::P0
/** Pin number for MISO (master in - slave out) */
#define MISO            PropWare::Port::P1
/** Pin number for the clock signal */
#define SCLK            PropWare::Port::P2
/** Pin number for chip select */
#define CS              PropWare::Port::P3
#define FREQ            100000

// We're going to read from just channel 1 in this demo, but feel free to read
// from any that you like
#define CHANNEL         PropWare::MCP3000::CHANNEL_1

#define DEBUG_LEDS      PropWare::BYTE_2

/**
 * @brief       Report errors to the Debug LEDs for user interpretation
 *
 * @param[in]   err     Error value
 */
void error (const PropWare::ErrorCode err);

/**@}*/

#endif /* MCP3000_DEMO_H_ */
