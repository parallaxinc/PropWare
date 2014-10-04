/**
* @file    SeeedTFT_Demo.cpp
*
* @author  David Zemon
*/

// Includes
#include "SeeedTFT_Demo.h"

// Main function
int main () {
    char     buffer[64];
    uint32_t startTimer, totalTime;

    PropWare::SeeedTFTFast tft;
    PropWare::SimplexUART  uart(PropWare::Pin::P30);

    startTimer = CNT;
    tft.start(PropWare::Port::P2, PropWare::Pin::P10, PropWare::Pin::P13,
            PropWare::Pin::P12, PropWare::Pin::P11);
    totalTime = CNT - startTimer;

    sprint(buffer, "Initialization time: %u (ms)" CRLF,
            totalTime / MILLISECOND);
    uart.puts(buffer);

    startTimer = CNT;

    tft.drawString((char *) "Happy!", 0, 160, 1, PropWare::SeeedTFT::CYAN);
    tft.drawString((char *) "Happy!", 0, 200, 2, PropWare::SeeedTFT::WHITE);
    tft.drawString((char *) "Happy!", 0, 240, 4, PropWare::SeeedTFT::WHITE);
    totalTime = CNT - startTimer;
    sprint(buffer, "Text print time: %u (ms)" CRLF, totalTime / MILLISECOND);
    uart.puts(buffer);

    return 0;
}
