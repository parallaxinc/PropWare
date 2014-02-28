/**
 * @file    MAX6675_Demo.h
 */
/**
 * @brief   Display the temperature on stdout and a character LCD
 *
 * @project MAX6675_Demo
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

#ifndef MAX6675_DEMO_H_
#define MAX6675_DEMO_H_

/**
 * @defgroup    _propware_example_max6675  MAX6675 Demo
 * @ingroup     _propware_examples
 * @{
 */

#include <tinyio.h>
#include <PropWare.h>
#include <spi.h>
#include <hd44780.h>
#include <max6675.h>

/** Pin number for MOSI (master out - slave in) */
#define MOSI            PropWare::GPIO::P0
/** Pin number for MISO (master in - slave out) */
#define MISO            PropWare::GPIO::P1
/** Pin number for the clock signal */
#define SCLK            PropWare::GPIO::P2
/** Pin number for chip select */
#define CS              PropWare::GPIO::P5
#define FREQ            10000

#define DEBUG_LEDS      PropWare::BYTE_2

#define RS              PropWare::GPIO::P14
#define RW              PropWare::GPIO::P12
#define EN              PropWare::GPIO::P10

#define DATA_H          PropWare::GPIO::P26 | PropWare::GPIO::P25 | \
                        PropWare::GPIO::P24 | PropWare::GPIO::P23
#define DATA_L          PropWare::GPIO::P22 | PropWare::GPIO::P21 | \
                        PropWare::GPIO::P20 | PropWare::GPIO::P19

#ifdef DATA_L
#define BITMODE         PropWare::HD44780::BM_8
#define DATA            DATA_H | DATA_L
#else
#define BITMODE         HD44780_4BIT
#define DATA            DATA_H
#endif

#define DIMENSIONS      PropWare::HD44780::DIM_16x2

void error (int8_t err);

/**@}*/

#endif /* MAX6675_DEMO_H_ */
