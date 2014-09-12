/**
* @file    SeeedTFT_Demo.cpp
*
* @author  David Zemon
*/

// Includes
#include <PropWare/seeedtft.h>

// Main function
int main () {
    PropWare::SeeedTFT tft;
    tft.start(PropWare::Port::P2, PropWare::Pin::P10, PropWare::Pin::P13,
            PropWare::Pin::P12, PropWare::Pin::P11);

    tft.drawString("Happy!", 0, 160, 1, PropWare::SeeedTFT::CYAN);
    tft.drawString("Happy!", 0, 200, 2, PropWare::SeeedTFT::WHITE);
    tft.drawString("Happy!", 0, 240, 4, PropWare::SeeedTFT::WHITE);

    return 0;
}
