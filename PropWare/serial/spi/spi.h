/**
 * @file        PropWare/serial/spi/spi.h
 *
 * @author      David Zemon
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

#include <PropWare/PropWare.h>
#include <PropWare/gpio/pin.h>
#include <PropWare/hmi/output/printer.h>
#include <PropWare/hmi/input/scancapable.h>

namespace PropWare {

/**
 * @brief   SPI serial communications library; Core functionality comes from a dedicated assembly cog
 *
 * Generally, multiple instances of the SPI class are not desired. To avoid the programmer from accidentally creating
 * multiple instances, this class is set up as a singleton. A static instance can be retrieved with
 * get_instance(). If multiple instances of PropWare::SPI are desired, the PropWare library (and your
 * project) should be built from source with PROPWARE_NO_SAFE_SPI defined
 */
class SPI : public PrintCapable,
            public ScanCapable {
    public:
        /**
         * @brief   Descriptor for SPI signal as defined by Motorola modes
         *
         * CPOL 0 refers to a low polarity (where the clock idles in the low state) and CPOL 1 is for high polarity.
         *
         * TODO: Describe phase
         * <table><tr><td>SPI Mode</td><td>CPOL</td><td>CPHA</td></tr><tr><td>0</td><td>0</td><td>0</td></tr><tr>
         * <td>1</td><td>0</td><td>1</td></tr><tr><td>2</td><td>1</td><td>0</td></tr><tr><td>3</td><td>1</td><td>1</td>
         * </tr></table>
         */
        /* Raw text version of the above HTML table
         *
         * SPI Mode     CPOL    CPHA
         * 0            0       0
         * 1            0       1
         * 2            1       0
         * 3            1       1
         */
        typedef enum {
            /** Mode 0 */MODE_0,
            /** Mode 1 */MODE_1,
            /** Mode 2 */MODE_2,
            /** Mode 3 */MODE_3
        } Mode;

        /**
         * @brief   Determine if data is communicated with the LSB or MSB sent/received first
         *
         * @note    Initial value is SPI_MODES + 1 making them easily distinguishable
         */
        typedef enum {
            /**
             * Start the enumeration where Mode left off; this ensures no overlap
             */
                    LSB_FIRST = MODE_3 + 1,
                    MSB_FIRST
        } BitMode;

        /**
         * Error codes - Proceeded by nothing
         */
        typedef enum {
            /** No error */           NO_ERROR     = 0,
            /** First SPI error */    BEG_ERROR    = 1,
            /** SPI Error 1 */        INVALID_FREQ = BEG_ERROR,
            /** Last SPI error code */END_ERROR    = INVALID_FREQ
        } ErrorCode;

    public:
        static const int32_t DEFAULT_FREQUENCY = 100000;

    public:
        /**
         * @brief       Best way to access an SPI instance is through here, where you can get a shared instance of the
         *              SPI module (not thread safe)
         *
         * @returns     Address of the shared SPI instance
         */
        static SPI &get_instance () {
            static SPI defaultInstance;
            return defaultInstance;
        }

    public:
        /**
         * @brief       Construct an SPI bus on the given pins with the given settings
         *
         * @param[in]   mosi        Pin mask for MOSI
         * @param[in]   miso        Pin mask for MISO
         * @param[in]   sclk        Pin mask for SCLK
         * @param[in]   frequency   Frequency to run the bus, in hertz
         * @param[in]   mode        Determines clock phase and polarity
         * @param[in]   bitmode     Determine if the MSB or LSB should be clocked out first
         */
        SPI (const Port::Mask mosi = PropWare::Port::NULL_PIN, const Port::Mask miso = PropWare::Port::NULL_PIN,
             const Port::Mask sclk = PropWare::Port::NULL_PIN, const int32_t frequency = DEFAULT_FREQUENCY,
             const Mode mode = MODE_0, const BitMode bitmode = MSB_FIRST)
                : m_bitmode(bitmode) {
            this->set_mosi(mosi);
            this->set_miso(miso);
            this->set_sclk(sclk);
            this->set_clock(frequency);
        }

        /**
         * @brief   Release the pins to floating inputs
         */
        ~SPI () {
            this->m_mosi.set_dir_in();
            this->m_sclk.set_dir_in();
        }

        /**
         * @brief   Release the current MOSI pin as a floating input and set the new one as output
         */
        void set_mosi (const Port::Mask mask) {
            this->reset_pin_mask(this->m_mosi, mask);
        }

        /**
         * @brief   Set the new pin as input
         */
        void set_miso (const Port::Mask mask) {
            this->reset_pin_mask(this->m_miso, mask);
            this->m_miso.set_dir_in();
        }

        /**
         * @brief   Release the current SCLK pin as a floating input and set the new one as output
         */
        void set_sclk (const Port::Mask mask) {
            this->reset_pin_mask(this->m_sclk, mask);
            this->set_mode(this->m_mode);
        }

        /**
         * @brief       Set the mode of SPI communication
         *
         * @param[in]   mode    Sets the SPI mode to one MODE_0, MODE_1, MODE_2, or MODE_3
         *
         * @return      Can return non-zero in the case of a timeout
         */
        void set_mode (const Mode mode) {
            this->m_mode = mode;

            if (0x02 & mode)
                this->m_sclk.set();
            else
                this->m_sclk.clear();
        }

        /**
         * @brief       Set the bitmode of SPI communication
         *
         * @param[in]   bitmode     Select one of LSB_FIRST or MSB_FIRST to choose which bit will be shifted
         *                          out first
         *
         * @return      Can return non-zero in the case of a timeout
         */
        void set_bit_mode (const BitMode bitmode) {
            this->m_bitmode = bitmode;
        }

        /**
         * @brief       Change the SPI module's clock frequency
         *
         * @param[in]   frequency   Frequency, in Hz, to run the SPI clock; Must be less than CLKFREQ/4 (for 80 MHz,
         *                          900 kHz is the fastest I've tested successfully)
         *
         * @return      Returns 0 upon success, otherwise error code
         */
        PropWare::ErrorCode set_clock (const int32_t frequency) {
            static const int32_t MAX_CLOCK = CLKFREQ / 80;
            if (MAX_CLOCK <= frequency || 0 > frequency)
                return INVALID_FREQ;
            this->m_clkDelay = (CLKFREQ / frequency) >> 1;
            return NO_ERROR;
        }

        /**
         * @brief       Retrieve the SPI module's clock frequency
         *
         * @return      Returns 0 upon success, otherwise error code
         */
        int32_t get_clock () const {
            return CLKFREQ / (this->m_clkDelay << 1);
        }

        /**
         * @brief       Send a value out to a peripheral device
         *
         * Pass a value and mode into the assembly cog to be sent to the peripheral; NOTE: this function is non-blocking
         * and chip-select should not be set inactive immediately after the return (you should call spi_wait() before
         * setting chip-select inactive)
         *
         * @param[in]   bits        Number of bits to be shifted out
         * @param[in]   value       The value to be shifted out
         *
         * @return      Returns 0 upon success, otherwise error code
         */
        void shift_out (uint8_t bits, uint32_t value) const {
            switch (this->m_bitmode) {
                case MSB_FIRST:
                    this->shift_out_msb_first(bits, value);
                    break;
                case LSB_FIRST:
                    this->shift_out_lsb_first(bits, value);
                    break;
            }
        }

        /**
         * @brief       Read a value from the MISO line
         *
         * @param[in]   bits    Number of bits to read
         *
         * @returns     Value from the data bus
         */
        uint32_t shift_in (const unsigned int bits) const {
            const bool clockPhase = this->m_mode & 0x01;
            if (clockPhase) {
                switch (this->m_bitmode) {
                    case MSB_FIRST:
                        return this->shift_in_msb_phs1(bits);
                    case LSB_FIRST:
                        return this->shift_in_lsb_phs1(bits);
                }
            } else {
                switch (this->m_bitmode) {
                    case MSB_FIRST:
                        return this->shift_in_msb_phs0(bits);
                    case LSB_FIRST:
                        return this->shift_in_lsb_phs0(bits);
                }
            }
            return (uint32_t) -1;
        }

        /**
         * @brief       Send an array of data at max transmit speed. Mode is always MODE_0 and data is always MSB first
         *
         * @param[in]   buffer[]        Address where data is stored
         * @param[in]   numberOfBytes   Number of words to send
         */
        void shift_out_block_msb_first_fast (const uint8_t buffer[], size_t numberOfBytes) const {
            __asm__ volatile (
#define ASMVAR(name) FC_ADDR(#name "%=",  "SpiBlockWriteStart%=")
            FC_START("SpiBlockWriteStart%=", "SpiBlockWriteEnd%=")
                    "       jmp #" FC_ADDR("outerLoop%=", "SpiBlockWriteStart%=") "                             \n\t"

                    // Temporary variables
                    "bitIdx%=:                                                                                  \n\t"
                    "       nop                                                                                 \n\t"
                    "data%=:                                                                                    \n\t"
                    "       nop                                                                                 \n\t"


                    "outerLoop%=:                                                                               \n\t"
                    "       rdbyte " ASMVAR(data) ", %[_bufAdr]                                                 \n\t"
                    "       mov " ASMVAR(bitIdx) ", #8                                                          \n\t"
                    "       ror " ASMVAR(data) ", " ASMVAR(bitIdx) "                                            \n\t"

                    "loop%=:                                                                                    \n\t"
                    "       rol " ASMVAR(data) ", #1 wc                                                         \n\t"
                    "       muxc outa, %[_mosi]                                                                 \n\t"
                    "       xor outa, %[_sclk]                                                                  \n\t"
                    "       xor outa, %[_sclk]                                                                  \n\t"
                    "       djnz " ASMVAR(bitIdx) ", #" FC_ADDR("loop%=", "SpiBlockWriteStart%=") "             \n\t"

                    // Write the word back to the buffer in HUB memory
                    "       add %[_bufAdr], #1                                                                  \n\t"

                    "       djnz %[_numberOfBytes], #" FC_ADDR("outerLoop%=", "SpiBlockWriteStart%=") "        \n\t"

                    "       or outa, %[_mosi]                                                                   \n\t"
                    FC_END("SpiBlockWriteEnd%=")
#undef ASMVAR
            : [_bufAdr] "+r"(buffer),
            [_numberOfBytes] "+r"(numberOfBytes)
            :[_mosi] "r"(this->m_mosi.get_mask()),
            [_sclk] "r"(this->m_sclk.get_mask())
            );
        }

        /**
         * @brief       Receive an array of data at max transmit speed. Mode is always MODE_0 and data is always MSB
         *              first
         *
         * @param[out]  buffer          Address to store data
         * @param[in]   numberOfBytes   Number of bytes to receive
         */
        void shift_in_block_mode0_msb_first_fast (uint8_t *buffer, size_t numberOfBytes) const {
            __asm__ volatile (
#define ASMVAR(name) FC_ADDR(#name "%=", "SpiBlockReadStart%=")
            FC_START("SpiBlockReadStart%=", "SpiBlockReadEnd%=")
                    "       jmp #" FC_ADDR("outerLoop%=", "SpiBlockReadStart%=") "                              \n\t"

                    // Temporary variables
                    "bitIdx%=:                                                                                  \n\t"
                    "       nop                                                                                 \n\t"
                    "data%=:                                                                                    \n\t"
                    "       nop                                                                                 \n\t"


                    "outerLoop%=:                                                                               \n\t"
                    "       mov " ASMVAR(data) ", #0                                                            \n\t"
                    "       mov " ASMVAR(bitIdx) ", #8                                                          \n\t"

                    "loop%=:                                                                                    \n\t"
                    "       test %[_miso], ina wc                                                               \n\t"
                    "       xor outa, %[_sclk]                                                                  \n\t"
                    "       rcl " ASMVAR(data) ", #1                                                            \n\t"
                    "       xor outa, %[_sclk]                                                                  \n\t"
                    "       djnz " ASMVAR(bitIdx) ", #" FC_ADDR("loop%=", "SpiBlockReadStart%=") "              \n\t"

                    // Write the word back to the buffer in HUB memory
                    "       wrbyte " ASMVAR(data) ", %[_bufAdr]                                                 \n\t"
                    "       add %[_bufAdr], #1                                                                  \n\t"

                    "       djnz %[_numberOfBytes], #" FC_ADDR("outerLoop%=", "SpiBlockReadStart%=") "          \n\t"
                    FC_END("SpiBlockReadEnd%=")
            : [_bufAdr] "+r"(buffer),
            [_numberOfBytes] "+r"(numberOfBytes)
            :[_miso] "r"(this->m_miso.get_mask()),
            [_sclk] "r"(this->m_sclk.get_mask())
            );
#undef ASMVAR
        }

        virtual void put_char (const char c) {
            this->shift_out(8, (uint32_t) c);
        }

        virtual void puts (const char string[]) {
            char *sPtr = (char *) string;
            while (sPtr)
                this->put_char(*sPtr++);
        }

        virtual char get_char () {
            char c = 0;
            return c;
        }

        /**
         * @brief       Print an error string through the provided PropWare::Printer interface
         *
         * @param[in]   printer     Object used for printing error string
         * @param[in]   err         Error number used to determine error string
         */
        void print_error_str (const Printer &printer, const ErrorCode err) const {
            const char str[]       = "SPI Error ";
            const int  relativeErr = err - BEG_ERROR;

            switch (err) {
                case INVALID_FREQ:
                    printer << str << relativeErr << ": Frequency set too high";
                    break;
                default:
                    // Is the error an SPI error?
                    if (err > BEG_ERROR && err < (BEG_ERROR + END_ERROR))
                        printer << "Unknown SPI error " << relativeErr << '\n';
                    else
                        printer << "Unknown error " << err << '\n';
            }
        }

    protected:

        void shift_out_msb_first (uint32_t bits, uint32_t data) const {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
            unsigned int clock;
            __asm__ volatile (
            FC_START("SpiSendMsbFirstStart%=", "SpiSendMsbFirstEnd%=")
                    "       ror %[_data], %[_bitCount]                                                  \n\t"
                    "       mov %[_clock], %[_clkDelay]                                                 \n\t"
                    "       add %[_clock], CNT                                                          \n\t"

                    "loop%=:                                                                            \n\t"
                    "       rol %[_data], #1 wc                                                         \n\t"
                    "       muxc outa, %[_mosi]                                                         \n\t"
                    "       waitcnt %[_clock], %[_clkDelay]                                             \n\t"
                    "       xor outa, %[_sclk]                                                          \n\t"
                    "       waitcnt %[_clock], %[_clkDelay]                                             \n\t"
                    "       xor outa, %[_sclk]                                                          \n\t"
                    "       djnz %[_bitCount], #" FC_ADDR("loop%=", "SpiSendMsbFirstStart%=") "         \n\t"

                    "       or outa, %[_mosi]                                                           \n\t"
                    FC_END("SpiSendMsbFirstEnd%=")
            : [_bitCount] "+r"(bits),
            [_data] "+r"(data),
            [_clock] "+r"(clock)
            : [_mosi] "r"(this->m_mosi.get_mask()),
            [_sclk] "r"(this->m_sclk.get_mask()),
            [_clkDelay] "r"(this->m_clkDelay)
            );
#pragma GCC diagnostic pop
        }

        void shift_out_lsb_first (uint32_t bits, uint32_t data) const {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
            unsigned int clock;
            __asm__ volatile (
            FC_START("SpiSendLsbFirstStart%=", "SpiSendLsbFirstEnd%=")
                    // Local variable declaration
                    "       mov %[_clock], CNT                                                          \n\t"
                    "       add %[_clock], %[_clkDelay]                                                 \n\t"

                    "loop%=:                                                                            \n\t"
                    "       ror %[_data], #1 wc                 '' move LSB into carry                  \n\t"
                    "       muxc OUTA, %[_mosi]                                                         \n\t"
                    "       waitcnt %[_clock], %[_clkDelay]                                             \n\t"
                    "       xor OUTA, %[_sclk]                                                          \n\t"
                    "       waitcnt %[_clock], %[_clkDelay]                                             \n\t"
                    "       xor OUTA, %[_sclk]                                                          \n\t"
                    "       djnz %[_bitCount], #" FC_ADDR("loop%=", "SpiSendLsbFirstStart%=") "         \n\t"

                    "       or outa, %[_mosi]                                                           \n\t"
                    FC_END("SpiSendLsbFirstEnd%=")
            : [_bitCount] "+r"(bits),
            [_data] "+r"(data),
            [_clock] "+r"(clock)
            : [_mosi] "r"(this->m_mosi.get_mask()),
            [_sclk] "r"(this->m_sclk.get_mask()),
            [_clkDelay] "r"(this->m_clkDelay)
            );
#pragma GCC diagnostic pop
        }

        uint32_t shift_in_msb_phs0 (unsigned int bits) const {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
            unsigned int clock;
            unsigned int tempData;
            __asm__ volatile (
            FC_START("SpiReadMsbPhs0Start%=", "SpiReadMsbPhs0End%=")
                    "       ror %[_data], %[_bitCount]              '' move MSB into bit 31                 \n\t"
                    "       mov %[_clock], %[_clkDelay]                                                     \n\t"
                    "       add %[_clock], CNT                                                              \n\t"

                    "loop%=:                                                                                \n\t"
                    "       test %[_miso], ina wc                                                           \n\t"
                    "       waitcnt %[_clock], %[_clkDelay]                                                 \n\t"
                    "       xor outa, %[_sclk]                                                              \n\t"
                    "       rcl %[_data], #1                                                                \n\t"
                    "       waitcnt %[_clock], %[_clkDelay]                                                 \n\t"
                    "       xor outa, %[_sclk]                                                              \n\t"
                    "       djnz %[_bitCount], #" FC_ADDR("loop%=", "SpiReadMsbPhs0Start%=") "              \n\t"
                    FC_END("SpiReadMsbPhs0End%=")
            : [_bitCount] "+r"(bits),
            [_clock] "+r"(clock),
            [_data] "+r"(tempData)
            :[_miso] "r"(this->m_miso.get_mask()),
            [_sclk] "r"(this->m_sclk.get_mask()),
            [_clkDelay] "r"(this->m_clkDelay)
            );
#pragma GCC diagnostic pop
            return tempData;
        }

        uint32_t shift_in_lsb_phs0 (const unsigned int bits) const {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
            unsigned int clock;
            unsigned int tempData;
            unsigned int modifiableBits = bits;
            __asm__ volatile (
            FC_START("SpiReadLsbPhs0Start%=", "SpiReadLsbPhs0End%=")
                    "       ror %[_data], %[_bitCount]              '' move MSB into bit 31                 \n\t"
                    "       mov %[_clock], %[_clkDelay]                                                     \n\t"
                    "       add %[_clock], CNT                                                              \n\t"

                    "loop%=:                                                                                \n\t"
                    "       test %[_miso], ina wc                                                           \n\t"
                    "       waitcnt %[_clock], %[_clkDelay]                                                 \n\t"
                    "       xor outa, %[_sclk]                                                              \n\t"
                    "       rcr %[_data], #1                                                                \n\t"
                    "       waitcnt %[_clock], %[_clkDelay]                                                 \n\t"
                    "       xor outa, %[_sclk]                                                              \n\t"
                    "       djnz %[_bitCount], #" FC_ADDR("loop%=", "SpiReadLsbPhs0Start%=") "              \n\t"
                    FC_END("SpiReadLsbPhs0End%=")
            : [_bitCount] "+r"(modifiableBits),
            [_clock] "+r"(clock),
            [_data] "+r"(tempData)
            :[_miso] "r"(this->m_miso.get_mask()),
            [_sclk] "r"(this->m_sclk.get_mask()),
            [_clkDelay] "r"(this->m_clkDelay)
            );
#pragma GCC diagnostic pop
            return tempData;
        }

        uint32_t shift_in_msb_phs1 (unsigned int bits) const {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
            unsigned int clock;
            unsigned int tempData;
            __asm__ volatile (
            FC_START("SpiReadMsbPhs1Start%=", "SpiReadMsbPhs1End%=")
                    "       ror %[_data], %[_bitCount]              '' move MSB into bit 31                 \n\t"
                    "       mov %[_clock], %[_clkDelay]                                                     \n\t"
                    "       add %[_clock], CNT                                                              \n\t"

                    "loop%=:                                                                                \n\t"
                    "       xor outa, %[_sclk]                                                              \n\t"
                    "       waitcnt %[_clock], %[_clkDelay]                                                 \n\t"
                    "       test %[_miso], ina wc                                                           \n\t"
                    "       xor outa, %[_sclk]                                                              \n\t"
                    "       waitcnt %[_clock], %[_clkDelay]                                                 \n\t"
                    "       rcl %[_data], #1                                                                \n\t"
                    "       djnz %[_bitCount], #" FC_ADDR("loop%=", "SpiReadMsbPhs1Start%=") "              \n\t"
                    FC_END("SpiReadMsbPhs1End%=")
            : [_bitCount] "+r"(bits),
            [_clock] "+r"(clock),
            [_data] "+r"(tempData)
            :[_miso] "r"(this->m_miso.get_mask()),
            [_sclk] "r"(this->m_sclk.get_mask()),
            [_clkDelay] "r"(this->m_clkDelay)
            );
#pragma GCC diagnostic pop
            return tempData;
        }

        uint32_t shift_in_lsb_phs1 (unsigned int bits) const {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
            unsigned int clock;
            unsigned int tempData;
            __asm__ volatile (
            FC_START("SpiReadLsbPhs1Start%=", "SpiReadLsbPhs1End%=")
                    "       ror %[_data], %[_bitCount]              '' move MSB into bit 31                 \n\t"
                    "       mov %[_clock], %[_clkDelay]                                                     \n\t"
                    "       add %[_clock], CNT                                                              \n\t"

                    "loop%=:                                                                                \n\t"
                    "       xor outa, %[_sclk]                                                              \n\t"
                    "       waitcnt %[_clock], %[_clkDelay]                                                 \n\t"
                    "       test %[_miso], ina wc                                                           \n\t"
                    "       xor outa, %[_sclk]                                                              \n\t"
                    "       waitcnt %[_clock], %[_clkDelay]                                                 \n\t"
                    "       rcr %[_data], #1                                                                \n\t"
                    "       djnz %[_bitCount], #" FC_ADDR("loop%=", "SpiReadLsbPhs1Start%=") "              \n\t"
                    FC_END("SpiReadLsbPhs1End%=")
            : [_bitCount] "+r"(bits),
            [_clock] "+r"(clock),
            [_data] "+r"(tempData)
            :[_miso] "r"(this->m_miso.get_mask()),
            [_sclk] "r"(this->m_sclk.get_mask()),
            [_clkDelay] "r"(this->m_clkDelay)
            );
#pragma GCC diagnostic pop
            return tempData;
        }

    private:

        static void reset_pin_mask (Pin &pin, const Port::Mask mask) {
            pin.set_dir_in();
            pin.set_mask(mask);
            pin.set();
            pin.set_dir_out();
        }

    protected:
        PropWare::Pin m_mosi;
        PropWare::Pin m_miso;
        PropWare::Pin m_sclk;
        unsigned int  m_clkDelay;
        Mode          m_mode;
        BitMode       m_bitmode;
};

}
