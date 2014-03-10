#ifndef LIBPROPELLER_L3GD20_H_
#define LIBPROPELLER_L3GD20_H_

#ifndef UNIT_TEST
#include "libpropeller/i2c/i2c.h"
#else
#include "libpropeller/i2c/i2cMOCK.h"
#endif

/** Provides an interface the L3GD20 gyroscope.
 * 
 * "Output Data Rate, in digital-output accelerometers, defines the rate at which data is sampled. Bandwidth is the highest frequency signal that can be sampled without aliasing by the specified Output Data Rate. Per the Nyquist sampling criterion, bandwidth is half the Output Data Rate." -Analog Devices
 * 
 * See this question for more information on ODR: 
 * http://electronics.stackexchange.com/questions/67610/lsm303dlhc-low-power-and-high-precision-modes
 * 
 * @author SRLM (srlm@srlmproductions.com)
 */
class L3GD20 {
public:

    /** Specify the least significant bit of the L3GD20 I2C address.*/
    enum AddressLSB {
        LSB_0, LSB_1
    };
    
    /** Create a new L3GD20 instance.
     */
    L3GD20(){
        bus_ = NULL;
        status_ = false;
    }

    /** Set the control registers of the device:
     * 
        CTRL_REG1
        -ODR = 760 Hz
        -Cut-Off = 100
        -Normal power
        -XYZ enabled

        CTRL_REG4:
        -Continuous block data update (default)
        -Data LSb at lower address (default)
        -2000+- degrees per second
        -(no functionality)
        -00 (no functionality)
        -SPI interface mode off
     * 
     * @param i2cbus the I2C bus that the gyro is connected to.
     * @return true when the gyro is successfully initialized.
     */
    bool Init(I2C * i2c_bus, const AddressLSB address = LSB_0) {
        SetAddress(address);
        bus_ = i2c_bus;

        //Check to make sure the gyro is actually there.
        status_ = bus_->Ping(device_address);
        if (status_ == false) {
            return false;
        }

        bus_->Put(device_address, kCTRL_REG1, 0b11111111);
        bus_->Put(device_address, kCTRL_REG4, 0b00110000);

        return status_;
    }

    /** Get the current rotation rate readings from the gyroscope.
     * 
     * If there is an error then x, y, and z will be set to zero and false will be returned.
     * 
     * @param x The gyroscope x axis value. Will be overwritten.
     * @param y The gyroscope y axis value. Will be overwritten.
     * @param z The gyroscope z axis value. Will be overwritten.
     *
     * @return true if all is successful, false otherwise. If false, try reinitilizing
     */
    bool ReadGyro(int& x, int& y, int& z) {
        char data[6];

        if (status_ == false) {
            x = y = z = 0;
            return false;
        }

        if (bus_->Get(device_address, kOUT_X_L, data, 6) == false) {
            x = y = z = 0;
            return false;
        }
        x = ((data[0] | (data[1] << 8)) << 16) >> 16;
        y = ((data[2] | (data[3] << 8)) << 16) >> 16;
        z = ((data[4] | (data[5] << 8)) << 16) >> 16;

        return true;
    }

private:
    I2C * bus_;

    int status_;


    const static unsigned char kCTRL_REG1 = 0x20;
    const static unsigned char kCTRL_REG4 = 0x23;
    const static unsigned char kOUT_X_L = 0x28 | 0x80; //(turn on auto increment)

    unsigned char device_address;

    void SetAddress(const AddressLSB address) {
        if (address == LSB_0) {
            device_address = 0b11010100;
        } else if (address == LSB_1) {
            device_address = 0b11010110;
        }
    }

};

#endif // LIBPROPELLER_L3GD20_H_
