/**
 * @file    PropWare/i2cbase.h
 *
 * @author  SRLM
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

#include <PropWare/pin.h>

#ifdef __PROPELLER_COG__
#error PropWare::I2CBase is not functional in the Propeller's COG mode! I'm very sorry!
// TODO: Why doesn't it work? It should. I put guards in place... but they don't seem to help. O-scope time I think
#endif

namespace PropWare {

/**
 * @brief   Low level I2C driver. Only does the most basic functions that all I2C devices implement.
 *
 * Requires that the SDA and SCL pins have sufficient pullups. These should be selected based on the capacitance of the
 * devices on the I2C bus, and the expected clock speed (400kHz currently).
 *
 * @author  SRLM
 * @author  David Zemon
 */
class I2CBase {
    public:
        static const Pin::Mask    DEFAULT_SCL_MASK  = Pin::P28;
        static const Pin::Mask    DEFAULT_SDA_MASK  = Pin::P29;
        static const unsigned int DEFAULT_FREQUENCY = 400000;

    public:
        /**
         * @brief   Set the IO Pins to float high; Does not require a cog
         *
         * @param[in]   sclMask     SCL pin mask; Defaults to the Propeller default SCL pin.
         * @param[in]   sdaMask     SDA pin mask; Defaults to the Propeller default SDA pin.
         * @param[in]   frequency   Frequency in Hz to run the bus
         */
        I2CBase (const Pin::Mask sclMask = DEFAULT_SCL_MASK, const Pin::Mask sdaMask = DEFAULT_SDA_MASK,
                 const unsigned int frequency = DEFAULT_FREQUENCY) {
            this->m_scl.set_mask(sclMask);
            this->m_sda.set_mask(sdaMask);

            this->set_frequency(frequency);

            //Set pins to input
            this->m_scl.set_dir_in();
            this->m_sda.set_dir_in();

            //Set outputs low
            this->m_scl.clear();
            this->m_sda.clear();
        }

        /**
         * @brief       Set the bus frequency
         *
         * @param[in]   frequency   Frequency in Hz to run the bus
         */
        void set_frequency (const unsigned int frequency) {
            this->m_clockDelay = CLKFREQ / (frequency << 1);
        }

        /**
         * @brief   Output a start condition on the I2C bus
         */
        void start () const {
            //Set pins as output
            this->m_scl.set_dir_out();
            this->m_sda.set_dir_out();

            this->m_scl.set();
            this->m_sda.set();
#ifndef __PROPELLER_CMM_
            __asm__ volatile("nop");
#endif
            this->m_sda.toggle();
            this->m_scl.toggle();
        }

        /**
         * @brief   Output a stop condition on the I2C bus
         */
        void stop () const {
            //Set pins to input
            this->m_scl.set_dir_in();
            this->m_sda.set_dir_in();
        }

        /**
         * @brief   Output a byte on the I2C bus.
         *
         * @param[in]   byte    8 bits to send on the bus
         *
         * @return      true if the device acknowledges, false otherwise
         */
        bool send_byte (const uint8_t byte) const {
            int result   = 0;
            int datamask = 0;
            int nextCNT  = 0;
            int temp     = 0;

            __asm__ volatile(
#ifndef __PROPELLER_COG__
            "         fcache #(PutByteEnd - PutByteStart)\n\t"
                    "         .compress off                  \n\t"
                    "PutByteStart: "
#endif
                    /* Setup for transmit loop */
                    "         mov %[datamask], #256          \n\t" /* 0x100 */
                    "         mov %[result],   #0            \n\t"
                    "         mov %[nextCNT],  cnt           \n\t"
                    "         add %[nextCNT],  %[clockDelay] \n\t"

                    /* Transmit Loop (8x) */
                    //Output bit of byte
                    "PutByteLoop%=: "
                    "         shr  %[datamask], #1                \n\t" // Set up mask
                    "         and  %[datamask], %[databyte] wz,nr \n\t" // Move the bit into Z flag
                    "         muxz dira,        %[SDAMask]        \n\t"

                    //Pulse clock
                    "         waitcnt %[nextCNT], %[clockDelay] \n\t"
                    "         andn    dira,       %[SCLMask]    \n\t" // Set SCL high
                    "         waitcnt %[nextCNT], %[clockDelay] \n\t"
                    "         or      dira,       %[SCLMask]    \n\t" // Set SCL low

                    //Return for more bits
#ifdef __PROPELLER_COG__
                    "         djnz %[datamask], #PutByteLoop%= nr \n\t"
#else
                    "         djnz %[datamask], #__LMM_FCACHE_START+(PutByteLoop%=-PutByteStart) nr \n\t"
#endif

                    // Get ACK
                    "         andn    dira,       %[SDAMask]    \n\t" // Float SDA high (release SDA)
                    "         waitcnt %[nextCNT], %[clockDelay] \n\t"
                    "         andn    dira,       %[SCLMask]    \n\t" // SCL high (by float)
                    "         waitcnt %[nextCNT], %[clockDelay] \n\t"
                    "         mov     %[temp],    ina           \n\t" //Sample input
                    "         and     %[SDAMask], %[temp] wz,nr \n\t" // If != 0, ack'd, else nack
                    "         muxz    %[result],  #1            \n\t" // Set result to equal to Z flag (aka, 1 if ack'd)
                    "         or      dira,       %[SCLMask]    \n\t" // Set scl low
                    "         or      dira,       %[SDAMask]    \n\t" // Set sda low
#ifndef __PROPELLER_COG__
                    "         jmp     __LMM_RET                 \n\t"
                    "PutByteEnd: "
                    "         .compress default                 \n\t"
#endif
            : // Outputs
            [datamask] "=&r"(datamask),
            [result] "=&r"(result),
            [nextCNT] "=&r"(nextCNT),
            [temp] "=&r"(temp)
            : // Inputs
            [SDAMask] "r"(this->m_sda.get_mask()),
            [SCLMask] "r"(this->m_scl.get_mask()),
            [databyte] "r"(byte),
            [clockDelay] "r"(m_clockDelay));

            return (bool) result;
        }

        /**
         * @brief       Get a byte from the bus
         *
         * @param[in]   acknowledge     true to acknowledge the byte received, false otherwise
         *
         * @return      Byte clocked in from the bus
         */
        uint8_t read_byte (const bool acknowledge) const {
            int result = 0;
            int datamask, nextCNT, temp;

            __asm__ volatile(
#ifndef __PROPELLER_COG__
            "         fcache #(GetByteEnd - GetByteStart)\n\t"
                    "         .compress off                   \n\t"
                    "GetByteStart: "
#endif
                    // Setup for receive loop
                    "         andn dira,        %[SDAMask]    \n\t"
                    "         mov  %[datamask], #256          \n\t" /* 0x100 */
                    "         mov  %[result],   #0            \n\t"
                    "         mov  %[nextCNT],  cnt           \n\t"
                    "         add  %[nextCNT],  %[clockDelay] \n\t"

                    // Recieve Loop (8x)
                    //Get bit of byte
                    "GetByteLoop%=: "

                    "         waitcnt %[nextCNT],  %[clockDelay] \n\t"
                    "         shr     %[datamask], #1            \n\t" // Set up mask

                    //Pulse clock
                    "         andn    dira,       %[SCLMask]       \n\t" // Set SCL high
                    "         waitcnt %[nextCNT], %[clockDelay]    \n\t"
                    "         mov     %[temp],    ina              \n\t" //Sample the input
                    "         and     %[temp],    %[SDAMask] nr,wz \n\t"
                    "         muxnz   %[result],  %[datamask]      \n\t"
                    "         or      dira,       %[SCLMask]       \n\t" // Set SCL low

                    //Return for more bits
#ifdef __PROPELLER_COG__
                    "         djnz %[datamask], #GetByteLoop%= nr    \n\t"
#else
                    "         djnz %[datamask], #__LMM_FCACHE_START+(GetByteLoop%=-GetByteStart) nr \n\t"
#endif

                    // Put ACK

                    "         and     %[acknowledge], #1 nr,wz  \n\t" //Output ACK

                    "         muxnz   dira,       %[SDAMask]    \n\t"
                    "         waitcnt %[nextCNT], %[clockDelay] \n\t"
                    "         andn    dira,       %[SCLMask]    \n\t" // SCL high (by float)
                    "         waitcnt %[nextCNT], %[clockDelay] \n\t"

                    "         or   dira, %[SCLMask]       \n\t" // Set scl low
                    "         or   dira, %[SDAMask]       \n\t" // Set sda low
#ifndef __PROPELLER_COG__
                    "         jmp  __LMM_RET              \n\t"
                    "GetByteEnd: "
                    "         .compress default           \n\t"
#endif
            : // Outputs
            [datamask] "=&r"(datamask),
            [result] "=&r"(result),
            [temp] "=&r"(temp),
            [nextCNT] "=&r"(nextCNT)

            : // Inputs
            [SDAMask] "r"(this->m_sda.get_mask()),
            [SCLMask] "r"(this->m_scl.get_mask()),
            [acknowledge] "r"(acknowledge),
            [clockDelay] "r"(m_clockDelay));

            return (uint8_t) result;

        }

    private:
        Pin          m_scl;
        Pin          m_sda;
        unsigned int m_clockDelay;
};

}
