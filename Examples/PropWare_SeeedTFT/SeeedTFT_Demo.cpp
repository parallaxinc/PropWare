/**
* @file    SeeedTFT_Demo.cpp
*
* @author  David Zemon
*/

// Includes
#include "SeeedTFT_Demo.h"

// Main function
int main () {
    char const *string = "Happy!";
    char *strPtr = (char *) string;
    uint32_t startTimer, totalTime;

    PropWare::SeeedTFTFast tft;

    startTimer = CNT;
    tft.start(PropWare::Port::P2, PropWare::Pin::P10, PropWare::Pin::P13,
            PropWare::Pin::P12, PropWare::Pin::P11);
    totalTime = CNT - startTimer;
    print("Initialization time: %u ms\n", totalTime / MILLISECOND);

    startTimer = CNT;

    tft.drawString(strPtr, 0, 160, 1, PropWare::SeeedTFT::CYAN);
    tft.drawString(strPtr, 0, 200, 2, PropWare::SeeedTFT::WHITE);
    tft.drawString(strPtr, 0, 240, 4, PropWare::SeeedTFT::WHITE);
    totalTime = CNT - startTimer;
    print("Text print time: %u ms\n", totalTime / MILLISECOND);


    return 0;
}
