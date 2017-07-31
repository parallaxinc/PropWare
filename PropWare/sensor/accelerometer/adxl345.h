/**
 * @file    PropWare/sensor/accelerometer/adxl345.h
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

#pragma once

#include <PropWare/serial/spi/spi.h>

namespace PropWare {

class ADXL345 {
    public:
        /**
         * @brief   Register map for all registers on the device
         */
        enum class Register {
                /**
                 * @brief Device ID
                 *
                 * Name: `DEVID`, Access: read-only, Reset value: `0b11100101`
                 */
                    DEVICE_ID,
                /**
                 * @brief Tap threshold
                 *
                 * Name: `THRESH_TAP`, Access: read/write, Reset value: `0`
                 */
                    TAP_THRESHOLD = 0x1D,
                /**
                 * @brief X-axis offset
                 *
                 * Name: `OFSX`, Access: read/write, Reset value: `0`
                 */
                    X_AXIS_OFFSET,
                /**
                 * @brief Y-axis offset
                 *
                 * Name: `OFSY`, Access: read/write, Reset value: `0`
                 */
                    Y_AXIS_OFFSET,
                /**
                 * @brief Z-axis offset
                 *
                 * Name: `OFSZ`, Access: read/write, Reset value: `0`
                 */
                    Z_AXIS_OFFSET,
                /**
                 * @brief Tap duration
                 *
                 * Name: `DUR`, Access: read/write, Reset value: `0`
                 */
                    TAP_DURATION,
                /**
                 * @brief Tap latency
                 *
                 * Name: `Latent`, Access: read/write, Reset value: `0`
                 */
                    TAP_LATENCY,
                /**
                 * @brief Tap window
                 *
                 * Name: `Window`, Access: read/write, Reset value: `0`
                 */
                    TAP_WINDOW,
                /**
                 * @brief Activity threshold
                 *
                 * Name: `THRESH_ACT`, Access: read/write, Reset value: `0`
                 */
                    ACTIVITY_THRESHOLD,
                /**
                 * @brief Inactivity threshold
                 *
                 * Name: `THRESH_INACT`, Access: read/write, Reset value: `0`
                 */
                    INACTIVITY_THRESHOLD,
                /**
                 * @brief Inactivity time
                 *
                 * Name: `TIME_INACT`, Access: read/write, Reset value: `0`
                 */
                    INACTIVITY_TIME,
                /**
                 * @brief Axis enable control for activity and inactivity detection
                 *
                 * Name: `ACT_INACT_CTL`, Access: read/write, Reset value: `0`
                 */
                    ACT_INACT_CTL,
                /**
                 * @brief Free-fall threshold
                 *
                 * Name: `THRESH_FF`, Access: read/write, Reset value: `0`
                 */
                    FREE_FALL_THRESHOLD,
                /**
                 *
                 * @brief Free-fall time
                 * Name: `TIME_FF`, Access: read/write, Reset value: `0`
                 */
                    FREE_FALL_TIME,
                /**
                 * @brief Axis control for tap/double tap
                 *
                 * Name: `TAP_AXES`, Access: read/write, Reset value: `0`
                 */
                    TAP_AXES,
                /**
                 *
                 * @brief Source of tap/double tap
                 *
                 * Name: `ACT_TAP_STATUS`, Access: read-only, Reset value: `0`
                 */
                    TAP_SOURCE,
                /**
                 * @brief Data rate and power mode control
                 *
                 * Name: `BW_RATE`, Access: read/write, Reset value: `0b00001010`
                 */
                    RATE_AND_POWER_MODE,
                /**
                 * @brief Control of the power-saving features
                 *
                 * Name: `POWER_CTL`, Access: read/write, Reset value: `0`
                 */
                    POWER_CONTROL,
                /**
                 * @brief Interrupt enable control
                 *
                 * Name: `INT_ENABLE`, Access: read/write, Reset value: `0`
                 */
                    INTERRUPT_ENABLE,
                /**
                 * @brief Interrupt mapping control
                 *
                 * Name: `INT_MAP`, Access: read/write, Reset value: `0`
                 */
                    INTERRUPT_MAPPING,
                /**
                 * @brief Source of interrupts
                 *
                 * Name: `INT_SOURCE`, Access: read-only, Reset value: `0b00000010`
                 */
                    INTERRUPT_SOURCES,
                /**
                 * @brief Data format control
                 *
                 * Name: `DATA_FORMAT`, Access: read/write, Reset value: `0`
                 */
                    DATA_FORMAT,
                /**
                 * @brief X-Axis Data 0
                 *
                 * Name: `DATAX0`, Access: read-only, Reset value: `0`
                 */
                    X_AXIS_0,
                /**
                 * @brief X-Axis Data 1
                 *
                 * Name: `DATAX1`, Access: read-only, Reset value: `0`
                 */
                    X_AXIS_1,
                /**
                 * @brief Y-Axis Data 0
                 *
                 * Name: `DATAY0`, Access: read-only, Reset value: `0`
                 */
                    Y_AXIS_0,
                /**
                 * @brief Y-Axis Data 1
                 *
                 * Name: `DATAY1`, Access: read-only, Reset value: `0`
                 */
                    Y_AXIS_1,
                /**
                 * @brief Z-Axis Data 0
                 *
                 * Name: `DATAZ0`, Access: read-only, Reset value: `0`
                 */
                    Z_AXIS_0,
                /**
                 * @brief Z-Axis Data 1
                 *
                 * Name: `DATAZ1`, Access: read-only, Reset value: `0`
                 */
                    Z_AXIS_1,
                /**
                 * @brief FIFO control
                 *
                 * Name: `FIFO_CTL`, Access: read/write, Reset value: `0`
                 */
                    FIFO_CONTROL,
                /**
                 * @brief FIFO status
                 *
                 * Name: `FIFO_STATUS`, Access: read-only, Reset value: `0`
                 */
                    FIFO_STATUS
        };

        /**
         * @brief   Convenience enumeration for selecting axes via the ADXL345::read_axis() method
         */
        typedef enum {
            X,
            Y,
            Z,
            AXES
        } Axis;

        /**
         * @brief   Data rate codes for the Register::RATE_AND_POWER_MODE register
         *
         * Enumeration names are based on the data rate with underscores representing decimal points for 6.25 and
         * 12.5 Hz.
         */
        typedef enum {
            /** Output data rate (Hz) = 6.25, Bandwidth (Hz) = 3.125, Current (uA) = 145 */ _6_25_HZ = 6,
            /** Output data rate (Hz) = 12.5, Bandwidth (Hz) = 6.25, Current (uA) = 100 */  _12_5_HZ,
            /** Output data rate (Hz) = 25, Bandwidth (Hz) = 12.5, Current (uA) = 145 */    _25_HZ,
            /** Output data rate (Hz) = 50, Bandwidth (Hz) = 25, Current (uA) = 145 */      _50_HZ,
            /** Output data rate (Hz) = 100, Bandwidth (Hz) = 50, Current (uA) = 145 */     _100_HZ,
            /** Output data rate (Hz) = 200, Bandwidth (Hz) = 100, Current (uA) = 145 */    _200_HZ,
            /** Output data rate (Hz) = 400, Bandwidth (Hz) = 200, Current (uA) = 100 */    _400_HZ,
            /** Output data rate (Hz) = 800, Bandwidth (Hz) = 400, Current (uA) = 65 */     _800_HZ,
            /** Output data rate (Hz) = 1600, Bandwidth (Hz) = 800, Current (uA) = 55 */    _1600_HZ,
            /** Output data rate (Hz) = 3200, Bandwidth (Hz) = 1600, Current (uA) = 40 */   _3200_HZ
        } DataRate;

        /**
         * @brief   Wakeup frequency codes for the Register::POWER_SAVINGS register
         */
        typedef enum {
            /** Device will wake up 8 times per second to take readings */ HZ_8,
            /** Device will wake up 4 times per second to take readings */ HZ_4,
            /** Device will wake up 2 times per second to take readings */ HZ_2,
            /** Device will wake up 1 times per second to take readings */ HZ_1
        } WakeupFrequency;

        /**
         * @brief   Selection of data ranges
         *
         * Larger data ranges decrease the device's precision
         */
        typedef enum {
            /** Data range will be +/- 2 g */  _2G,
            /** Data range will be +/- 4 g */  _4G,
            /** Data range will be +/- 8 g */  _8G,
            /** Data range will be +/- 16 g */ _16G
        } Range;

        /**
         * @brief   Data codes for the various modes of operation that the FIFO can assume.
         */
        typedef enum {
            /**
             * FIFO is bypassed.
             */
                BYPASS,
            /**
             * FIFO collects up to 32 values and then stops collecting data, collecting new data only when FIFO is
             * not full.
             */
                FIFO,
            /**
             * FIFO holds the last 32 data values. When FIFO is full, the oldest data is overwritten with newer data.
             */
                STREAM,
            /**
             * When triggered by the trigger bit, FIFO holds the last data samples before the trigger event and then
             * continues to collect data until full. New data is collected only when FIFO is not full.
             */
                TRIGGER
        } FIFOMode;

        /**
         * @brief   Bit-mapping for Register::ACT_INACT_CTL
         */
        union ActivityDetection {
            struct {
                /** Enable z-axis participation in detecting inactivity */
                unsigned int inactiveZEnable: 1;
                /** Enable y-axis participation in detecting inactivity */
                unsigned int inactiveYEnable: 1;
                /** Enable x-axis participation in detecting inactivity */
                unsigned int inactiveXEnable: 1;
                /** Select between ac-coupled and dc-couple operation. Low for dc-coupled, high for ac-coupled. */
                unsigned int inactiveAcDc: 1;
                /** Enable z-axis participation in detecting activity */
                unsigned int activeZEnable: 1;
                /** Enable y-axis participation in detecting activity */
                unsigned int activeYEnable: 1;
                /** Enable x-axis participation in detecting activity */
                unsigned int activeXEnable: 1;
                /** Select between ac-coupled and dc-couple operation. Low for dc-coupled, high for ac-coupled. */
                unsigned int activeAcDc: 1;
            }       fields;
            uint8_t raw;
        };

        /**
         * @brief   Bit-mapping for Register::TAP_AXES
         */
        union TapAxes {
            struct {
                /** Enable z-axis participation in tap detection */
                unsigned int tapZEnable: 1;
                /** Enable y-axis participation in tap detection */
                unsigned int tapYEnable: 1;
                /** Enable x-axis participation in tap detection */
                unsigned int tapXEnable: 1;
                /**
                 * When set high, suppress double-tap detection for too high of an acceleration (as set per
                 * Register::TAP_THRESHOLD)
                 */
                unsigned int suppress: 1;
            }       fields;
            uint8_t raw;
        };

        /**
         * @brief   Bit-mapping for Register::TAP_SOURCE
         */
        union TapSource {
            struct {
                /** When set, a tap was detected on the z-axis */
                unsigned int zTapped: 1;
                /** When set, a tap was detected on the y-axis */
                unsigned int yTapped: 1;
                /** When set, a tap was detected on the x-axis */
                unsigned int xTapped: 1;
                /** When set, the device is sleeping */
                unsigned int asleep: 1;
                /** When set, activity was detected on the z-axis */
                unsigned int zActivity: 1;
                /** When set, activity was detected on the y-axis */
                unsigned int yActivity: 1;
                /** When set, activity was detected on the x-axis */
                unsigned int xActivity: 1;
            }       fields;
            uint8_t raw;
        };

        /**
         * @brief   Bit-mapping for Register::RATE_AND_POWER_MODE
         */
        union RateAndPowerMode {
            struct {
                /**
                 * Set the data polling rate.
                 */
                DataRate     dataRate: 3;
                /**
                 * Enable or disable low-power mode. Low power mode has a somewhat higher noise but reduces power
                 * consumption. True to enable low power mode, false for normal operation.
                 */
                unsigned int lowPowerMode: 1;
            }       fields;
            uint8_t raw;
        };

        /**
         * @brief   Bit-mapping for Register::POWER_SAVINGS
         */
        union PowerControl {
            struct {
                /**
                 * Frequency of reading when in sleep mode
                 */
                WakeupFrequency wakeup: 2;
                /**
                 * When set high, the device will be put into sleep mode. Low for normal operation. While in sleep
                 * mode, the device's sample rate is set by PowerControl::wakeup.
                 *
                 * When clearing the sleep bit, it is recommended that the part be placed into standby mode and then
                 * set back to measurement mode with a subsequent write. This is done to ensure that the device is
                 * properly biased if sleep mode is manually disabled; otherwise, the first few samples of data after
                 * the sleep bit is cleared may have additional noise, especially if the device was asleep when the
                 * bit was cleared
                 */
                unsigned int    sleep: 1;
                /**
                 * Set high for normal operation and low for standby mode. When in standby mode, power consumption is
                 * at a minimum but now measurements can be made. The device is booted in standby mode.
                 */
                unsigned int    measure: 1;
                /**
                 * When set high, the device will automatically enter a low-power sleep mode any time the sampled
                 * activity dips below the threshold configured via Register::INACTIVITY_THRESHOLD for at least the
                 * amount of time configured via Register::INACTIVITY_TIME. Clearing the bit low will disable
                 * automatic sleep mode.
                 *
                 * When clearing the AUTO_SLEEP bit, it is recommended that the part be placed into standby mode and
                 * then set back to measurement mode with a subsequent write. This is done to ensure that the device
                 * is properly biased if sleep mode is manually disabled; otherwise, the first few samples of data
                 * after the AUTO_SLEEP bit is cleared may have additional noise, especially if the device was asleep
                 * when the bit was cleared.
                 */
                unsigned int    autoSleep: 1;
                /**
                 * Set high to enable linked mode and clear low for concurrent functionality of inactivity and
                 * activity functions. See the datasheet for a far more verbose explanation.
                 */
                unsigned int    link: 1;
            }       fields;
            uint8_t raw;
        };

        /**
         * @brief   Bit-mapping for Register::INTERRUPT_ENABLE
         */
        union InterruptEnable {
            struct {
                /** Enable interrupt generation for the overrun event */ unsigned int    overrun: 1;
                /** Enable interrupt generation for the watermark event */ unsigned int  watermark: 1;
                /** Enable interrupt generation for the free fall event */ unsigned int  freeFall: 1;
                /** Enable interrupt generation for the inactivity event */ unsigned int inactivity: 1;
                /** Enable interrupt generation for the activity event */ unsigned int   activity: 1;
                /** Enable interrupt generation for the double tap event */ unsigned int doubleTap: 1;
                /** Enable interrupt generation for the single tap event */ unsigned int singleTap: 1;
                /** Enable interrupt generation for the data ready event */ unsigned int dataReady: 1;
            }       fields;
            uint8_t raw;
        };

        /**
         * @brief   Bit-mapping for Register::INTERRUPT_MAPPING
         */
        union InterruptMapping {
            struct {
                /**
                 * When set high, the overrun event will be mapped to pin INT1, otherwise INT0.
                 */ unsigned int
                    overrun: 1;
                /**
                 * When set high, the watermark event will be mapped to pin INT1, otherwise INT0.
                 */ unsigned int
                    watermark: 1;
                /**
                 * When set high, the free fall event will be mapped to pin INT1, otherwise INT0.
                 */ unsigned int
                    freeFall: 1;
                /**
                 * When set high, the inactivity event will be mapped to pin INT1, otherwise INT0.
                 */ unsigned int
                    inactivity: 1;
                /**
                 * When set high, the activity event will be mapped to pin INT1, otherwise INT0.
                 */ unsigned int
                    activity: 1;
                /**
                 * When set high, the double tap event will be mapped to pin INT1, otherwise INT0.
                 */ unsigned int
                    doubleTap: 1;
                /**
                 * When set high, the single tap event will be mapped to pin INT1, otherwise INT0.
                 */ unsigned int
                    singleTap: 1;
                /**
                 * When set high, the data ready event will be mapped to pin INT1, otherwise INT0.
                 */ unsigned int
                    dataReady: 1;
            }       fields;
            uint8_t raw;
        };

        /**
         * @brief   Bit-mapping for Register::INTERRUPT_SOURCES
         */
        union InterruptSources {
            struct {
                /** When set, an overrun event has occurred. */ unsigned int    overrun: 1;
                /** When set, a watermark event has occurred. */ unsigned int   watermark: 1;
                /** When set, a free fall event has occurred. */ unsigned int   freeFall: 1;
                /** When set, an inactivity event has occurred. */ unsigned int inactivity: 1;
                /** When set, an activity event has occurred. */ unsigned int   activity: 1;
                /** When set, a double tap event has occurred. */ unsigned int  doubleTap: 1;
                /** When set, a single tap event has occurred. */ unsigned int  singleTap: 1;
                /** When set, a data ready event has occurred. */ unsigned int  dataReady: 1;
            }       fields;
            uint8_t raw;
        };

        /**
         * @brief   Bit-mapping for Register::DATA_FORMAT
         */
        union DataFormat {
            struct {
                /**
                 * Data range of the device
                 */
                Range        range: 2;
                /**
                 * When set high, data will be left (MSB) justified. When cleared low, it will be right justified and
                 * sign extended.
                 */
                unsigned int justify: 1;
                /**
                 * When set high, the resolution will always be 4 mg/LSB no matter the range. When set low, the
                 * resolution will change with the range and the data will remain 10 bits wide.
                 */
                unsigned int fullResolution: 1;
                /**
                 * When set high, interrupts will be active high instead of active low.
                 */
                unsigned int invertedInterrupts: 1;
                /**
                 * When set high, the device will work in 3-wire SPI mode. Cleared low for 4-wire SPI mode.
                 */
                unsigned int spiMode;
                /**
                 * Setting high will enable self-test mode, causing a shift in the output data. Cleared low for
                 * normal operation.
                 */
                unsigned int selfTest;
            }       fields;
            uint8_t raw;
        };

        /**
         * @brief   Bit-mapping for Register::FIFO_CONTROL
         */
        union FIFOControl {
            struct {
                /**
                 * The function of these bits depends on the FIFO mode selected via FIFOContro::fifoMode. Entering a
                 * value of 0 in the samples bits immediately sets the watermark status bit in
                 * Register::INTERRUPT_SOURCE, regardless of which FIFO mode is selected. Undesirable operation may
                 * occur if a value of 0 is used for the samples bits when trigger mode is used.
                 */
                unsigned int samples: 5;
                /**
                 * Clear low to link the trigger event of trigger mode to INT1, and set high to link the trigger
                 * event to INT2.
                 */
                unsigned int trigger: 1;
                /**
                 * Set the mode that the FIFO will assume.
                 */
                FIFOMode     fifoMode: 2;
            }       fields;
            uint8_t raw;
        };

        /**
         * @brief   Bit-mapping for Register::FIFO_STATUS
         */
        union FIFOStatus {
            struct {
                /** These bits report how many data values are stored in FIFO. */ unsigned int         entries: 6;
                /** Unused bit */ unsigned int                                                         unused: 1;
                /** Set high when a trigger event is occurring. Cleared low otherwise. */ unsigned int fifoTrigger: 1;
            }       fields;
            uint8_t raw;
        };

    public:
        static const SPI::Mode SPI_MODE = SPI::Mode::MODE_3;

        /**
         * @brief   The ADXL345 is hardwired for a device ID of `0xE5`
         */
        static const uint8_t DEVICE_ID = 0xE5;

        /**
         * @brief   The highest resolution that the device is capable of. Resolution will become less precise (larger
         *          number) as Range increases. Use the ADXL345::scale method to get the human-readable value for
         *          acceleration.
         */
        static constexpr double FULL_RESOLUTION_SCALE = 0.004;

        /**
         * @brief       Scale the non-justified, raw accelerometer value to a human-readable number
         *
         * This method is only necessary when the fullResolution bit of ADXL345::DataFormat is cleared low. If it is
         * high, simply multiply the raw value by ADXL345::FULL_RESOLUTION_SCALE to achieve the correct result.
         *
         * @param[in]   value   Raw accelerometer
         * @param[in]   range   The range that was selected at the time the reading was made.
         *
         * @return      Human-readable acceleration value for the requested axis
         */
        static double scale (const int value, const Range range) {
            return value * (1 << range) * FULL_RESOLUTION_SCALE;
        }

    public:
        /**
         * @brief                           Create an object which communicates over the given SPI bus
         *
         * @param[in]   bus                 SPI bus for communication
         * @param[in]   csMask              Chip-select's pin mask
         * @param[in]   alwaysSetSPIMode    When true, the SPI bus's mode will always be set prior to communication
         */
        ADXL345 (SPI &bus, const Pin::Mask csMask, const bool alwaysSetSPIMode = false)
            : m_bus(&bus),
              m_cs(csMask, Pin::Dir::OUT),
              m_alwaysSetMode(alwaysSetSPIMode) {
            if (!alwaysSetSPIMode)
                this->m_bus->set_mode(SPI_MODE);
            this->m_cs.set();
        }

        /**
         * @brief                           Create an object which communicates over the shared SPI bus
         *
         * @param[in]   csMask              Chip-select's pin mask
         * @param[in]   alwaysSetSPIMode    When true, the SPI bus's mode will always be set prior to communication
         */
        ADXL345 (const Pin::Mask csMask, const bool alwaysSetSPIMode = true)
            : m_bus(&SPI::get_instance()),
              m_cs(csMask, Pin::Dir::OUT),
              m_alwaysSetMode(alwaysSetSPIMode) {
            if (!alwaysSetSPIMode)
                this->m_bus->set_mode(SPI::Mode::MODE_1);
            this->m_cs.set();
        }

        /**
         * @brief       Choose whether to always set the SPI mode before writing to the device; Useful when multiple
         *              devices with different SPI specifications are connected to the bus
         *
         * @param[in]   alwaysSetMode   For any non-zero value, the SPI modes will always be set before a write routine
         */
        void always_set_spi_mode (const bool alwaysSetMode) {
            this->m_alwaysSetMode = alwaysSetMode;
        }

        /**
         * @brief       Convenience function to start the device in a sane fashion for basic reading
         *
         * See the documentation of PowerControl::fields for detailed descriptions of each parameter
         *
         * @param[in]   autoSleep           Set the auto-sleep feature
         * @param[in]   link                Set the link feature
         * @param[in]   sleep               Set the sleep mode
         * @param[in]   measure             Set the standby mode
         * @param[in]   wakeupFrequency     Set the wakeup frequency
         */
        void start (const bool autoSleep = false, const bool link = false, const bool sleep = false,
                    const bool measure = true, const WakeupFrequency wakeupFrequency = WakeupFrequency::HZ_8) const {
            this->write(Register::POWER_CONTROL, PowerControl{fields: {
                wakeup: wakeupFrequency,
                sleep: static_cast<unsigned int>(sleep),
                measure: static_cast<unsigned int>(measure),
                autoSleep: static_cast<unsigned int>(autoSleep),
                link: static_cast<unsigned int>(link)
            }}.raw);
        }

        /**
         * @brief       Retrieve the values from all 3 axes with a fast, 6-byte successive read
         *
         * @param[out]  values  Memory where 6-bytes can be stored. Note that this MUST be an array of 2-byte
         *                      variables. DO NOT cast an array of 4-byte variables (such as an array of ints) and
         *                      pass that address in.
         */
        void read (int16_t *values) const {
            this->read(Register::X_AXIS_0, AXES, values);
        }

        /**
         * @brief       Read data for all three axes and return only the requested axis
         *
         * @param[in]   axis    Axis which should be read
         *
         * @return      Reading from the requested axis
         */
        int16_t read_axis (const Axis axis) const {
            int16_t temp[3];
            this->read(temp);
            return temp[static_cast<size_t>(axis)];
        }

        /**
         * @brief   Read data for all three axes and return only the x-axis
         *
         * @return  Reading from the x-axis
         */
        int16_t read_x () const {
            return this->read_axis(Axis::X);
        }

        /**
         * @brief   Read data for all three axes and return only the y-axis
         *
         * @return  Reading from the y-axis
         */
        int16_t read_y () const {
            return this->read_axis(Axis::Y);
        }

        /**
         * @brief   Read data for all three axes and return only the z-axis
         *
         * @return  Reading from the z-axis
         */
        int16_t read_z () const {
            return this->read_axis(Axis::Z);
        }

        /**
         * @brief       Perform a manual write to the device
         *
         * @param[in]   address     Address of the register that should be modified
         * @param[in]   value       Value that should be written to the register
         */
        void write (const Register address, const uint8_t value) const {
            if (this->m_alwaysSetMode)
                this->m_bus->set_mode(SPI_MODE);

            this->m_cs.clear();
            this->m_bus->shift_out(8, static_cast<uint32_t>(address));
            this->m_bus->shift_out(8, value);
            this->m_cs.set();
        }

        /**
         * @brief       Perform a single manual read from the device
         *
         * @param[in]   address     Address of the register that should be read
         *
         * @return      Value in the register
         */
        uint8_t read (const Register address) const {
            if (this->m_alwaysSetMode)
                this->m_bus->set_mode(SPI_MODE);

            this->m_cs.clear();
            this->m_bus->shift_out(8, BIT_7 | static_cast<uint32_t>(address));
            const uint8_t result = static_cast<const uint8_t>(this->m_bus->shift_in(8));
            this->m_cs.set();
            return result;
        }

        /**
         * @brief       Perform a manual, multi-byte read from the device
         *
         * @param[in]   address     Address of the first register to read
         * @param[in]   words       Number of 2-byte words to read
         * @param[out]  result[]    Memory location were resulting data can be stored
         */
        void read (const Register startingAddress, const size_t words, int16_t *result) const {
            if (this->m_alwaysSetMode)
                this->m_bus->set_mode(SPI_MODE);

            this->m_cs.clear();
            this->m_bus->shift_out(8, BIT_7 | BIT_6 | static_cast<uint32_t>(startingAddress));
            for (size_t i = 0; i < words; ++i) {
                const uint8_t lowByte  = static_cast<const uint8_t>(this->m_bus->shift_in(8));
                const uint8_t highByte = static_cast<const uint8_t>(this->m_bus->shift_in(8));
                result[i] = highByte << 8 | lowByte;
            }
            this->m_cs.set();
        }

        /**
         * @brief       Set a single bit in a register on the device
         *
         * This method is highly inefficient (runtime performance) for multi-bit modifications. Use this only when
         * code size or code legibility is more important than runtime performance.
         *
         * @param[in]   address     Address of the register to modify
         * @param[in]   bit         Bit in the register that should be set high
         */
        void set_bit (const Register address, const Bit bit) const {
            const auto startingValue = this->read(address);
            this->write(address, bit | startingValue);
        }

        /**
         * @brief       Clear a single bit in a register on the device
         *
         * This method is highly inefficient (runtime performance) for multi-bit modifications. Use this only when
         * code size or code legibility is more important than runtime performance.
         *
         * @param[in]   address     Address of the register to modify
         * @param[in]   bit         Bit in the register that should be cleared low
         */
        void clear_bit (const Register address, const Bit bit) const {
            const auto startingValue = this->read(address);
            this->write(address, ~bit & startingValue);
        }

    private:
        SPI       *m_bus;
        const Pin m_cs;
        bool      m_alwaysSetMode;
};

}
