/**
 * @file    MCP300x_Demo.h
 *
 * @author  David Zemon
 * @project MCP300x_Demo
 */

#ifndef MCP300X_DEMO_H_
#define MCP300X_DEMO_H_

#include <propeller.h>
#include <stdio.h>
#include <stdlib.h>

#include <PropWare.h>
#include <mcp300x.h>

#define MOSI            BIT_0
#define MISO            BIT_1
#define SCLK            BIT_2
#define CS              BIT_3
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
