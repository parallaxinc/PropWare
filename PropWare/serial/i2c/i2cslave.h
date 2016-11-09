/**
 * @file        PropWare/serial/i2c/i2cslave.h
 *
 * @author      Markus Ebner
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

#include <PropWare/gpio/pin.h>
#include <PropWare/concurrent/runnable.h>

namespace PropWare {

/**
 * @brief   Basic I2C slave driver
 *
 * Requires that the SDA and SCL pins have sufficient pull-ups. These should be selected based on the capacitance of the
 * devices on the I2C bus, and the expected clock speed.
 *
 * The driver supports restarts and only 7-bit addressing.
 * The driver does not support clock stretching.
 *
 * @warning    If the timeslot between start & restart, restart & restart, or stop and start is too small (depending on
 *             the master), a transmission might be completely lost, due to the onReceive callback taking too much time.
 */
template<class UserDataType>
class I2CSlave: public Runnable {
    public:
        static const Pin::Mask DEFAULT_SCL_MASK = Pin::Mask::P28;
        static const Pin::Mask DEFAULT_SDA_MASK = Pin::Mask::P29;
        typedef void(*I2CCallback) (I2CSlave<UserDataType> &, UserDataType);

    public:
        /**
         * @brief       Create an I2CSlave object (requires static allocation of buffer and stack)
         *
         * @param[in]   address         Address to join the bus as slave with
         * @param[in]   sclMask         Pin mask for the SCL pin
         * @param[in]   sdaMask         Pin mask for the SDA pin
         * @param[in]   buffer          Receive buffer to store messages as they arrive
         * @param[in]   stack           Reserved stack space that can be used for a new cog to execute the `run()` method
         * 
         * @param       <UserDataType>  Type of the userData that can be set and is then passed to all callback functions
         *
         * @warning     Providing a `buffer` that is too small will lead to received messages being truncated.
         */
        template<size_t BUFFER_SIZE, size_t STACK_SIZE>
        I2CSlave (const uint8_t address, uint8_t (&buffer)[BUFFER_SIZE], const uint32_t (&stack)[STACK_SIZE],
                  const Pin::Mask sclMask = DEFAULT_SCL_MASK, const Pin::Mask sdaMask = DEFAULT_SDA_MASK)
            : Runnable(stack),
              m_slaveAddress(address),
              m_scl(sclMask),
              m_sda(sdaMask),
              m_buffer(buffer),
              m_bufferUpperBound(BUFFER_SIZE - 1),
              m_bufferPtr(BUFFER_SIZE) {
        }

        /**
         * @brief       Create an I2C slave object (Allows dynamic allocation of buffer and stack)
         *
         * @param[in]   address     Address to join the bus as slave with
         * @param[in]   sclMask     Pin mask for the SCL pin
         * @param[in]   sdaMask     Pin mask for the SDA pin
         * @param[in]   buffer      Receive buffer to store messages as they arrive
         * @param[in]   bufferSize  Size of the receive buffer, that will hold a received message (-> maximal message
         *                          size)
         * @param[in]   stack       Reserved stack space that can be used for a new cog to execute the `run()` method
         * @param[in]   stackSize   Size of the reserved stack (in measured in 32-bit words, not bytes)
         *
         * @warning     Providing a `buffer` that is too small will lead to received messages being truncated.
         */
        I2CSlave (const uint8_t address, uint8_t *buffer, const size_t bufferSize, const uint32_t *stack,
                  const size_t stackSize, const Pin::Mask sclMask = DEFAULT_SCL_MASK,
                  const Pin::Mask sdaMask = DEFAULT_SDA_MASK)
            : Runnable(stack, stackSize),
              m_slaveAddress(address),
              m_scl(sclMask),
              m_sda(sdaMask),
              m_buffer(buffer),
              m_bufferUpperBound(bufferSize - 1),
              m_bufferPtr(bufferSize) {
        }

        /**
         * @brief       Give the I2CSlave a callback to notify the user about an incomming transmission
         *
         * @warning     If the execution of this delegates takes too long, data on the bus might be missed.
         */
        void set_on_receive (const I2CCallback onReceive) {
            this->m_onReceive = onReceive;
        }

        /**
         * @brief       Give the I2CSlave a callback to notify the user about an ancomming request
         *
         * @warning     This method should have the data to send on the bus prepared.
         *              Taking too long before transmit starts could mess the i2c state machines state up.
         */
        void set_on_request (const I2CCallback onRequest) {
            this->m_onRequest = onRequest;
        }

        /**
         * @brief       Set a custom user-data object stored internally and passed to all callbacks.
         *
         * @param[in]   userData   Userdata to store internally and pass to every callback.
         */
        void set_user_data (UserDataType userData) {
			this->m_userPtr = userData;
        }


        /**
         * @brief   Enter the loop that will watch and operate the bus.
         */
        void run () {
            this->m_scl.set_dir_in();
            this->m_sda.set_dir_in();
            this->m_scl.clear();
            this->m_sda.clear();

            const uint_fast8_t slaveAddress = this->m_slaveAddress;

            while (true) { // start loop
                this->await_start();
                while (true) { //restart loop
                    const uint_fast8_t address = this->read_address();
                    if ((address >> 1) == slaveAddress) { // Master is talking to us
                        this->send_ack(); // Tell master we are there
                        if (address & BIT_0) { // Master wants us to speak
                            this->m_requestEnded = false;
                            this->m_onRequest(*this, this->m_userData);
                            break;
                        } else { // Master wants us to listen
                            bool restart = this->read_to_end();
                            this->m_onReceive(*this, this->m_userData);
                            this->reset_receive_buffer(); // Throw away bytes that the user did not fetch
                            if (!restart)
                                break; // received stop, go back to outer loop and await new start
                        }
                    } else // Master is talking to another slave
                        //The next thing that interests us now is the next start -> go to await_start();
                        break;
                }
            }
        }

        /**
         * @brief   Get the amount of bytes in the receive buffer
         *
         * @return  The amount of bytes in the receive buffer
         */
        uint32_t available () const {
            return this->m_bufferUpperBound - this->m_bufferPtr + 1;
        }

        /**
         * @brief   Read the next byte from the receiveBuffer
         *
         * @return  The next byte from the receiveBuffer, `-1` when none is available.
         */
        int read () {
            if (this->m_bufferPtr <= this->m_bufferUpperBound)
                return this->m_buffer[this->m_bufferPtr++];
            else
                return -1;
        }


        /**
         * @brief       Send the given byte of data on the bus during a request from the bus master.
         *
         * @param[in]   data    Byte to send to the requesting master.
         *
         * @warning     Calling this method too late may result in a defective state of the i2c state machine.
         */
        void write (const uint8_t data) {
            if (!this->m_requestEnded) {
                uint32_t dataMask; //Initialized by setting BIT_7 in asm below

                __asm__ volatile(
                    "       mov            %[_dataMask],       #128                   \n\t" // Initialize the mask that specifies the bit from the byte to send
                    "       waitpne        %[_SCLMask],        %[_SCLMask]            \n\t" // Wait for the clock to be low first
                    "       or             dira,               %[_SDAMask]            \n\t" // Take SDA >after< clock is low (master has sda, since he is sending an ACK)

                    "loop%=: "
                    "       test           %[_data],           %[_dataMask]     wz    \n\t" // Test whether bit to send is 0 or 1
                    "       muxnz          outa,               %[_SDAMask]            \n\t" // Set the bit on the bus while the clock is low
                    "       waitpeq        %[_SCLMask],        %[_SCLMask]            \n\t" // Wait for the next clock cycle to start
                    "       shr            %[_dataMask],       #1               wz    \n\t" // Shift the mask one down to select the next lower bit
                    "       waitpne        %[_SCLMask],        %[_SCLMask]            \n\t" // Wait for the clock cycle to end
                    "if_nz  brs            #loop%=                                    \n\t" // Continue until dataMask is 0 (no bit left)

                    //wait for ack
                    "       andn           dira,               %[_SDAMask]            \n\t" // Set SDA to input, because master has to pull it down
                    "       waitpeq        %[_SCLMask],        %[_SCLMask]            \n\t" // Wait for the ACK-Clock to begin
                    "       test           %[_SDAMask],        ina              wz    \n\t" // Test whether master pulled SDA down or not
                    "if_z   mov            %[_requestEnded],   #0                     \n\t" // SDA low -> ACK
                    "if_nz  mov            %[_requestEnded],   #1                     \n\t" // SDA high -> NAK
                : // Output
                [_dataMask]     "+r" (dataMask),
                [_requestEnded] "+r" (this->m_requestEnded)
                : // Input
                [_SDAMask] "r" (this->m_sda.get_mask()),
                [_SCLMask] "r" (this->m_scl.get_mask()),
                [_data]    "r" (data));
            }
        }


    private:

        /**
         * @brief   Wait for a start / restart condition on the bus.
         */
        void await_start () const {
            __asm__ volatile(
                "loop%=: "
                "       waitpeq     %[_SDAMask],    %[_SDAMask]         \n\t" // Wait for sda to be high
                "       waitpne     %[_SDAMask],    %[_SDAMask]         \n\t" // Wait for sda to get low
                "       test        %[_SCLMask],    ina             wz  \n\t" // If scl was high while sda got low...
                "if_z   brs         #loop%=                             \n\t" // ... return, otherwise: start anew
            : // Output
            : // Input
            [_SDAMask] "r" (this->m_sda.get_mask()),
            [_SCLMask] "r" (this->m_scl.get_mask()));
        }

        /**
         * @brief   Read one byte from the bus without sending any response.
         */
        uint_fast8_t read_address () const {
            uint32_t result;
            uint32_t bitCounter;

            __asm__ volatile(
            FC_START("ReadAddressStart", "ReadAddressEnd")
                "        mov        %[_result],      #0                                              \n\t"
                "        mov        %[_bitCounter],  #8                                              \n\t"
                "nextBit%=: "
                "        waitpne    %[_SCLMask],     %[_SCLMask]                                     \n\t" // Wait for clock to get low (should already be low at this time)
                "        waitpeq    %[_SCLMask],     %[_SCLMask]                                     \n\t" // Wait for clock to get high
                "        test       %[_SDAMask],     ina          wc                                 \n\t" // Read bit from bus ...
                "        rcl        %[_result],      #1                                              \n\t" // ... and store in result
                "        djnz       %[_bitCounter],  #" FC_ADDR("nextBit%=", "ReadAddressStart") "    \n\t"
                FC_END("ReadAddressEnd")
                : [_result]     "+r" (result),
                  [_bitCounter] "+r" (bitCounter)
                : [_SDAMask] "r" (this->m_sda.get_mask()),
                  [_SCLMask] "r" (this->m_scl.get_mask())
            );
            return result;
        }

        /**
         * @brief   Wait for the next clock and pull the data line down to signal the master an ACK
         */
        inline __attribute__((always_inline)) void send_ack () const {
            //The code does not work anymore when removing inline and attribute always_inline. Why is this?
            __asm__ volatile(
                "       waitpne     %[_SCLMask],    %[_SCLMask]      \n\t" // Wait for SCL to be low first
                "       or          dira,           %[_SDAMask]      \n\t" // Take SDA and ...
                "       andn        outa,           %[_SDAMask]      \n\t" // ... pull it down
                "       waitpeq     %[_SCLMask],    %[_SCLMask]      \n\t" // Wait for SCL to go high ...
                "       waitpne     %[_SCLMask],    %[_SCLMask]      \n\t" // ... and wait for it to go low again
                "       andn        dira,           %[_SDAMask]      \n\t" // Let go of SDA again (high by float)
            : : // Inputs
            [_SDAMask] "r" (this->m_sda.get_mask()),
            [_SCLMask] "r" (this->m_scl.get_mask()));
        }

        /**
         * @brief   Read all bytes the master sends until either a restart or a stop condition is received
         *
         * @return  `true` if a restart condition was received, `false` if a stop condition was received
         */
        bool read_to_end () {
            uint32_t result;
            uint32_t bitCounter;
            uint32_t isRestart;

            while(true) {
                __asm__ volatile(
                    "       mov         %[_isRestart],       #2                 \n\t"
                    "       mov         %[_bitCounter],      #7                 \n\t"
                    "       mov         %[_result],          #0                 \n\t"
                    "       waitpne     %[_SCLMask],         %[_SCLMask]        \n\t" // Wait for scl to be low first
                    "       waitpeq     %[_SCLMask],         %[_SCLMask]        \n\t" // Wait for scl to go high
                    "       test        %[_SDAMask],         ina         wc     \n\t" // Read bit and...
                    "       rcl         %[_result],          #1                 \n\t" // ... store in result
                    "if_c   brs         #DetectRestart%=                        \n\t" // The first bit of a received byte may be b7, or a stop / restart
                    // If sda was high, it can only be a restart
                    "DetectStop%=: "
                    "       test        %[_SCLMask],         ina         wz     \n\t" // scl went low -> no chance for a stop-condition to be detected ...
                    "if_z   brs         #loop%=                                 \n\t" // ... continue receiving data bits
                    "       test        %[_SDAMask],         ina         wz     \n\t"
                    "if_nz  mov         %[_isRestart],       #0                 \n\t" // stop detected. Set isRestart to false ...
                    "if_nz  brs         #ReceiveEnd%=                           \n\t" // ... and exit
                    "       brs         #DetectStop%=                           \n\t"

                    "DetectRestart%=: "
                    "       test        %[_SCLMask],         ina         wz     \n\t" // scl went low -> no chance for a (re)start-condition to be detected ...
                    "if_z   brs         #loop%=                                 \n\t" // ... continue receiving data bits
                    "       test        %[_SDAMask],         ina         wz     \n\t"
                    "if_z   mov         %[_isRestart],       #1                 \n\t" // restart detected. Set isRestart to true...
                    "if_z   brs         #ReceiveEnd%=                           \n\t" // ... and exit
                    "       brs         #DetectRestart%=                        \n\t"

                    "loop%=: "                                                        // for(int i = 0; i < 8; ++i) {
                    "       waitpne     %[_SCLMask],         %[_SCLMask]        \n\t"        // Wait for ...
                    "       waitpeq     %[_SCLMask],         %[_SCLMask]        \n\t"        // ... next clock
                    "       test        %[_SDAMask],         ina         wc     \n\t"        // Read bit and...
                    "       rcl         %[_result],          #1                 \n\t"        // ... store in result
                    "       sub         %[_bitCounter],      #1          wz     \n\t"
                    "if_nz  brs         #loop%=                                 \n\t" // }

                    "ReceiveEnd%=: "
                : // Outputs
                [_result]     "+r" (result),
                [_bitCounter] "+r" (bitCounter),
                [_isRestart]  "+r" (isRestart)
                : // Inputs
                [_SDAMask] "r" (this->m_sda.get_mask()),
                [_SCLMask] "r" (this->m_scl.get_mask()));

                if (2 == isRestart) {
                    this->send_ack();
                    this->append_receive_buffer(static_cast<uint8_t>(result));
                } else {
                    return static_cast<bool>(isRestart);
                }
            }
        }



        /**
         * @brief   Add a byte to the receive buffer that the user can then later fetch from it in the onReceive handler.
         */
        void append_receive_buffer (const uint8_t data) {
            if (this->m_bufferPtr)
                this->m_buffer[--this->m_bufferPtr] = data;
        }

        /**
         * @brief   Reset the receiveBuffer's state for the next message. This throws away bytes that the user did not fetch in the handler.
         */
        void reset_receive_buffer () {
            this->m_bufferPtr = this->m_bufferUpperBound + 1;
        }

    private:
        const uint8_t  m_slaveAddress;
        const Pin      m_scl;
        const Pin      m_sda;

        /**
         * Buffer storing the received messages
         */
        uint8_t*       m_buffer;
        /**
         * receiveBufferSize - 1
         */
        uint32_t       m_bufferUpperBound;
        /**
         * Index always pointing to the last written element in the receiveBuffer (= receive_buffer_size when empty)
         */
        uint32_t       m_bufferPtr;
        I2CCallback    m_onReceive;

        bool           m_requestEnded;
        I2CCallback    m_onRequest;

        /**
         * Custom pointer that can be set and is then passed to all I2CCallbacks.
         */
        UserDataType   m_userData;
};

}
