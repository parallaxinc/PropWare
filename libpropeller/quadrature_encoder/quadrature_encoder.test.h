#include "unity.h"
#include "libpropeller/quadrature_encoder/quadrature_encoder.h"
#include "c++-alloc.h"


const int basePin = 9;
QuadratureEncoder * sut;

class UnityTests {
public:

    static void setUp(void) {
        sut = new QuadratureEncoder();
        printf("Started in cog: %i", sut->Start(9, 2));
        waitcnt(CLKFREQ / 10 + CNT);
    }

    static void tearDown(void) {
        sut->Stop();
        delete sut;
        sut = NULL;
    }

    static void test_Empty(void) {
        //while(true){
        printf("\nPosition: %i", sut->GetReading(1));
        waitcnt(CLKFREQ / 10 + CNT);
        //}
    }

};