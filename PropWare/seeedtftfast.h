/**
* @file        seeedtftfast.h
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

#ifndef SEEEDTFTFAST_H_
#define SEEEDTFTFAST_H_

#include <PropWare/seeedtft.h>

namespace PropWare {
    // Symbol for assembly instructions to start a new SPI cog
    extern "C" {
    extern uint32_t _SeeedTftStartCog (void *arg);
    }

    class SeeedTFTFast : public PropWare::SeeedTFT {
    public:
        virtual void start (const PropWare::Pin::Mask lsbDataPin,
                const PropWare::Port::Mask csMask,
                const PropWare::Port::Mask rdMask,
                const PropWare::Port::Mask wrMask,
                const PropWare::Port::Mask rsMask) {
            this->m_cog = _SeeedTftStartCog(
                    (void *) &mailbox);

            SeeedTFT::start(lsbDataPin, csMask, rdMask, wrMask, rsMask);
        }

    protected:
        virtual void sendCommand (const uint_fast8_t index) const {
            mailbox = index << 8 + pmb_sendCMD;
        }

        virtual void sendData (const uint_fast16_t data) const {
            mailbox = data << 8 + pmb_sendDATA;
        }

    protected:
        typedef enum {
            pmb_idle,
            pmb_sendCMD,
            pmb_sendDATA,
            pmb_repeat
        } SeeedTftAsmFunc;

        int8_t m_cog;

        static atomic_t mailbox;
    };
}

#endif /* SEEEDTFTFAST_H_ */