/**
 * @file    ADXL345_Demo.cpp
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

#include <PropWare/PropWare.h>
#include <PropWare/sensor/accelerometer/adxl345.h>

using PropWare::SPI;
using PropWare::Port;
using PropWare::ADXL345;

static const Port::Mask SCLK = Port::P0;
static const Port::Mask MOSI = Port::P1;
static const Port::Mask MISO = Port::P2;
static const Port::Mask CS   = Port::P3;

static const unsigned int AVERAGING_BUFFER_LENGTH = 8;
static const int          PRINT_LOOP_FREQUENCY    = 40;

// Reading at the maximum frequency gives us enough samples in a short enough period of time that we can filter while
// still printing up-to-date values
static const ADXL345::DataRate DATA_RATE = ADXL345::DataRate::_3200_HZ;
static const ADXL345::Range    RANGE     = ADXL345::_2G;

void initialize (const ADXL345 &accelerometer);

/**
 * @example     ADXL345_Demo.cpp
 *
 * Print the current acceleration to the screen in a constant loop while doing a simple averaging filter.
 *
 * @include PropWare_ADXL345/CMakeLists.txt
 */
int main () {
    const ADXL345 accelerometer(CS);
    initialize(accelerometer);

    auto timer = CNT + SECOND / PRINT_LOOP_FREQUENCY;
    while (1) {
        int16_t buffer[AVERAGING_BUFFER_LENGTH][ADXL345::AXES];
        int32_t totals[ADXL345::AXES] = {0, 0, 0};
        float   fValues[ADXL345::AXES];

        // Read the most recent entries
        for (unsigned int i = 0; i < AVERAGING_BUFFER_LENGTH; ++i)
            accelerometer.read(buffer[i]);

        // Create an average for each axis
        for (unsigned int i = 0; i < AVERAGING_BUFFER_LENGTH; ++i)
            for (unsigned int axis = 0; axis < ADXL345::AXES; ++axis)
                totals[axis] += buffer[i][axis];

        for (unsigned int axis = 0; axis < ADXL345::AXES; ++axis)
            fValues[axis] = ADXL345::scale(totals[axis], RANGE) / AVERAGING_BUFFER_LENGTH;

        pwOut.printf("X: %5.2f, Y: %5.2f, Z: %5.2f\n", fValues[ADXL345::X], fValues[ADXL345::Y], fValues[ADXL345::Z]);
        timer = waitcnt2(timer, SECOND / PRINT_LOOP_FREQUENCY);
    }
}

void initialize (const ADXL345 &accelerometer) {
    SPI::get_instance().set_mosi(MOSI);
    SPI::get_instance().set_miso(MISO);
    SPI::get_instance().set_sclk(SCLK);

    ADXL345::RateAndPowerMode rateAndPowerMode;
    rateAndPowerMode.fields.dataRate     = DATA_RATE;
    accelerometer.write(ADXL345::Register::RATE_AND_POWER_MODE, rateAndPowerMode.raw);

    ADXL345::DataFormat dataFormat;
    dataFormat.fields.range          = RANGE;
    accelerometer.write(ADXL345::Register::DATA_FORMAT, dataFormat.raw);

    ADXL345::FIFOControl fifoControl;
    fifoControl.fields.fifoMode = ADXL345::FIFOMode::STREAM;
    accelerometer.write(ADXL345::Register::FIFO_CONTROL, fifoControl.raw);

    accelerometer.start();
}
