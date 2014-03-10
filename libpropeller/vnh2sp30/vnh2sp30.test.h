#include "unity.h"
#include "c++-alloc.h"

#include "libpropeller/vnh2sp30/vnh2sp30.h"

#include "propeller.h"
#include "libpropeller/pwm32/pwm32.h"

vnh2sp30 * sut;

//pwm32 * pwm;


PWM2 * pwm2;
PWM32 * pwm32;

const int pwmPin = 6;
const int inApin = 7;
const int inBpin = 5;

class UnityTests {
public:

    static void setUp(void) {
        pwm2 = new PWM2();
        pwm2->Start();

        pwm32 = new PWM32();
        pwm32->Start();

        waitcnt(CLKFREQ / 10 + CNT);

        sut = new vnh2sp30();


        //Init the PWM2 based version
        sut->Init(pwm2, vnh2sp30::X, pwmPin, inApin, inBpin);

        //Init the PWM32 based version
        //sut->Init(pwm32, pwmPin, inApin, inBpin);


    }

    static void tearDown(void) {
        delete sut;
        sut = NULL;

        pwm2->Stop();
        delete pwm2;
        pwm2 = NULL;

        pwm32->Stop();
        delete pwm32;
        pwm32 = NULL;
    }

    static void test_Nothing(void) {
        for (int i = 0; i <= 90; i++) {
            sut->Set(vnh2sp30::FORWARD, i);
            waitcnt(CLKFREQ / 10 + CNT);
        }
        //TEST_IGNORE_MESSAGE("Need to have a motor connected...");
    }

    static void test_Nothing2(void) {
        sut->Set(vnh2sp30::FORWARD, 100);
        //while(true){}
    }

};



