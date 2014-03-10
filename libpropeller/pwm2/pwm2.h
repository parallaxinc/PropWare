#ifndef libpropeller_pwm2_h__
#define libpropeller_pwm2_h__


#include <propeller.h>
extern char _load_start_pwm2_cog[];

/** Run a high speed pulse width modulation (PWM) class that can output up to
 * two channels of duty cycled PWM at a single frequency.
 * 
 * The two channels are channel X and channel Y.
 * 
 * Requires a cog to operate. No external hardware is required beyond whatever
 * you are PWM'ing.
 * 
 * You can use this class to PWM H bridges, LEDs, audio, etc.
 * 
 * @todo(SRLM): figure out the maximum frequency (as a function of clock speed)
 * 
 * Copyright (c) 2013 Kenneth Bedolla  (libpropeller@kennethbedolla.com)
 * 
 */
class PWM2 {
public:

    /** Start a PWM driver in a new cog.
     * 
     */
    void Start(void) {
        volatile void * asm_reference = NULL;
        __asm__ volatile ( "mov %[asm_reference], #PWM2_Entry \n\t"
                : [asm_reference] "+r" (asm_reference));

        SetDutyX(0);
        SetDutyY(0);
        SetPinX(-1);
        SetPinY(-1);

        SetFrequency(kDefaultFrequency);
        Cog = cognew(_load_start_pwm2_cog, &Periodxy) + 1;
    }

    /** Stop the PWM driver (if running)
     * 
     */
    void Stop(void) {
        if (Cog > 0) {
            SetDutyX(0);
            SetDutyY(0);
            waitcnt(Periodxy * 2 + CNT);
            cogstop(Cog - 1);
            Cog = 0;
        }

    }

    /** Set a pin to PWM out on.
     * 
     * You can change this during operation.
     * 
     * @param pinX The I/O pin [0..31] to set the X channel on.
     */
    void SetPinX(const int pinX) {
        pin_x = pinX;
        if (pin_x == -1) {
            Pinxmask = 0;
            Ctrxval = 0;

        } else {
            Pinxmask = (1 << pinX);
            Ctrxval = ((4 << 26) + pinX);
        }
    }

    /** Set a pin to PWM out on.
     * 
     * You can change this during operation.
     * 
     * @param pinY The I/O pin [0..31] to set the Y channel on.
     */
    void SetPinY(const int pinY) {
        pin_y = pinY;
        if (pin_y == -1) {
            Pinymask = 0;
            Ctryval = 0;
        } else {
            Pinymask = (1 << pinY);
            Ctryval = ((4 << 26) + pinY);
        }
    }

    /** Output a square wave with specified duty cycle.
     * 
     * @param percent The duty [0..100] to PWM. 0 is off, 100 is full on.
     */
    void SetDutyX(const int percent) {
        Percentx = percent;
        Dutyx = ((percent * Periodxy) / 100);
    }

    /** Output a square wave with specified duty cycle.
     * 
     * You can change this during operation.
     * 
     * @param percent The duty [0..100] to PWM. 0 is off, 100 is full on.
     */
    void SetDutyY(const int percent) {
        Percenty = percent;
        Dutyy = ((percent * Periodxy) / 100);
    }

    /** Set the frequency to output channel X and Y on.
     * 
     * @todo(SRLM): SetFrequency doesn't seem to actually change the frequency (at least once the cog is started...)
     * 
     * @param frequency The frequency in Hz, [0..40000??]
     */
    void SetFrequency(const int frequency) {
        Periodxy = (CLKFREQ / frequency);
        SetDutyX(Percentx);
        SetDutyY(Percenty);
    }
private:
    int Cog;
    int Percentx, Percenty;
    int pin_x, pin_y;
    int volatile Periodxy;
    int volatile Dutyx, Pinxmask, Ctrxval;
    int volatile Dutyy, Pinymask, Ctryval;

    static const int kDefaultFrequency = 12000;
};

#endif // libpropeller_pwm2_h__
