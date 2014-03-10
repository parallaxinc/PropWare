#ifndef LIBPROPELLER_PIN_H_
#define LIBPROPELLER_PIN_H_

// Comment out one definition of INLINE.
#define INLINE __attribute__((always_inline)) inline 
//#define INLINE inline

#include <propeller.h>

/** Pin class to allow access to single pins
 * 
 * 
 * What's with the __attribute__((always_inline))?
 * SRLM found that the forced inlining is essential to performance. Without
 * it GCC would not inline a function (even if marked) if it was used more than
 * two or three times. This has a rather drastic performance impact. It does
 * come at the expense of some space, however. For example, the high() function
 * expands to 5 instructions:
 *
 *     mov	r7, OUTA
 *     rdlong	r6, r3
 *     or	r7, r6
 *     mov	OUTA, r7
 *     or	DIRA,r6
 * 
 * If you use high() in many places the total could add up. On the other hand,
 * the forced inlining may give the optimizer more chances to make things small.
 * You should try it both ways and compare.
 * 
 * * Hardware:
 * To read 3.3v signals just connect the signal directly to any I/O pin. To read
 * a 5v signal connect it to any I/O pin through a 3.3kOhm (or higher) resistor.
 * To read a higher voltage, use the following equation to determine the proper
 * series voltage:
 * resistance = (voltage - 3.3v - 0.6v) / 500uA
 * 
 * The original source for this class was posted by David Michael Betz, and
 * modified by SRLM.
 * 
 */
class Pin {
public:
    /** Create a null pin (pin with no effect).
     * 
     */
    Pin();


    /** Create the pin instance.
     * 
     * @param pin the I/O pin number (0 through 31) to control.
     */
    Pin(int pin);

    /** Get the pin number [0..31]. Returns -1 if no pin.
     * 
     */
    int getPin(void);


    /** Set pin to output high.
     */
    void high(void);

    /** Set pin to output low.
     */
    void low(void);

    /** Toggle the pin on output. If it was low, make it high. If it was high, 
     * make it low.
     */
    void toggle(void);

    /** Set the pin to input and get it's value.
     * 
     * @return 1 if the pin is high, 0 if it is low.
     */
    int input(void);

    /** Output a value on the pin.
     * 
     * @param setting 1 to output high, 0 to output low.
     */
    void output(int setting);

    /** Get the input or output direction.
     * 
     * @warning Can only tell if the output bit is set for this cog (not all cogs)
     * 
     * @return true if output, false if input
     */
    bool isOutput(void);

    

    /** Output a PWM wave on a pin.
     * 
     * @warning You MUST stop the PWM by calling this function with a frequency
     * of 0!. Otherwise the Propeller counters will continue to output the PWM,
     * even if this class has been garbage collected and destroyed.
     * 
     * @warning This function uses the Propeller counters. There are only two
     * per cog, so that means that you can have up to two PWM waves and that's
     * it! In addition, nothing else can use the counters.
     * 
     * @param decihz       The frequency to PWM at. A parameter of 0 disables the PWM.
     * @param useCTRA      Explicitly specify which counter to use. True for CTRA, false for CTRB
     * @param alternatePin Specify a pin to have an alternating PWM on. That pin will be the logical inverse of whatever this pin is.
     */
    void pwm(const int decihz, const bool useCTRA = true, Pin * alternatePin = NULL) {
        
        low();
        
        const int frq = (decihz * (((1 << 30) / CLKFREQ) << 2)) / 10;
        int ctr = (0b00101000 << 23) + pinNumber;
        
        if(alternatePin != NULL){
            ctr +=  alternatePin->getPin() << 9;
            alternatePin->setOutput();
            alternatePin->low();
        }
        
        
        if(decihz == 0){
            ctr = 0;
        }
        
        if (useCTRA) {
            FRQA = frq;
            CTRA = ctr;
        } else {
            FRQB = frq;
            CTRB = ctr;
        }
        
        setOutput();
    }


private:
    unsigned int pin_mask;
    int pinNumber;
    void setOutput();
};

INLINE Pin::Pin() : pin_mask(0) {
    pinNumber = -1;
}

INLINE Pin::Pin(int pin) : pin_mask(1 << pin) {
    pinNumber = pin;
}

INLINE int Pin::getPin(void) {
    return pinNumber;
}

INLINE void Pin::high() {
    OUTA |= pin_mask;
    setOutput();
}

INLINE void Pin::low() {
    OUTA &= ~pin_mask;
    setOutput();
}

INLINE void Pin::toggle() {
    OUTA ^= pin_mask;
    setOutput();
}

INLINE void Pin::output(int setting) {
    if (setting == 1)
        high();
    else
        low();
}

INLINE bool Pin::isOutput() {
    return (DIRA & pin_mask) ? true : false;
}

INLINE int Pin::input() {
    DIRA &= ~pin_mask;
    return (INA & pin_mask) != 0;
}

INLINE void Pin::setOutput() {
    DIRA |= pin_mask;
}


#endif // LIBPROPELLER_PIN_H_

