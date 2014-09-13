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
        SeeedTFTFast() : PropWare::SeeedTFT() {
        };

        virtual void start (const PropWare::Pin::Mask lsbDataPin,
                const PropWare::Port::Mask csMask,
                const PropWare::Port::Mask rdMask,
                const PropWare::Port::Mask wrMask,
                const PropWare::Port::Mask rsMask) {
            this->m_cog = _SeeedTftStartCog(
                    (void *) &mailbox);

            SeeedTFT::start(lsbDataPin, csMask, rdMask, wrMask, rsMask);
        }

        virtual void paintScreenBlack () const {
            this->sendMultiData(PropWare::SeeedTFT::BLACK, 38400 * 2);
        }

        virtual void drawVerticalLine (const uint16_t posX, const uint16_t posY,
                uint16_t length, const uint16_t color) const {
            this->setXY(posX, posY);
            this->setOrientation(PropWare::SeeedTFT::VERTICAL);

            if (length + posY > PropWare::SeeedTFT::MAX_Y)
                length = MAX_Y - posY;

            this->sendMultiData(color, length);
        }

        virtual void drawHorizontalLine (const uint16_t posX,
                const uint16_t posY, uint16_t length,
                const uint16_t color) const {
            this->setXY(posX, posY);
            this->setOrientation(PropWare::SeeedTFT::HORIZONTAL);

            if (length + posY > PropWare::SeeedTFT::MAX_X)
                length = MAX_X - posX;

            this->sendMultiData(color, length);
        }

    protected:
        virtual void sendCommand (const uint8_t index) const {
            PropWare::SeeedTFTFast::mailbox = index << 8 + SEND_CMD;
        }

        virtual void sendData (const uint16_t data) const {
            PropWare::SeeedTFTFast::mailbox = data << 8 + SEND_DATA;
        }

        virtual void sendMultiData(const uint16_t data,
                const size_t len) const {
            if (1 < len)
                PropWare::SeeedTFTFast::mailbox = (len - 1) << 8 + REPEAT;
            this->sendData(data);
            while (0 != PropWare::SeeedTFTFast::mailbox);
        }

    protected:
        typedef enum {
            SEND_CMD = 1,
            SEND_DATA,
            REPEAT
        } SeeedTftAsmFunc;

        int8_t m_cog;

        static atomic_t mailbox;
    };
}

#endif /* SEEEDTFTFAST_H_ */