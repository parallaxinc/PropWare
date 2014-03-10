
#ifndef libpropeller_vnh2sp30_h__
#define libpropeller_vnh2sp30_h__

#include "pwm32/pwm32.h"
#include "pwm2/pwm2.h"

#include "pin/pin.h"

/** Operate a VNH2SP30 brushed DC motor driver.
 * 
 * This class runs using either the PWM2 or PWM32 class. The advantage of the
 * PWM2 class is that it can be run at a higher frequency, which may reduce
 * audible motor noise. The PWM32 class has the advantage that it can run as
 * many channels as PWM as needed (up to 32), including up to 10 channels of
 * VNH2SP30 drivers. It can also run servos and general PWM. The downside is
 * that the maximum PWM frequency is lower.
 * 
 * This object requires the VNH2SP30 driver chip connected via INA, INB, and the
 * PWM pin. Note that the VNH2SP30 driver requires a minimum of 3.25v for the
 * high signal, and a minimum of 0.6v for the low signal. To reliably output
 * this signal you will need to add some sort of 3.3v->5v conversion from the
 * Propeller pin to the VNH2SP30 I/O pins.
 * 
 * One option for voltage conversion is to use a 20kOhm resistor connected from
 * the signal line (Propeller I/O -> VNH2SP30) to 5v. This will pull up the
 * signal to 3.9v when outputting high (3.3v + 0.6v for the protective diode) 
 * and the Propeller pin can pull the signal down to 0v when outputting low.
 * Thanks goes to Mike Green for this simple option. SRLM hasn't experienced any
 * issues with this circuit, but your experience may differ.
 * 
 * A more complex (and likely reliable) option would be to use appropriate level
 * converter chips as described in the Parallax Eddie schematic.
 * 
 * Note that this driver does not have any feedback on what the actual speed of
 * the motor is. A heavy load will decrease the actual speed, and requires more
 * power delivered (a higher PWM duty cycle) to increase the actual speed of the
 * motor.
 * 
 * @todo(SRLM): add in the minimum off time. Although this doesn't seem to be necessary on my test module.
 */

class vnh2sp30 {
public:

    /** Construct a new instance. Does not start a new cog.
     */
    vnh2sp30() {
        pwm32 = NULL;
        pwm2 = NULL;
        channel = NONE;
    }

    /** Delete this instance. Does not stop the PWM driver, but does stop the 
     * motor.
     */
    ~vnh2sp30() {
        SetPower(0);
        waitcnt(CLKFREQ / 10 + CNT);
        inA.input();
        inB.input();
        pwmPin.input();
    }

    

    /** If direction needs to be specified use these constants.
     */
    enum Direction {
        FORWARD, REVERSE
    };

    /** If using the PWM2 driver you must specify a channel to use. Each
     * VNH2SP30 driver can use at most one channel.
     */
    enum PwmChannel {
        NONE, X, Y
    };

    /** Initialize this motor driver using a PWM32 object.
     * 
     * Sets the motor to stop.
     * 
     * @param new_pwm    The PWM32 driver to use. Must be Start()'d
     * @param new_pwmPin The pin connected to the VNH2SP30 PWM pin (with voltage translator)
     * @param inApin     The pin connected to inA (with voltage translator)
     * @param inBpin     The pin connected to inB (with voltage translator)
     */
    void Init(PWM32 * new_pwm, const int new_pwmPin, const int inApin, const int inBpin) {
        inA = Pin(inApin);
        inB = Pin(inBpin);
        pwmPin = Pin(new_pwmPin);

        inA.low();
        inB.low();
        pwmPin.low();

        pwm32 = new_pwm;

        SetDirection(FORWARD);
        SetPower(0);
    }

    /** Initialize this motor driver using a PWM2 object.
     * 
     * Sets the motor to stop.
     * 
     * @param new_pwm     The PWM2 driver to use. Must be Start()'d
     * @param new_channel The PWM2 driver channel to use. Must not be used by anything else
     * @param new_pwmPin  The pin connected to the VNH2SP30 PWM pin (with voltage translator)
     * @param inApin      The pin connected to inA (with voltage translator)
     * @param inBpin      The pin connected to inB (with voltage translator)
     */
    void Init(PWM2 * new_pwm, const PwmChannel new_channel, const int new_pwmPin, const int inApin, const int inBpin) {
        inA = Pin(inApin);
        inB = Pin(inBpin);
        pwmPin = Pin(new_pwmPin);

        inA.low();
        inB.low();
        pwmPin.low();


        pwm2->SetFrequency(kFrequency);

        pwm2 = new_pwm;
        channel = new_channel;

        if (channel == X) {
            pwm2->SetPinX(pwmPin.getPin());
        } else if (channel == Y) {
            pwm2->SetPinY(pwmPin.getPin());
        }

        SetDirection(FORWARD);
        SetPower(0);
    }
    
    
    /** Set motor speed and direction.
     * 
     * @param power Set the power in range [-100..100] where negative is reverse
     *                   and positive is forward
     */
    void Set(const int power){
        if(power < 0){
            SetDirection(REVERSE);
            SetPower(-power);
        }else{
            SetDirection(FORWARD);
            SetPower(power);
        }
    }

    /** Set motor speed and direction
     * 
     * @param direction The direction
     * @param power     The power [0..100] to deliver to the motor
     */
    void Set(const Direction direction, const int power) {
        SetDirection(direction);
        SetPower(power);
    }

    
    /** Set the power delivered to the motor.
     * 
     * @param power The power [0..100] to deliver to the motor
     */
    void SetPower(const int power) {
        int adjustedSpeed;
        if (power < 0) {
            adjustedSpeed = 0;
        } else if (power > 100) {
            adjustedSpeed = 100;
        } else {
            adjustedSpeed = power;

        }

        if (pwm32 != NULL) {
            SetSpeedPwm32(adjustedSpeed);
        } else if (pwm2 != NULL) {
            SetSpeedPwm2(adjustedSpeed);
        }
    }

    
    /** The direction to rotate the motor
     * 
     * @param direction
     */
    void SetDirection(const Direction direction) {
        if (direction == FORWARD) {
            inA.high();
            inB.low();
        } else { //REVERSE
            inA.low();
            inB.high();
        }
    }

private:

    Pin inA, inB, pwmPin;
    PWM32* pwm32;
    PWM2* pwm2;

    PwmChannel channel;

    static const int kFrequency = 1000;


    static const int kResolution = 10;
    static const int kScale = 2147483647 / (1 << (kResolution - 1));

    void SetSpeedPwm2(const int speed) {

        if (channel == X) {
            pwm2->SetDutyX(speed);
        } else if (channel == Y) {
            pwm2->SetDutyY(speed);
        }
    }

    void SetSpeedPwm32(const int speed) {
        pwm32->Duty(pwmPin.getPin(), speed, 1000000 / kFrequency);
    }












};

#endif // libpropeller_vnh2sp30_h__
