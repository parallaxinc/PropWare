#include <stdio.h>
#include <stdint.h>
#include <propeller.h>


#ifndef LIBPROPELLER_ELUM_H_
#define LIBPROPELLER_ELUM_H_

/** Elum class to allow access to single Elums

@warning This should not be used for a "heartbeat" or watchdog type application,
since the LED will remain lit even if the program crashes or does some other
unexpected action.

@warning Not cog-safe: do not attempt to use from multiple cogs, even if they
don't use it simultaneously (it uses cog counters for flashing, so it won't work
on a different cog).

@warning This object may use the cog counters.
 * @TODO(SRLM): In which situations?

 * @TODO(SRLM): What are the hardware requirements?
 * 
Some parts based on Pins.h by David Michael Betz.
Some parts based on an example by Tracey Allen.
 */
class Elum {
public:

    /**
     * Use these constants to set the displayed LED color:
     * Max8819::RED
     * Max8819::GREEN
     */
    enum elumColor {
        RED, GREEN
    };

    enum patternType {
        kSingleSlow, kSingle, kSingleSyncopated, kDouble, kTriple, kManyFast, kJitterFast
    };

    /** Initialize the Elum class.

    @param RedPin the pin that the RED led is sunk to
    @param GreenPin the pin that the GREEN led is sunk to
    @param ButtonPin: the pin to make input to read the button. The button should
                      have a pullup, so that it is NC=1, and active low.
     */
    Elum(int RedPin, int GreenPin, int ButtonPin) {
        Start(RedPin, GreenPin, ButtonPin);
    }

    Elum() {
    }

    void Start(int RedPin, int GreenPin, int ButtonPin) {
        b_mask = 1 << ButtonPin;
        DIRA &= ~b_mask; //Set button to input

        pin_r = RedPin;
        pin_g = GreenPin;

        DIRA |= (1 << pin_r);
        DIRA |= (1 << pin_g);

        _clockfreq = CLKFREQ;
    }

    inline ~Elum() {
        Off();
    }

    /**
@return true if button is pressed, false if it is not.
     */
    inline bool GetButton() {
        return (INA & b_mask) == 0;
    }

    inline void Slowclock(void) {
        _clockfreq = 20000;
    }

    void On(elumColor whichColor) {

        CTRA = (CTRB = 0);
        if (whichColor == RED) {
            OUTA |= (1 << pin_g);
            OUTA &= ~(1 << pin_r);
        } else {
            OUTA &= ~(1 << pin_g);
            OUTA |= (1 << pin_r);
        }
    }

    void Off(void) {
        CTRA = (CTRB = 0);
        OUTA &= ~(1 << pin_r);
        OUTA &= ~(1 << pin_g);
    }

    /**
    Set the LED color to flash.
    
    For example:

        elum.Flash(Elum::GREEN, 1000, 750); //Set Green to turn on for 750ms, and off for 250ms

    @param color     Elum::RED or Elum::GREEN
    @param period_ms The frequency that the color is flashed
    @param flash_ms  The duration of the displayed color. Must be less than period_ms
     */

    void Flash(int color, int period_ms, int flash_ms) {
        CTRA = (CTRB = 0); //Stop counters while updating

        if (color == RED) {
            OUTA |= 1 << pin_g; //Make second pin high
            Pwm(pin_r, period_ms, flash_ms);
        } else if (color == GREEN) {
            OUTA |= 1 << pin_r; //Make second pin high
            Pwm(pin_g, period_ms, flash_ms);

        }
    }

    /**
    Alternate in the following pattern:

    Single Slow: 5, 10, 0          (RG RG RG RG ...) ~13 seconds / cycle
    Single Slow Reverse: 5, 10, 50 (GR GR GR GR ...) ~13 seconds / cycle
    Single: 20, 60,  0             (G R G R G R ...)
    Single Syncopated: 20, 60, 50  (RG GR RG GR ...)
    Double: 20, 100, 0             (R R G G R R ...)
    Triple: 10, 50, 50             (G G GR R RG ...)
    Many Fast: 20, 400, 0          (G ... R ...    ) Fast flickers, repeat each color ~ 10 times ~3 seconds / cycle
    Jitter Fast: 200, 300, 0       (GGRR GGRR   ...) Really fast 

     */

    void Pattern(patternType pattern) {
        Off();
        switch (pattern) {
            case kSingleSlow:
                Pattern(5, 10, 0);
                break;
            case kSingle:
                Pattern(20, 60, 0);
                break;
            case kSingleSyncopated:
                Pattern(20, 60, 50);
                break;
            case kDouble:
                Pattern(20, 100, 0);
                break;
            case kTriple:
                Pattern(10, 50, 50);
                break;
            case kManyFast:
                Pattern(20, 400, 0);
                break;
            case kJitterFast:
                Pattern(300, 400, 0);
                break;
            default:
                Pattern(0, 0, 0);
        }
    }

    /** Fades both LEDs in and out.

    @param frequency The rate at which to flash the combined LEDs, in units of
                     0.1Hz/LSb
     */
    void Fade(int frequency) {
        //Source: http://forums.parallax.com/showthread.php/143483-Make-an-LED-pulse-(smoothly-go-on-and-off)-without-any-code-interaction?p=1160777#post1160777
        Off();
        CTRA = CTRB = 0; //Stop counters while updating.	
        FRQA = (((1000) << 2) * (0x40000000 / (CLKFREQ / 1000))) / 1000; // 1000 Hz base frequency.
        CTRA = (0b00100000 << 23) + pin_r;
        FRQB = (((10000 + frequency) << 2) * (0x40000000 / (CLKFREQ / 1000))) / 10000;
        CTRB = (0b00100000 << 23) + pin_g;
    }


private:
    int pin_r;
    int pin_g;
    int _clockfreq;
    unsigned int b_mask;

    int Fraction(int Y, int X, int B) {
        int F = 0;
        {
            int _idx__0000;
            _idx__0000 = B;
            do {
                Y = (Y << 1);
                F = (F << 1);
                if (Y >= X) {
                    Y = (Y - X);
                    (F++);
                }
                _idx__0000 = (_idx__0000 + -1);
            } while (_idx__0000 >= 1);
        }
        return F;
    }

    int Pwm(int Pin, int Period_ms, int Flash_ms) {
        int Phsx, Frqx;
        FRQA = (FRQB = 0);
        Flash_ms = ((Period_ms / 2) - (Min__(Flash_ms, (Period_ms / 2))));
        Phsx = Fraction(Flash_ms, Period_ms, 32);
        Frqx = Fraction(1, ((_clockfreq / 1000) * Period_ms), 32);
        PHSB = (PHSA + Phsx);
        FRQA = (FRQB = Frqx);
        _clockfreq = CLKFREQ;
        OUTA &= ~(1 << Pin);
        CTRA = (0x10000000 + Pin);
        CTRB = (0x10000000 + Pin);
        return Phsx;
    }

    void Pattern(int tRed, int tGreen, int phsGreen) {
        PHSB = (0x7FFFFFFF / 100 * phsGreen) << 1;
        FRQA = tRed;
        FRQB = tGreen;
        CTRA = (0b00100000 << 23) + pin_r;
        CTRB = (0b00100000 << 23) + pin_g;
    }


    //TODO(SRLM): Get rid of this min.
    static inline int32_t Min__(int32_t a, int32_t b) {
        return a < b ? a : b;
    }
};



#endif // SRLM_PROPGCC_ELUM_H_







