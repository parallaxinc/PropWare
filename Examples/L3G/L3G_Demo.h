/**
 * @file    L3G_Demo.h
 */
/**
 * @brief   Demonstration using various functions of the L3G gyroscope
 *
 * @project L3G_Demo
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

#ifndef L3G_DEMO_H_
#define L3G_DEMO_H_

/**
 * @defgroup    _propware_example_l3g   L3G Gyroscope Demo
 * @ingroup     _propware_examples
 * @{
 */

#include <propeller.h>
#include <PropWare.h>
#include <l3g.h>
#include <safeSpi.h>

/** Pin number for MOSI (master out - slave in) */
#define MOSI                PropWare::GPIO::P0
/** Pin number for MISO (master in - slave out) */
#define MISO                PropWare::GPIO::P1
/** Pin number for the clock signal */
#define SCLK                PropWare::GPIO::P2
/** Pin number for chip select */
#define CS                  PropWare::GPIO::P4
/** Frequency (in Hertz) to run the SPI protocol */
#define FREQ                10000

#define DEBUG_LEDS          PropWare::BYTE_2

void error (const int8_t err);

/**@}*/

#endif /* L3G_DEMO_H_ */
