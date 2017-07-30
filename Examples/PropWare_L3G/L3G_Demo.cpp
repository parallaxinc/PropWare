/**
 * @file    L3G_Demo.cpp
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

// Includes
#include <PropWare/sensor/gyroscope/l3g.h>

using PropWare::Port;
using PropWare::SPI;
using PropWare::L3G;

static const unsigned int AVERAGING_BUFFER_LENGTH = 16;
static const int          PRINT_LOOP_FREQUENCY    = 40;
static const L3G::DPSMode DEGREES_PER_SECOND      = L3G::DPS_250;
static const unsigned int PERIOD                  = SECOND / PRINT_LOOP_FREQUENCY;

static const Port::Mask SCLK = Port::P0;
static const Port::Mask MOSI = Port::P1;
static const Port::Mask MISO = Port::P2;
static const Port::Mask CS   = Port::P4;


void read_average (const L3G &gyro, float *result);

void print_graph (const int markerIndex);

/**
 * @example     L3G_Demo.cpp
 *
 * Print a simple graph of the rotational speed along a single axis
 *
 * @include PropWare_L3G/CMakeLists.txt
 */
int main () {
    SPI spi = SPI::get_instance();
    spi.set_mosi(MOSI);
    spi.set_miso(MISO);
    spi.set_sclk(SCLK);
    spi.set_mode(L3G::SPI_MODE);
    spi.set_bit_mode(L3G::SPI_BITMODE);
    L3G gyro(spi, CS);

    // Select a reasonable configuration for playing with a gyro on your desk.
    gyro.set_dps(DEGREES_PER_SECOND);
    gyro.write(L3G::Register::CTRL_REG1, 0b11001111); // Data rate = 760 Hz, Low-pass filter = 30 Hz
    gyro.write(L3G::Register::CTRL_REG2, 6); // High-pass filter = 0.9 Hz
    gyro.write(L3G::Register::CTRL_REG5, PropWare::BIT_6 | PropWare::BIT_4); // Enable FIFO & high-pass filter
    gyro.write(L3G::Register::FIFO_CTRL_REG, PropWare::BIT_6); // Set FIFO for stream mode

    auto timer = CNT + PERIOD;
    while (1) {
        float values[L3G::AXES];
        read_average(gyro, values);
        print_graph(static_cast<const int>(values[L3G::X]));
        timer = waitcnt2(timer, PERIOD);
    }
}

void read_average (const L3G &gyro, float *result) {
    int16_t buffer[AVERAGING_BUFFER_LENGTH][L3G::AXES];
    int32_t totals[] = {0, 0, 0};

    // Read the most recent entries
    for (unsigned int i = 0; i < AVERAGING_BUFFER_LENGTH; ++i)
        gyro.read(buffer[i]);

    // Create an average for each axis
    for (unsigned int i = 0; i < AVERAGING_BUFFER_LENGTH; ++i)
        for (unsigned int axis = 0; axis < L3G::AXES; ++axis)
            totals[axis] += buffer[i][axis];

    for (unsigned int axis = 0; axis < L3G::AXES; ++axis)
        result[axis] = L3G::to_dps(totals[axis], DEGREES_PER_SECOND) / AVERAGING_BUFFER_LENGTH;
}

void print_graph (const int markerIndex) {
    pwOut << '|';
    for (int i = -50; i <= 50; ++i) {
        if (i == markerIndex)
            pwOut << '*';
        else if (i < 0)
            pwOut << ' ';
        else if (i == 0)
            pwOut << '|';
        else
            pwOut << ' ';
    }
    pwOut << '|' << '\n';
}
