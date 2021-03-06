/**
 * @file        PropWare/serial/uart/uartrx.h
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

#include <PropWare/hmi/input/scancapable.h>
#include <PropWare/serial/uart/uart.h>
#include <PropWare/gpio/pin.h>

namespace PropWare {

#ifdef __PROPELLER_COG__
#define virtual
#endif

/**
 * @brief   Receive routines for basic UART communication
 */
class UARTRX : public UART
#ifndef __PROPELLER_COG__
        , public ScanCapable
#endif
{
    public:
        /**
         * @see PropWare::UART::UART()
         */
        UARTRX () {
            // Can't rely on parent constructor because the setters are virtual
            this->set_data_width(UART::DEFAULT_DATA_WIDTH);
            this->set_parity(UART::DEFAULT_PARITY);
            this->set_stop_bit_width(UART::DEFAULT_STOP_BIT_WIDTH);
            this->set_baud_rate(_cfg_baudrate);
            this->set_rx_mask((Port::Mask) (1 << _cfg_rxpin));
        }

        /**
         * @brief       Initialize a UART receiver
         *
         * @param[in]   rx  Pin mask for RX (receive) pin
         */
        UARTRX (const Port::Mask rx) {
            // Can't rely on parent constructor because the setters are virtual
            this->set_data_width(UART::DEFAULT_DATA_WIDTH);
            this->set_parity(UART::DEFAULT_PARITY);
            this->set_stop_bit_width(UART::DEFAULT_STOP_BIT_WIDTH);
            this->set_baud_rate(_cfg_baudrate);
            this->set_rx_mask(rx);
        }

        void set_rx_mask (const Port::Mask rx) {
            this->m_pin.set_mask(rx);
            this->m_pin.set_dir_in();
            this->m_pin.set();
        }

        Port::Mask get_rx_mask () const {
            return this->m_pin.get_mask();
        }

        virtual ErrorCode set_data_width (const uint8_t dataWidth) {
            ErrorCode err;
            check_errors(UART::set_data_width(dataWidth));

            this->set_msb_mask();
            this->set_receivable_bits();

            return NO_ERROR;
        }

        virtual void set_parity (const UART::Parity parity) {
            UART::set_parity(parity);
            this->set_msb_mask();
            this->set_receivable_bits();
        }

        /**
         * @brief   Retrieve a single word from the bus
         *
         * @return  Either the word is returned from the bus, or -1 if a parity error occurs.
         */
        uint32_t receive () const {
            uint32_t rxVal = this->shift_in_data(this->m_receivableBits, this->m_bitCycles, this->m_pin.get_mask(),
                                                 this->m_msbMask);

            if (static_cast<bool>(this->m_parity) && this->check_parity(rxVal))
                return static_cast<uint32_t>(-1);
            else
                return rxVal & this->m_dataMask;
        }

        /**
         * @brief       Receive one byte from the uart. This blocks until the byte is received.
         *
         * @param[out]  data    The byte that was read.
         *
         * @return      An ErrorCode that specifies if something went wrong, and what.
         */
        PropWare::ErrorCode receive (uint32_t &data) const {
            const uint32_t rxVal = this->shift_in_data(this->m_receivableBits, this->m_bitCycles,
                                                       this->m_pin.get_mask(), this->m_msbMask);

            if (static_cast<bool>(this->m_parity) && this->check_parity(rxVal))
                return PARITY_ERROR;
            else {
                data = rxVal & this->m_dataMask;
                return NO_ERROR;
            }
        }

        /**
         * @brief       Receive one byte from the uart. This blocks until either the byte was received or the timeout
         *              triggered.
         *
         * @param[out]  data        The byte that was read.
         * @param[in]   timeout     Timeout after which the function will exit without having received a byte.
         *
         * @return      An ErrorCode that specifies if something went wrong, and what.
         *
         * @warning     If this method is used for multiple consecutive bytes, the baud rate should be no greater than
         *              56000. If this method is used only for the first byte of a multi-byte transmission,
         *              then the standard maximum baud rate is acceptable.
         */
        PropWare::ErrorCode receive (uint32_t &data, const uint32_t timeout) const {
            const uint32_t rxVal = this->shift_in_data(this->m_receivableBits, this->m_bitCycles,
                                                       this->m_pin.get_mask(), this->m_msbMask, timeout);
            if (static_cast<uint32_t>(-1) == rxVal)
                return TIMEOUT_ERROR;
            else if (static_cast<bool>(this->m_parity) && this->check_parity(rxVal))
                return PARITY_ERROR;
            else {
                data = rxVal & this->m_dataMask;
                return NO_ERROR;
            }
        }

        /**
         * @brief       Read bytes until the provided delimiter is read or max length is reached.
         *
         * If the delimiter is read before hitting the max length, then the delimiter will be inserted into the buffer
         * and the function will return. At the point that the max length is hit, the function will immediately return.
         *
         * @post        The input buffer will contain data read in and the length parameter will contain the number of
         *              characters received.
         *
         * @param[out]  buffer*     Area in memory will data can be stored
         * @param[in]   length*     Maximum number of characters to read from the bus. Upon returning, the number of
         *                          characters actually received will be stored here.
         * @param[in]   delimiter   Read from the bus until this character is received.
         *
         * @return      0 upon success, error code otherwise.
         */
        PropWare::ErrorCode get_line (char *buffer, int32_t *length, const char delimiter = '\n') const {
            if (NULL == length)
                return NULL_POINTER;
            else if (0 == *length)
                *length     = INT32_MAX;
            int32_t wordCnt = 0;

            // Check if the total receivable bits can fit within a byte
            if (8 >= this->m_receivableBits) {
                // Set RX as input
                __asm__ volatile ("andn dira, %0" : : "r" (this->m_pin.get_mask()));

                *length = this->shift_in_byte_array((uint32_t) buffer, *length, delimiter);

                if (Parity::NO_PARITY != this->m_parity)
                    for (int32_t i = 0; i < *length; --i)
                        if (this->check_parity(static_cast<uint32_t>(buffer[i])))
                            return UART::PARITY_ERROR;
            }
                // If total receivable bits does not fit within a byte, shift in one word at a time (this offers no speed
                // improvement - it is only here for user convenience)
            else {
                uint32_t rxVal;

                do {
                    rxVal = this->receive();
                    if (static_cast<uint32_t>(-1) == rxVal)
                        return UART::PARITY_ERROR;
                    else {
                        *buffer = (char) rxVal;
                        ++buffer;
                        ++wordCnt;
                    }
                } while (rxVal != delimiter && wordCnt < *length);

                *length = wordCnt;
            }

            return NO_ERROR;
        }

        /**
         * @brief       Reads multiple bytes into the given buffer. This blocks until requested number of words have
         *              been received
         *
         * @param[in]   buffer*     Area in memory where received data can be stored
         * @param[in]   length      Number of bytes to read
         *
         * @return      An ErrorCode that specifies if something went wrong, and what.
         */
        PropWare::ErrorCode receive_array (uint8_t *buffer, uint32_t length) const {
            PropWare::ErrorCode err;

            // Check if the total receivable bits can fit within a byte
            if (8 >= this->m_receivableBits) {
                // Set RX as input
                __asm__ volatile ("andn dira, %0" : : "r" (this->m_pin.get_mask()));

                this->shift_in_byte_array(buffer, length);

                if (Parity::NO_PARITY != this->m_parity)
                    for (uint32_t i = 0; i < length; --i)
                        check_errors(this->check_parity(buffer[i]));
            }
            // If total receivable bits does not fit within a byte, shift in one word at a time (this offers no speed
            // improvement - it is only here for user convenience)
            else {
                uint32_t      temp;
                for (uint32_t i = 0; i < length; ++i) {
                    if (static_cast<uint32_t>(-1) == (temp = this->receive()))
                        return UART::PARITY_ERROR;
                    else
                        buffer[i] = (char) temp;
                }
            }

            return NO_ERROR;
        }

        /**
         * @brief      Reads multiple bytes into the given buffer. This blocks until either the
         *
         * @param[in]   buffer*     Area in memory where received data can be stored
         * @param[in]   length      Number of bytes to read
         * @param[in]   timeout     Amount of clock cycles after which the function will stop waiting for a new byte
         *                          to start. This timeout is not for all bytes read here.
         *
         * @return      An ErrorCode that specifies if something went wrong, and what.
         */
        PropWare::ErrorCode receive_array (uint8_t *buffer, uint32_t length, const uint32_t timeout) const {
            PropWare::ErrorCode err;

            // Check if the total receivable bits can fit within a byte
            if (8 >= this->m_receivableBits) {
                // Set RX as input
                __asm__ volatile ("andn dira, %0" : : "r" (this->m_pin.get_mask()));

                if (!this->shift_in_byte_array(buffer, length, timeout))
                    return TIMEOUT_ERROR;
                else if (Parity::NO_PARITY != this->m_parity)
                    for (uint32_t i = 0; i < length; --i)
                        check_errors(this->check_parity(buffer[i]));
            }
            // If total receivable bits does not fit within a byte, shift in one word at a time (this offers no speed
            // improvement - it is only here for user convenience)
            else {
                uint32_t      rxVal;
                for (uint32_t i = 0; i < length; ++i) {
                    check_errors(this->receive(rxVal, timeout));
                    buffer[i] = static_cast<uint8_t>(rxVal);
                }
            }

            return NO_ERROR;
        }

        /**
         * @brief   Read words from the bus until a newline character (`\\n`) is received or the buffer is filled
         *
         * If found, the newline character will be replaced with a null-terminator. If the buffer is filled before a
         * newline is found, no null-terminator will be inserted
         *
         * @param[in]   string[]        Output buffer which should store the data
         * @param[out]  bufferSize*     Address where the new length of the buffer will be written
         *
         * @returns     Zero upon success, error code otherwise
         */
        PropWare::ErrorCode fgets (char string[], int32_t *bufferSize) const {
            const int32_t originalBufferSize = *bufferSize;

            PropWare::ErrorCode err;
            check_errors(this->get_line(string, bufferSize));

            // Replace delimiter with null-terminator IFF we found one
            if (*bufferSize != originalBufferSize || '\n' == string[originalBufferSize])
                string[*bufferSize - 1] = '\0';
            return NO_ERROR;
        }

        virtual char get_char () {
            return (char) this->receive();
        }

    protected:
        /**
         * @brief   Set a bit-mask for the data word's MSB (assuming LSB is bit
         *          0 - the start bit is not taken into account)
         */
        void set_msb_mask () {
            if (static_cast<bool>(this->m_parity))
                this->m_msbMask = (Port::Mask) (1 << this->m_dataWidth);
            else
                this->m_msbMask = (Port::Mask) (1 << (this->m_dataWidth - 1));
        }

        /**
         * @brief   Set the number of receivable bits - based on data width and
         *          parity selection
         */
        void set_receivable_bits () {
            if (static_cast<bool>(this->m_parity))
                this->m_receivableBits = (uint8_t) (this->m_dataWidth + 1);
            else
                this->m_receivableBits = this->m_dataWidth;
        }

        /**
         * Shift in one word of data (FCache function)
         */
        uint32_t shift_in_data (uint_fast8_t bits, const uint32_t bitCycles, const register uint32_t rxMask,
                                const uint32_t msbMask) const {
            volatile uint32_t data       = 0;
            volatile uint32_t waitCycles = bitCycles;

#ifndef DOXYGEN_IGNORE
            __asm__ volatile (
            FC_START("ShiftInDataStart%=", "ShiftInDataEnd%=")
                    "       shr %[_waitCycles], #1                                          \n\t"
                    "       add %[_waitCycles], %[_bitCycles]                               \n\t"
                    "       waitpne %[_rxMask], %[_rxMask]                                  \n\t"
                    "       add %[_waitCycles], CNT                                         \n\t"

                    // Receive a word
                    "loop%=:                                                                \n\t"
                    "       waitcnt %[_waitCycles], %[_bitCycles]                           \n\t"
                    "       shr %[_data],# 1                                                \n\t"
                    "       test %[_rxMask],ina wz                                          \n\t"
                    "       muxnz %[_data], %[_msbMask]                                     \n\t"
                    "       djnz %[_bits], #" FC_ADDR("loop%=", "ShiftInDataStart%=") "     \n\t"

                    // Wait for a stop bit
                    "       waitpeq %[_rxMask], %[_rxMask]                                  \n\t"
                    FC_END("ShiftInDataEnd%=")
            :// Outputs
            [_data] "+r"(data),
            [_waitCycles] "+r"(waitCycles),
            [_bits] "+r"(bits)
            :// Inputs
            [_rxMask] "r"(rxMask),
            [_msbMask] "r"(msbMask),
            [_bitCycles] "r"(bitCycles));
#endif

            return data;
        }

        /**
         * Shift in one word of data (FCache function)
         */
        uint32_t shift_in_data (uint_fast8_t bits, const uint32_t bitCycles, const uint32_t rxMask,
                                const uint32_t msbMask, uint32_t timeoutCycles) const {
            volatile uint32_t data       = -1;
            volatile uint32_t waitCycles = bitCycles;

            timeoutCycles /= 8; //the instruction that decrements the timeoutCounter needs 4 clock cycles

#ifndef DOXYGEN_IGNORE
            __asm__ volatile (
            FC_START("ShiftInDataStart%=", "ShiftInDataEnd%=")
                    "       shr %[_waitCycles], #1                                                  \n\t"
                    "       add %[_waitCycles], %[_bitCycles]                                       \n\t"

                    "awaitStart%=: "
                    "       test %[_rxMask],   ina                                 wz               \n\t"
                    " if_nz djnz %[_timeout],  #" FC_ADDR("awaitStart%=", "ShiftInDataStart%=") "   \n\t"
                    "       add  %[_waitCycles], CNT                                                \n\t"
                    " if_nz  jmp #" FC_ADDR("end%=", "ShiftInDataStart%=") "                        \n\t" //timed out

                    // Receive a word
                    "loop%=:                                                                        \n\t"
                    "       waitcnt %[_waitCycles], %[_bitCycles]                                   \n\t"
                    "       shr %[_data],# 1                                                        \n\t"
                    "       test %[_rxMask],ina wz                                                  \n\t"
                    "       muxnz %[_data], %[_msbMask]                                             \n\t"
                    "       djnz %[_bits], #" FC_ADDR("loop%=", "ShiftInDataStart%=") "             \n\t"

                    // Wait for a stop bit
                    "       waitpeq %[_rxMask], %[_rxMask]                                          \n\t"
                    "end%=: "
                    FC_END("ShiftInDataEnd%=")
            :// Outputs
            [_data] "+r"(data),
            [_waitCycles] "+r"(waitCycles),
            [_bits] "+r"(bits),
            [_timeout] "+r"(timeoutCycles)
            :// Inputs
            [_rxMask] "r"(rxMask),
            [_msbMask] "r"(msbMask),
            [_bitCycles] "r"(bitCycles));
#endif

            return data;
        }

        /**
         * @brief       Shift in an array of data (FCache function)
         *
         * @param[in]   bufferAddr
         * @param[in]   maxLength
         * @param[in]   delimiter
         */
        int shift_in_byte_array (uint32_t bufferAddr, const int maxLength, const char delimiter) const {
            volatile uint32_t data           = 0;
            volatile int      wordCnt        = 0;
            volatile uint32_t bitIdx         = 0;
            volatile uint32_t waitCycles     = 0;
            volatile uint32_t initWaitCycles = (this->m_bitCycles >> 1) + this->m_bitCycles;

#ifndef DOXYGEN_IGNORE
            // Initialize variables
            __asm__ volatile (
            FC_START("ShiftInStringStart%=", "ShiftInStringEnd%=")
                    "outerLoop%=:                                                                       \n\t"
                    // Initialize the index variable
                    "       mov %[_bitIdx], %[_bits]                                                    \n\t"
                    // Re-initialize the timer
                    "       mov %[_waitCycles], %[_initWaitCycles]                                      \n\t"
                    // Wait for the start bit
                    "       waitpne %[_rxMask], %[_rxMask]                                              \n\t"
                    // Begin the timer
                    "       add %[_waitCycles], CNT                                                     \n\t"

                    "innerLoop%=:                                                                       \n\t"
                    // Wait for the next bit
                    "       waitcnt %[_waitCycles], %[_bitCycles]                                       \n\t"
                    "       shr %[_data], #1                                                            \n\t"
                    "       test %[_rxMask], ina wz                                                     \n\t"
                    "       muxnz %[_data], %[_msbMask]                                                 \n\t"
                    "       djnz %[_bitIdx], #" FC_ADDR("innerLoop%=", "ShiftInStringStart%=") "        \n\t"

                    // Write the word back to the buffer in HUB memory
                    "       wrbyte %[_data], %[_bufAdr]                                                 \n\t"

                    // Check if we hit the delimiter (store result in Z)
                    "       and %[_data], #0xff                                                         \n\t"
                    "       cmp %[_data], %[_delim] wz                                                  \n\t"

                    // Clear the data register
                    "       mov %[_data], #0                                                            \n\t"
                    // Increment the buffer address and total word count
                    "       add %[_wordCnt], #1                                                         \n\t"
                    "       add %[_bufAdr], #1                                                          \n\t"

                    // Check if we hit the end of the buffer (store result in C)
                    "       cmp %[_wordCnt], %[_maxLength] wc                                           \n\t"

                    // Wait for the stop bits
                    "       waitpeq %[_rxMask], %[_rxMask]                                              \n\t"

                    // Finally, loop to the beginning if the delimiter is not equal to our most recent word and
                    // the buffer is not about to overflow
                    "if_nz_and_c jmp #" FC_ADDR("outerLoop%=", "ShiftInStringStart%=") "                \n\t"
                    FC_END("ShiftInStringEnd%=")
            :// Outputs
            [_bitIdx] "+r"(bitIdx),
            [_waitCycles] "+r"(waitCycles),
            [_data] "+r"(data),
            [_bufAdr] "+r"(bufferAddr),
            [_wordCnt] "+r"(wordCnt)
            :// Inputs
            [_rxMask] "r"(this->m_pin.get_mask()),
            [_bits] "r"(this->m_receivableBits),
            [_bitCycles] "r"(this->m_bitCycles),
            [_initWaitCycles] "r"(initWaitCycles),
            [_msbMask] "r"(this->m_msbMask),
            [_delim] "r"(delimiter),
            [_maxLength] "r"(maxLength));
#endif

            return wordCnt;
        }

        /**
         * @brief       Shift in an array of data (FCache function)
         *
         * @param[out]  *buffer     Buffer where data can be stored
         * @param[in]   length      Number of bytes that should be read from the port
         */
        void shift_in_byte_array (uint8_t *buffer, unsigned int length) const {
            uint32_t initWaitCycles = (this->m_bitCycles >> 1) + this->m_bitCycles;

#ifndef DOXYGEN_IGNORE
            // Initialize variables
            __asm__ volatile (
#define ASMVAR(name) FC_ADDR(#name "%=", "ShiftInArrayDataStart%=")
            FC_START("ShiftInArrayDataStart%=", "ShiftInArrayDataEnd%=")
                    "       jmp #" FC_ADDR("outerLoop%=", "ShiftInArrayDataStart%=") "                          \n\t"

                    // Temporary variables
                    "bitIdx%=:                                                                                  \n\t"
                    "       nop                                                                                 \n\t"
                    "waitCycles%=:                                                                              \n\t"
                    "       nop                                                                                 \n\t"
                    "data%=:                                                                                    \n\t"
                    "       nop                                                                                 \n\t"
                    "       mov " ASMVAR(data) ", #0                                                            \n\t"

                    "outerLoop%=:                                                                               \n\t"
                    // Initialize the index variable
                    "       mov " ASMVAR(bitIdx) ", %[_bits]                                                    \n\t"
                    // Re-initialize the timer
                    "       mov " ASMVAR(waitCycles) ", %[_initWaitCycles]                                      \n\t"
                    // Wait for the start bit
                    "       waitpne %[_rxMask], %[_rxMask]                                                      \n\t"
                    // Begin the timer
                    "       add " ASMVAR(waitCycles) ", CNT                                                     \n\t"

                    "innerLoop%=:                                                                               \n\t"
                    // Wait for the next bit
                    "       waitcnt " ASMVAR(waitCycles) ", %[_bitCycles]                                       \n\t"
                    "       shr " ASMVAR(data) ", #1                                                            \n\t"
                    "       test %[_rxMask], ina wz                                                             \n\t"
                    "       muxnz " ASMVAR(data) ", %[_msbMask]                                                 \n\t"
                    "       djnz " ASMVAR(bitIdx) ", #" FC_ADDR("innerLoop%=", "ShiftInArrayDataStart%=") "     \n\t"

                    // Write the word back to the buffer in HUB memory
                    "       wrbyte " ASMVAR(data) ", %[_bufAdr]                                                 \n\t"

                    // Clear the data register
                    "       mov " ASMVAR(data) ", #0                                                            \n\t"
                    // Increment the buffer address
                    "       add %[_bufAdr], #1                                                                  \n\t"

                    // Wait for the stop bits and the loop back
                    "       waitpeq %[_rxMask], %[_rxMask]                                                      \n\t"
                    "       djnz %[_length], #" FC_ADDR("outerLoop%=", "ShiftInArrayDataStart%=") "             \n\t"
                    FC_END("ShiftInArrayDataEnd%=")
#undef ASMVAR
            :// Outputs
            [_bufAdr] "+r"(buffer),
            [_length] "+r"(length)
            :// Inputs
            [_rxMask] "r"(this->m_pin.get_mask()),
            [_bits] "r"(this->m_receivableBits),
            [_bitCycles] "r"(this->m_bitCycles),
            [_initWaitCycles] "r"(initWaitCycles),
            [_msbMask] "r"(this->m_msbMask));
#endif
        }


        /**
         * @brief       Shift in an array of data (FCache function)
         *
         * @param[out]  buffer*     Buffer where data can be stored
         * @param[in]   length      Number of bytes that should be read from the port
         * @param[in]   timeout     Amount of clock cycles after which the function will stop waiting
         *                          for a new byte to start. This timeout is not for all bytes read here.
         *
         * @return      True upon success; False upon timeout.
         */
        bool shift_in_byte_array (uint8_t *buffer, unsigned int length, uint32_t timeout) const {
            uint32_t          initWaitCycles = (this->m_bitCycles >> 1) + this->m_bitCycles;
            volatile uint32_t timeLeft       = 0;

            timeout /= 8; //the instruction that decrements the timeoutCounter needs 4 clock cycles

#ifndef DOXYGEN_IGNORE
            // Initialize variables
            __asm__ volatile (
#define ASMVAR(name) FC_ADDR(#name "%=", "ShiftInArrayDataStart%=")
            FC_START("ShiftInArrayDataStart%=", "ShiftInArrayDataEnd%=")
                    "       jmp #" FC_ADDR("outerLoop%=", "ShiftInArrayDataStart%=") "                          \n\t"

                    // Temporary variables
                    "bitIdx%=:                                                                                  \n\t"
                    "       nop                                                                                 \n\t"
                    "waitCycles%=:                                                                              \n\t"
                    "       nop                                                                                 \n\t"
                    "data%=:                                                                                    \n\t"
                    "       nop                                                                                 \n\t"
                    "       mov " ASMVAR(data) ", #0                                                            \n\t"

                    "outerLoop%=:                                                                               \n\t"
                    // Initialize the index variable
                    "       mov " ASMVAR(bitIdx) ", %[_bits]                                                    \n\t"
                    // Re-initialize the timer
                    "       mov " ASMVAR(waitCycles) ", %[_initWaitCycles]                                      \n\t"

                    //reset timeout and wait for startBit
                    "       mov    %[_timeLeft],   %[_timeout]                                                  \n\t"
                    "awaitStart%=: "
                    "       test   %[_rxMask],     ina  wz                                                      \n\t"
                    " if_nz djnz   %[_timeLeft],   #" FC_ADDR("awaitStart%=", "ShiftInArrayDataStart%=") "      \n\t"
                    // Begin the timer (even if we just timed out - for performance reasons)
                    "       add " ASMVAR(waitCycles) ", CNT                                                     \n\t"
                    //timed out, jump to the end (timeLeft is 0 now)
                    " if_nz  jmp   #" FC_ADDR("end%=", "ShiftInArrayDataStart%=") "                             \n\t"

                    "innerLoop%=:                                                                               \n\t"
                    // Wait for the next bit
                    "       waitcnt " ASMVAR(waitCycles) ", %[_bitCycles]                                       \n\t"
                    "       shr " ASMVAR(data) ", #1                                                            \n\t"
                    "       test %[_rxMask], ina wz                                                             \n\t"
                    "       muxnz " ASMVAR(data) ", %[_msbMask]                                                 \n\t"
                    "       djnz " ASMVAR(bitIdx) ", #" FC_ADDR("innerLoop%=", "ShiftInArrayDataStart%=") "     \n\t"

                    // Write the word back to the buffer in HUB memory
                    "       wrbyte " ASMVAR(data) ", %[_bufAdr]                                                 \n\t"

                    // Clear the data register
                    "       mov " ASMVAR(data) ", #0                                                            \n\t"
                    // Increment the buffer address
                    "       add %[_bufAdr], #1                                                                  \n\t"

                    // Wait for the stop bits and the loop back
                    "       waitpeq %[_rxMask], %[_rxMask]                                                      \n\t"
                    "       djnz %[_length], #" FC_ADDR("outerLoop%=", "ShiftInArrayDataStart%=") "             \n\t"

                    "end%=: "
                    FC_END("ShiftInArrayDataEnd%=")
#undef ASMVAR
            :// Outputs
            [_bufAdr] "+r"(buffer),
            [_length] "+r"(length),
            [_timeLeft] "+r"(timeLeft)
            :// Inputs
            [_rxMask] "r"(this->m_pin.get_mask()),
            [_bits] "r"(this->m_receivableBits),
            [_bitCycles] "r"(this->m_bitCycles),
            [_initWaitCycles] "r"(initWaitCycles),
            [_msbMask] "r"(this->m_msbMask),
            [_timeout] "r"(timeout));
#endif

            return 0 != timeLeft;
        }


        /**
         * @brief       Check parity for a received value
         *
         * @param[in]   rxVal   Received value with parity bit exactly as
         *                      received
         *
         * @return      0 for proper parity; error code otherwise
         */
        PropWare::ErrorCode check_parity (uint32_t rxVal) const {
            uint32_t evenParityResult;

            evenParityResult = 0;
            __asm__ volatile("test %[_data], %[_dataMask] wc \n\t"
                    "muxc %[_parityResult], %[_parityMask]"
            : [_parityResult] "+r"(evenParityResult)
            : [_data] "r"(rxVal),
            [_dataMask] "r"(this->m_dataMask),
            [_parityMask] "r"(this->m_parityMask));

            if (Parity::ODD_PARITY == this->m_parity) {
                //if using odd parity, the result should be different from evenParity
                if (evenParityResult == (rxVal & this->m_parityMask))
                    return UART::PARITY_ERROR;
            } else if (evenParityResult != (rxVal & this->m_parityMask))
                //if using even parity, the result should be that of evenParity
                return UART::PARITY_ERROR;

            return UART::NO_ERROR;
        }

    protected:
        Pin        m_pin;
        Port::Mask m_msbMask;
        uint8_t    m_receivableBits;
};

#ifdef __PROPELLER_COG__
#undef virtual
#endif

}
