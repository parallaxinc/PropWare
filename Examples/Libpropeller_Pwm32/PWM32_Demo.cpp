/**
* @file    PWM32_Demo.cpp
*
* @author  David Zemon
*/

#include "libpropeller/pwm32/pwm32.h"

void pwm2 ();

int main () {
    libpropeller::PWM32 pwm1; //PWM32 constructor
    pwm1.Start();

    //Servo(int pin, int pulse_width) (pulse width in us)
    pwm1.Servo(16, 1);
    pwm1.Servo(17, 100);
    pwm1.Servo(18, 1000);
    pwm1.Servo(19, 1900);

    return 0;
}