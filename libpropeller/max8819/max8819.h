#ifndef LIBPROPELLER_MAX8819_H_
#define LIBPROPELLER_MAX8819_H_

#include <stdint.h>
#include <propeller.h>


/** Max8819.h - Max8819 class to allow access to single pins

The low level interface code is based on Pins.h from David Michael Betz.

 * Requires a 10k resistor from MAX8819A::CEN to 5V USB.
 * 
@todo Add support (is it neccessary?) For the "timer fault" condition (figure 3,
page 18 of datasheet) when CHG outputs a 2Hz square wave.

@author SRLM (srlm@srlmproductions.com)
@date 1-1-2013

 */
class Max8819 {
private:
    uint32_t cen_mask;
    uint32_t chg_mask;
    uint32_t en_mask;
    uint32_t dlim1_mask;
    uint32_t dlim2_mask;
public:

    /**
    Create the MAX8819 object, turn the power on, and turn off charging.

    @warning pmic->SetCharge(Max8819::HIGH); 
    @TODO(SRLM): There is some sort of bug where this *must* be in the code, otherwise it causes a reset.

    The pins from the Propeller to the MAX8819 can be connected directly: no need
    for current limiting resistors.

    @param CENpin   The pin connected to Charge Enable on the MAX8819
    @param CHGpin   The pin connected to (is) Charge(ing?) pin on the MAX8819
    @param ENpin    The pin connected to Enable123 pin on the MAX8819
    @param DLIM1pin The pin connected to DLIM1 pin on the MAX8819
    @param DLIM2pin The pin connected to DLIM2 pin on the MAX8819

     */
    void Start(int CENpin, int CHGpin, int ENpin, int DLIM1pin, int DLIM2pin);

    ~Max8819();

    /** Turn the power system on. The I/O holds the power on until @a  Off() is
    called.
     */
    void On();

    /** Turn the power system off.

    @warning includes Propeller power, so this had better be the last thing in the
    program.
     */
    void Off();

    /** Checks to see if the battery is being charged.

    @todo If SetCharge is set to off, and it's plugged in, what does GetCharge return?
    @returns the charging status of the battery. True indicates charging, false
             indicates no charging.
 
     */
    bool GetCharge();

    /**
    Sets the charge rate. Valid constants are:
    -OFF
    -LOW
    -MEDIUM
    -HIGH
    Note: The rate is set even if there is currently no external power, and stays in
    effect until next called (possibly during the connection of external power).

     * 
     * @warning If set to OFF it cuts off all power from the USB port. This means that if there is no battery the system will turn off!
     * 
     * @warning If it's set to something other than OFF then the maximum current from the USB port is limited. If there is no battery then you must make sure that the total current consumed is less than the maximum, otherwise the MAX8819 might reset.
     * 
    With a 3kOhm CISET resistor, we have the following charge rates:
    -LOW    == 95   mA
    -MEDIUM == 475  mA
    -HIGH   == 1000 mA

    From the datasheet:
    It is not necessary to limit the charge current based on the capabilities of
    the expected AC-toDC adapter or USB/DC input current limit, the system load, or
    thermal limitations of the PCB. The IC automatically lowers the charging current
    as necessary to accommodate for these factors.

    @param rate a constant specifying the rate of charge.
     */
    void SetCharge(int rate);


    bool GetPluggedIn(void);

    enum {
        HIGH, MEDIUM, LOW, OFF
    };
};

void inline Max8819::Start(int CENpin, int CHGpin, int ENpin, int DLIM1pin, int DLIM2pin) {


    cen_mask = 1 << CENpin;
    chg_mask = 1 << CHGpin;
    en_mask = 1 << ENpin;
    dlim1_mask = 1 << DLIM1pin;
    dlim2_mask = 1 << DLIM2pin;


    On(); // Default pin is off, so if we don't set it on before setting it to
          // output it will output low, which will cause a reset (turn off 
          // power rails).

    DIRA |= en_mask; // Set to output
    DIRA |= dlim1_mask; // Set to output
    DIRA |= dlim2_mask; // Set to output
    DIRA &= ~chg_mask; // Set to input
    DIRA &= ~cen_mask; // Set to input

    SetCharge(HIGH);
}

inline Max8819::~Max8819() {
}

inline void Max8819::On() {
    OUTA |= en_mask;
}

inline void Max8819::Off() {
    OUTA &= ~en_mask;
}

inline bool Max8819::GetCharge() {
    return (INA & chg_mask) == 0;
}

inline bool Max8819::GetPluggedIn() {
    return (INA & cen_mask) != 0;
}

inline void Max8819::SetCharge(int rate) {
    if (rate == OFF) {
        OUTA |= dlim1_mask; //1
        OUTA |= dlim2_mask; //1

    } else if (rate == HIGH) {
        OUTA &= ~dlim1_mask; //0
        OUTA &= ~dlim2_mask; //0

    } else if (rate == MEDIUM) {
        OUTA &= ~dlim1_mask; //0
        OUTA |= dlim2_mask; //1
    } else { //rate == LOW
        OUTA |= dlim1_mask; //1
        OUTA &= ~dlim2_mask; //0
    }
}

#endif // LIBPROPELLER_MAX8819_H_
