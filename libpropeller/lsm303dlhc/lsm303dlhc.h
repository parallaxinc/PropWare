#ifndef LIBPROPELLER_LSM303DLHC_H_
#define LIBPROPELLER_LSM303DLHC_H_

#ifndef UNIT_TEST
#include "libpropeller/i2c/i2c.h"
#else
#include "libpropeller/i2c/i2cMOCK.h"
#endif

/** Provides an interface to the LSM303DLHC accelerometer and magnetometer.
 * 
 * "Output Data Rate, in digital-output accelerometers, defines the rate at which data is sampled. Bandwidth is the highest frequency signal that can be sampled without aliasing by the specified Output Data Rate. Per the Nyquist sampling criterion, bandwidth is half the Output Data Rate." -Analog Devices
 * 
 * See this question for more information on ODR: 
 * http://electronics.stackexchange.com/questions/67610/lsm303dlhc-low-power-and-high-precision-modes
 * 
 * @author SRLM (srlm@srlmproductions.com)
 */
class LSM303DLHC {
public:

    /** Create a new LSM303DLHC instance.
     */
    LSM303DLHC() {
        bus_ = NULL;
        status_ = false;
    }

    /**Tests to make sure that the LSM303DLHC is actually on the bus, and returns false if it is not. Otherwise, sets the registers as follows and returns true.
     * 
     * Set the control registers of the accelerometer:

- CTRL_REG1_A:
 + 1.344kHz output rate
 + Normal power
 + XYZ enabled

- CTRL_REG4_A:
 + Block data continuous update (default)
 + data LSB @ lower address (default)
 + Full scale +-16G
 + High resolution output enable (SRLM: what does this mean? datasheet is no help)
 + 00 (no functionality)
 + SPI interface mode (default, not used)
     

     * Set the control registers of the Magn :

   - CRA_REG_M:
     + Temperature sensor enable true
     + 00 (no functionality)
     + 220Hz Output data rate
   - CRB_REG_M:
     + 8.1+- guass
     + 00000 (no functionality)
   - MR_REG_M:
     + 000000 (no functionality)
     + Continuous conversion mode (SRLM: What does this actually mean? Datasheet is no help)
     
     * @param  i2cbus The bus that the LSM303DLHC is on.
     * @return        true when both devices are successfully initialized.
     */
    bool Init(I2C * i2c_bus) {
        bus_ = i2c_bus;

        //Check to make sure the LSM303DLHC is actually there.
        status_ = bus_->Ping(kDeviceMagnAddress);
        if (status_ == false) {
            return false;
        }
        
        status_ = bus_->Ping(kDeviceAcclAddress);
        if (status_ == false) {
            return false;
        }


        //Initialize Magn
        bus_->Put(kDeviceMagnAddress, kCRA_REG_M, 0b10011100);
        bus_->Put(kDeviceMagnAddress, kCRB_REG_M, kGain_1_3);
        bus_->Put(kDeviceMagnAddress, kMR_REG_M, 0b00000000);

        //Initialize Accl
        bus_->Put(kDeviceAcclAddress, kCTRL_REG1_A, 0b10010111);
        bus_->Put(kDeviceAcclAddress, kCTRL_REG4_A, 0b00111000);

        return true;
    }

    /** Reads the accelerometer at the current settings, and updates the three reference values.
     * 
     * If there is an error then x, y, and z will be set to zero and false will be returned.
     * 
     * @param x The acceleration x axis value. Will be overwritten.
     * @param y The acceleration y axis value. Will be overwritten.
     * @param z The acceleration z axis value. Will be overwritten.
     * @return true if all is successful, false otherwise. If false, try reinitilizing
     */
    bool ReadAccl(int& x, int& y, int& z) {
        char data[6];

        if (status_ == false) {
            x = y = z = 0;
            return false;
        }

        if (bus_->Get(kDeviceAcclAddress, kOUT_X_L_A, data, 6) == false) {
            x = y = z = 0;
            return false;
        }

        //16 + 4 = 20, or the 12 bit data given by the LSM303DLHC.
        //We are keeping the last 4 bits, and can divide them out later
        //(SRLM thinks the accl is only 12 bits, but the datasheet doesn't seem to
        //specify.
        x = ((data[0] | (data[1] << 8)) << 16) >> 16;
        y = ((data[2] | (data[3] << 8)) << 16) >> 16;
        z = ((data[4] | (data[5] << 8)) << 16) >> 16;

        return true;
    }

    /**Reads the magnetometer at the current settings, and updates the three reference values.
     * 
     * If there is an error then x, y, and z will be set to zero and false will be returned.
     * 
     * @param x The magnetometer x axis value. Will be overwritten.
     * @param y The magnetometer y axis value. Will be overwritten.
     * @param z The magnetometer z axis value. Will be overwritten.
     * @return true if all is successful, false otherwise. If false, try reinitilizing
     */
    bool ReadMagn(int& x, int& y, int& z) {
        char data[6];

        if (status_ == false) {
            x = y = z = 0;
            return false;
        }

        if (bus_->Get(kDeviceMagnAddress, kOUT_X_H_M, data, 6) == false) {
            x = y = z = 0;
            return false;
        }

        //Warning: the LSM303DLHC datasheet lists the magnetometer high registers
        //first, instead of the low (backwards compared to the accel and L3GD20).
        //Also note that the order of the parameters is XZY!
        x = ((data[1] | (data[0] << 8)) << 16) >> 16;
        z = ((data[3] | (data[2] << 8)) << 16) >> 16;
        y = ((data[5] | (data[4] << 8)) << 16) >> 16;

        return true;
    }

private:
    I2C * bus_;

    bool status_;

    const static unsigned char kDeviceAcclAddress = 0b00110010;
    const static unsigned char kDeviceMagnAddress = 0b00111100;

    //For the LSM303DLH Accl
    const static unsigned char kCTRL_REG1_A = 0x20;
    const static unsigned char kCTRL_REG4_A = 0x23;
    const static unsigned char kOUT_X_L_A = 0x28 | 0x80; //(turn on auto increment)

    //For the LSM303DLH Magn
    const static unsigned char kCRA_REG_M = 0x00;
    const static unsigned char kCRB_REG_M = 0x01;
    const static unsigned char kMR_REG_M = 0x02;
    const static unsigned char kOUT_X_H_M = 0x03 | 0x80; //(turn on auto increment)
    
    const static unsigned char kGain_1_9 = 0b01000000;
    const static unsigned char kGain_1_3 = 0b00100000;
};

#endif // LIBPROPELLER_LSM303DLHC_H_
