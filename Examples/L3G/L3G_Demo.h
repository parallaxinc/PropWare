/**
 * @file    L3G_Demo.h
 * 
 * @author  David Zemon
 * @project L3G_Demo
 */

#ifndef L3G_DEMO_H_
#define L3G_DEMO_H_

#include <propeller.h>
#include <PropWare.h>
#include <l3g.h>

#define MOSI        BIT_0
#define MISO        BIT_1
#define SCLK        BIT_2
#define CS          BIT_4
#define FREQ        10000

#define DEBUG_LEDS  BYTE_2

void error (const int8_t err);

#endif /* L3G_DEMO_H_ */
