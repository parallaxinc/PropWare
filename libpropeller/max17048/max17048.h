
#ifndef LIBPROPELLER_MAX17048_H_
#define LIBPROPELLER_MAX17048_H_

#include "libpropeller/i2c/i2c.h"

/**Interface to the MAX17048 Fuel Chip
 * 
 * This chip monitors a single cell Lithium battery, and calculates metrics of battery use.
 * 
 * Hardware: The MAX17048 should be connected as described in the datasheet. The only connections required to the Propeller are via the I2C bus.
 * 
 * One MAX17948 can be connected per I2C bus.
 * 
 * @TODO(SRLM): Add temperature compensation
 * 
 * @author SRLM
 */
class MAX17048 {
public:

    /** Create a new MAX17048 instance.
     */
    MAX17048() {
        bus_ = NULL;
        status_ = false;
    }

    /**Initialize a MAX17048 object.
     * 
     * @param bus The I2C bus that the MAX17048 is on.
     * @return true if successfully initiated, false otherwise.
     */
    bool Init(I2C * bus) {
        bus_ = bus;
        GetStatus();
        return status_;
    }

    /** Test the presence of the MAX17048 chip by pinging the bus.
     * 
     * @return true if the MAX17048 is found, false otherwise.
     */
    bool GetStatus(void) {
        status_ = bus_->Ping(kDeviceAddress);
        return status_;
    }

    /**Get the state of charge from the chip.
     * 
     * The state of charge refers to the overall percentage of the battery remaining. The MAX17048 chip automatically adjusts for a number of features, including battery capacity.
     * 
     * @todo Figure out what voltage the 0% SOC is at, and if we can change it (since we can only run down to about 3.45v with MAX8819 for 3.3v systems.
     * 
     * @returns the current state of charge in units of 1%/LSb
     */
    int GetStateOfCharge(void) {
        int soc = GetShort(kSOC);
        return ((unsigned) soc) >> 8;
    }

    /** Use the MAX17048 internal ADC, and get the voltage
     * 
     * @returns the current voltage in units of 1mv/LSb (0.001v)
     */
    int GetVoltage(void) {
        int voltage = GetShort(kVCELL);

        //Originally units of 78.125uV/LSb,
        //Convert to 1mV/LSb
        voltage = ((voltage * 7812) / 100) / 1000;

        return voltage;
    }

    /** Fetches the rate of charge (positive or negative)
     * 
     * The charge rate is the change in battery state of charge over time. It should not be used to calculate current consumption (eg milliamps).
     * 
     * @todo(SRLM): Figure out the sign issues with this.
     * 
     * @returns the charge rate in units of 0.1%/hr. Note: Positive or negative for
     */
    int GetChargeRate(void) {
        //SRLM doesn't know if this is signed or not, but if it is then sign extend
        //And hopefully, it's not unsigned *and* 16 bits!
        int rate = (GetShort(kCRATE) << 16) >> 16;

        return (rate * 208) / 100;
    }

    /** Fetches the hardware version of the MAX17048.
     * 
     * @returns the version
     */
    int GetVersion(void) {
        return GetShort(kVERSION);
    }


private:

    const static unsigned char kDeviceAddress = 0b01101100;

    const static unsigned char kVCELL = 0x02;
    const static unsigned char kSOC = 0x04;
    const static unsigned char kVERSION = 0x08;
    const static unsigned char kCRATE = 0x16;

    I2C * bus_;
    bool status_;

    int GetShort(char address) {
        char data[2];
        bus_->Get(kDeviceAddress, address, data, 2);
        int result = (data[0] << 8) | data[1];
        return result;
    }

};


#endif // LIBPROPELLER_MAX17048_H_
