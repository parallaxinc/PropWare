/**
* @file    SeeedTFT_Demo.cpp
*
* @author  David Zemon
*/

// Includes
#include <PropWare/PropWare.h>
#include <PropWare/seeedtft.h>
#include <PropWare/seeedtftfast.h>
#include <PropWare/uart/simplexuart.h>
#include <PropWare/printer/printer.h>

// Main function
int main () {
    uint32_t startTimer, totalTime;

    PropWare::SeeedTFTFast tft;

    startTimer = CNT;
    tft.start(PropWare::Port::P2, PropWare::Pin::P10, PropWare::Pin::P13,
            PropWare::Pin::P12, PropWare::Pin::P11);
    totalTime = CNT - startTimer;

    pwOut.printf("Initialization time: %u (ms)" CRLF,
            totalTime / MILLISECOND);

    startTimer = CNT;

    tft.drawString((char *) "Happy!", 0, 160, 1, PropWare::SeeedTFT::CYAN);
    tft.drawString((char *) "Happy!", 0, 200, 2, PropWare::SeeedTFT::WHITE);
    tft.drawString((char *) "Happy!", 0, 240, 4, PropWare::SeeedTFT::WHITE);
    totalTime = CNT - startTimer;
    pwOut.printf("Text print time: %u (ms)" CRLF, totalTime / MILLISECOND);

    return 0;
}
