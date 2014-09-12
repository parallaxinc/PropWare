/**
* @file        seeedtft.h
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

#ifndef SEEEDTFT_H_
#define SEEEDTFT_H_

#include <PropWare/PropWare.h>
#include <PropWare/port.h>
#include <PropWare/pin.h>

namespace PropWare {
    class SeeedTFT {
    public:
        typedef enum {
            RED = 0xf800,
            GREEN = 0x07e0,
            BLUE = 0x001f,
            BLACK = 0x0000,
            YELLOW = 0xffe0,
            WHITE = 0xffff,

            CYAN = 0x07ff,
            BRIGHT_RED = 0xf810,
            GRAY1 = 0x8410,
            GRAY2 = 0x4208
        } Color;

        typedef enum {
            LEFT_TO_RIGHT,
            DOWN_TO_UP,
            RIGHT_TO_LFET,
            UP_TO_DOWN
        } DisplayDirection;

        typedef enum {
            MIN_X = 0,
            MIN_Y = 0,
            MAX_X = 240,
            MAX_Y = 320
        } Dimension;

        typedef enum {
            HORIZONTAL,
            VERTICAL
        } Orientation;

    public:
        SeeedTFT () {
            this->m_displayDirection = PropWare::SeeedTFT::LEFT_TO_RIGHT;
        };

        void start (const PropWare::Pin::Mask lsbDataPin,
                const PropWare::Port::Mask csMask,
                const PropWare::Port::Mask rdMask,
                const PropWare::Port::Mask wrMask,
                const PropWare::Port::Mask rsMask) {
            this->m_cs.set_mask(csMask);
            this->m_rd.set_mask(rdMask);
            this->m_wr.set_mask(wrMask);
            this->m_rs.set_mask(rsMask);

            this->m_cs.set_dir(PropWare::Port::OUT);
            this->m_rd.set_dir(PropWare::Port::OUT);
            this->m_wr.set_dir(PropWare::Port::OUT);
            this->m_rs.set_dir(PropWare::Port::OUT);

            this->m_dataPort.set_mask(lsbDataPin, 8);
            this->m_dataPort.low();
            this->m_dataPort.set_dir(PropWare::Port::OUT);

            waitcnt(100 * MILLISECOND + CNT);

            this->sendCommand(0x0001);
            this->sendData(0x0100);
            this->sendCommand(0x0002);
            this->sendData(0x0700);
            this->sendCommand(0x0003);
            this->sendData(0x1030);
            this->sendCommand(0x0004);
            this->sendData(0x0000);
            this->sendCommand(0x0008);
            this->sendData(0x0302);
            this->sendCommand(0x000A);
            this->sendData(0x0000);
            this->sendCommand(0x000C);
            this->sendData(0x0000);
            this->sendCommand(0x000D);
            this->sendData(0x0000);
            this->sendCommand(0x000F);
            this->sendData(0x0000);

            waitcnt(100 * MILLISECOND + CNT);

            this->sendCommand(0x0030);
            this->sendData(0x0000);
            this->sendCommand(0x0031);
            this->sendData(0x0405);
            this->sendCommand(0x0032);
            this->sendData(0x0203);
            this->sendCommand(0x0035);
            this->sendData(0x0004);
            this->sendCommand(0x0036);
            this->sendData(0x0B07);
            this->sendCommand(0x0037);
            this->sendData(0x0000);
            this->sendCommand(0x0038);
            this->sendData(0x0405);
            this->sendCommand(0x0039);
            this->sendData(0x0203);
            this->sendCommand(0x003c);
            this->sendData(0x0004);
            this->sendCommand(0x003d);
            this->sendData(0x0B07);
            this->sendCommand(0x0020);
            this->sendData(0x0000);
            this->sendCommand(0x0021);
            this->sendData(0x0000);
            this->sendCommand(0x0050);
            this->sendData(0x0000);
            this->sendCommand(0x0051);
            this->sendData(0x00ef);
            this->sendCommand(0x0052);
            this->sendData(0x0000);
            this->sendCommand(0x0053);
            this->sendData(0x013f);

            waitcnt(100 * MILLISECOND + CNT);

            this->sendCommand(0x0060);
            this->sendData(0xa700);
            this->sendCommand(0x0061);
            this->sendData(0x0001);
            this->sendCommand(0x0090);
            this->sendData(0x003A);
            this->sendCommand(0x0095);
            this->sendData(0x021E);
            this->sendCommand(0x0080);
            this->sendData(0x0000);
            this->sendCommand(0x0081);
            this->sendData(0x0000);
            this->sendCommand(0x0082);
            this->sendData(0x0000);
            this->sendCommand(0x0083);
            this->sendData(0x0000);
            this->sendCommand(0x0084);
            this->sendData(0x0000);
            this->sendCommand(0x0085);
            this->sendData(0x0000);
            this->sendCommand(0x00FF);
            this->sendData(0x0001);
            this->sendCommand(0x00B0);
            this->sendData(0x140D);
            this->sendCommand(0x00FF);
            this->sendData(0x0000);

            waitcnt(100 * MILLISECOND + CNT);

            this->sendCommand(0x0007);
            this->sendData(0x0133);

            waitcnt(50 * MILLISECOND + CNT);

            this->exitStandBy();
            this->sendCommand(0x0022);

            //paint screen black
            this->paintScreenBlack();
        }

#ifndef DOXYGEN_IGNORE

        __attribute__ ((fcache))
#endif
        void paintScreenBlack (void) {
            for (unsigned int f = 0; f < 76800; f++)
                this->sendData(PropWare::SeeedTFT::BLACK);
        }

        void exitStandBy (void) {
            this->sendCommand(0x0010);
            this->sendData(0x14E0);
            waitcnt(100 * MILLISECOND + CNT);
            this->sendCommand(0x0007);
            this->sendData(0x0133);
        }

        void setOrientation(const PropWare::SeeedTFT::Orientation
        orientation) const {
            this->sendCommand(0x03);
            if(PropWare::SeeedTFT::HORIZONTAL == orientation)
                this->sendData(0x5030);
            else
                this->sendData(0x5038);
            this->sendCommand(0x0022); //Start to write to display RAM
        }

        void setXY (const uint16_t poX, const uint16_t poY) const {
            this->sendCommand(0x0020);//X
            this->sendData(poX);
            this->sendCommand(0x0021);//Y
            this->sendData(poY);
            this->sendCommand(0x0022);//Start to write to display RAM
        }

        void drawVerticalLine (const uint16_t poX, const uint16_t poY,
                uint16_t length, const uint16_t color) const {
            this->setXY(poX, poY);
            this->setOrientation(PropWare::SeeedTFT::VERTICAL);
            if (length + poY > MAX_Y) {
                length = MAX_Y - poY;
            }

            for (unsigned int i = 0; i < length; i++) {
                this->sendData(color);
            }
        }

        void drawHorizontalLine (const uint16_t poX, const uint16_t poY,
                uint16_t length, const uint16_t color) const {
            this->setXY(poX, poY);
            this->setOrientation(PropWare::SeeedTFT::HORIZONTAL);
            if (length + poX > MAX_X)
                length = MAX_X - poX;
            for (uint16_t i = 0; i < length; i++)
                sendData(color);
        }


        void drawRectangle (const uint16_t poX, const uint16_t poY,
                const uint16_t length, const uint16_t width,
                const uint16_t color) const {
            this->drawHorizontalLine(poX, poY, length, color);
            this->drawHorizontalLine(poX, poY + width, length, color);

            this->drawVerticalLine(poX, poY, width, color);
            this->drawVerticalLine(poX + length, poY, width, color);
        }

        void fillRectangle (uint16_t posX, uint16_t posY, uint16_t length,
                uint16_t width, uint16_t color) const {
            for (unsigned int i = 0; i < width; i++)
                switch (this->m_displayDirection) {
                    case LEFT_TO_RIGHT:
                        drawHorizontalLine(posX, posY + i, length, color);
                        break;
                    case DOWN_TO_UP:
                        drawHorizontalLine(posX, posY - i, length, color);
                        break;
                    case RIGHT_TO_LFET:
                        drawHorizontalLine(posX, posY - i, length, color);
                        break;
                    case UP_TO_DOWN:
                        drawHorizontalLine(posX, posY + i, length, color);
                        break;
                    default:
                        break;
                }
        }

        void drawChar (char ascii, uint16_t poX, uint16_t poY, uint16_t size,
                const uint16_t fgColor) const {

            setXY(poX, poY);

            // Check for unsupported character
            if ((0x20 > ascii) || (0x7e < ascii))
                ascii = '?';

            for (uint8_t i = 0; i < 8; i++) {
                uint8_t temp = PropWare::SeeedTFT::SIMPLE_FONT[ascii - 0x20][i];
                for (uint8_t j = 0; j < 8; j++) {
                    if ((temp >> j) & PropWare::BIT_0)
                        switch (this->m_displayDirection) {
                            // TODO: Optimization can be performed by using
                            // addition for `i * size` and `j * size`
                            case LEFT_TO_RIGHT:
                                this->fillRectangle(poX + i * size,
                                        poY + j * size, size, size, fgColor);
                                break;
                            case DOWN_TO_UP:
                                this->fillRectangle(poX + j * size,
                                        poY - i * size, size, size, fgColor);
                                break;
                            case RIGHT_TO_LFET:
                                this->fillRectangle(poX - i * size,
                                        poY - j * size, size, size, fgColor);
                                break;
                            case UP_TO_DOWN:
                                this->fillRectangle(poX - j * size,
                                        poY + i * size, size, size, fgColor);
                                break;
                            default:
                                break;
                        }
                }
            }
        }

        void drawString (char *string, uint16_t posX, uint16_t posY,
                uint16_t size, const uint16_t fgColor) const {
            while (*string) {
                for (uint8_t i = 0; i < 8; i++)
                    this->drawChar(*string, posX, posY, size, fgColor);
                *string++;

                switch (this->m_displayDirection) {
                    case LEFT_TO_RIGHT:
                        if (posX < MAX_X)
                            posX += size << 3; // Move cursor right
                        break;
                    case DOWN_TO_UP:
                        if (posY > 0)
                            posY -= size << 3; // Move cursor right
                        break;
                    case RIGHT_TO_LFET:
                        if (posX > 0)
                            posX -= size << 3; // Move cursor right
                        break;
                    case UP_TO_DOWN:
                        if (posY < MAX_Y)
                            posY += size << 3; // Move cursor right
                        break;
                    default:
                        break;
                }
            }
        }

    protected:
#ifndef DOXYGEN_IGNORE

        __attribute__ ((fcache))
#endif
        void sendCommand (const uint_fast8_t index) const {
            this->m_cs.low();
            this->m_rs.low();
            this->m_rd.high();

            this->m_wr.low();
            this->m_dataPort.write(0);
            this->m_wr.toggle();

            this->m_wr.toggle();
            this->m_dataPort.write(index & PropWare::BYTE_0);
            this->m_wr.toggle();

            this->m_cs.toggle();
        }


#ifndef DOXYGEN_IGNORE

        __attribute__ ((fcache))
#endif
        void sendData (const uint_fast16_t data) const {
            this->m_cs.low();
            this->m_rs.high();
            this->m_rd.high();

            this->m_wr.low();
            this->m_dataPort.write(data >> 8);
            this->m_wr.toggle();

            this->m_wr.toggle();
            this->m_dataPort.write(data & PropWare::BYTE_0);
            this->m_wr.toggle();

            this->m_cs.toggle();
        }

    protected:
        PropWare::SimplePort m_dataPort;

        PropWare::Pin m_cs;
        PropWare::Pin m_rs;
        PropWare::Pin m_rd;
        PropWare::Pin m_wr;

        uint8_t m_displayDirection;

        static const uint8_t SIMPLE_FONT[][8];
    };
}

#endif /* SEEEDTFT_H_ */