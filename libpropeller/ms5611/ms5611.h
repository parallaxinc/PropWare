#ifndef LIBPROPELLER_MS5611_H_
#define LIBPROPELLER_MS5611_H_

#include <propeller.h>
#include "libpropeller/i2c/i2c.h"

#ifdef UNIT_TEST
#undef UNIT_TEST
#define UNIT_TEST_
#endif
#include "libpropeller/stopwatch/stopwatch.h"
#ifdef UNIT_TEST_
#define UNIT_TEST
#endif

/** MS5611 Barometer interface
 * 
 * Provides a simple interface to the MS5611 barometer and temperature sensor. 
 * Uses the I2C interface to the sensor.
 * 
 * @author SRLM (srlm@srlmproductions.com)
 */
class MS5611 {
public:

    /** Specify the least significant address bit.
     */
    enum AddressLSB {
        LSB_0, LSB_1
    };
    
    /** Initialize a new MS5611 instance.
     * 
     */
    MS5611(){
        bus_ = NULL;
        status_ = false;
    }

    /** Initialize MS5611 Barometer instance
     * 
     * This resets and initializes the sensor, reads the PROM, and begins a 
     * conversion.
     * 
     * @a Touch() should be called no sooner than 8.5ms after MS5611 
     * initialization.
     * 
     * @param newbus The I2C bus to use.
     * @param address The least significant byte of the I2C address. Use the enum constants.
     */
    bool Init(I2C * newbus, const AddressLSB address = LSB_0) {

        SetAddress(address);

        bus_ = newbus;

        GetStatus();
        if (status_ == false) {
            return false;
        }

        static const char kPROMRead [] = {
            0b10100000, // 16 bit reserved for manufacturer
            0b10100010, // C1
            0b10100100, // C2
            0b10100110, // C3
            0b10101000, // C4
            0b10101010, // C5
            0b10101100, // C6
            0b10101110 // CRC
        };


        D1_ = 0; // Pressure
        D2_ = 0; // Temperature

        //Read PROM here
        int C[6];
        for (int i = 0; i < 6; i++) {
            char data[2];
            bus_->Put(device_address, kPROMRead[i + 1]);
            bus_->Get(device_address, data, 2);
            C[i] = data[0] << 8 | data[1];

        }
        SetC(C[0], C[1], C[2], C[3], C[4], C[5]);

        convertingTemperature_ = true;
        bus_->Put(device_address, kConvertD2OSR4096);

        newData_ = false;

        timer.Start();

        return status_;
    }

    /** Keep the MS5611 running.
     * 
     * In general, must be called twice for every associated @a Get().
     * 
     * "Touches" the sensor to make sure that it keeps converting at the maximum pace.
     * 
     * @warning Undefined behavior if called more than every 8.5 ms or less than every 53 seconds.
     * 
     * Test results indicate that the Touch function takes the following amount 
     * of time, at 80MHz in:
     * + CMM -Os mode: 49056 cycles (~0.62ms)
     * + LMM -Os mode: 22624 cycles (~0.28ms)
     * @returns true when a new set of data is ready. If result is true, then @a Get() should be called.
     */
    bool Touch(void) {
        if (timer.GetElapsed() < 9) {
            return false;
        }

        //Read ADC on MS5611, and get whatever it was converting.
        char data[3];

        bus_->Put(device_address, kADCRead);

        bus_->Get(device_address, data, 3);
        int reading = ExpandReading(data);
        newData_ = true;

        timer.Start();


        if (convertingTemperature_) {
            D2_ = reading;
            //Set ADC to convert pressure
            bus_->Put(device_address, kConvertD1OSR4096);

            convertingTemperature_ = false;
            return false;
        } else {
            D1_ = reading;
            //Set ADC to convert temperature
            bus_->Put(device_address, kConvertD2OSR4096);

            convertingTemperature_ = true;
            return true;
        }
    }

    /** Get the most current readings from the MS5611 sensor.
     * 
     * Test results indicate that the Get function takes the following amounts 
     * of time, at 80MHz in when calibrationCalculation == true:
     * 
     * + CMM -Os mode: 48448 cycles (~0.62ms)
     * + LMM -Os mode: 13968 cycles (~0.17ms)
     * 
     * @warning Second order temperature compensation has not been tested! 
     * Particularly the very low temperature compensation!
     * @param tPressure The pressure, either raw or in units of 0.01 mBar
     * @param tTemperature The temperature, either raw or in units of 0.01C
     * @param calibrationCalculation Perform calculations to calibrate (they 
     * involve 64 bit integers, so they take a while).
     */
    void Get(int & tPressure, int & tTemperature,
            const bool calibrationCalculation = true) {

        if (calibrationCalculation == true) {
            if (newData_) {
                Calculate();
            }
            tPressure = pressure_;
            tTemperature = temperature_;
            newData_ = false;
        } else {
            tPressure = D1_;
            tTemperature = D2_;
        }
    }

    /**
     * @returns true if device is present and ready, false otherwise
     */
    bool GetStatus(void) {
        if (bus_ == NULL) {
            status_ = false;
        } else {
            status_ = bus_->Ping(device_address);
        }
        return status_;
    }

    /** Reset the pressure sensor.
     * 
     * @warning untested! (How do I test this?)
     * @return true if successfully reset, false otherwise. Takes 2.8ms to reload.
     */
    bool Reset(void) {
        return bus_->Put(device_address, kReset);
    }
private:

    /** Set the C PROM calibration constants.
     * 
     * @param C1
     * @param C2
     * @param C3
     * @param C4
     * @param C5
     * @param C6
     */
    void SetC(const int newC1, const int newC2, const int newC3,
            const int newC4, const int newC5, const int newC6) {
        C1_ = ((int64_t) newC1) << 15;
        C2_ = ((int64_t) newC2) << 16;
        C3_ = (int64_t) newC3;
        C4_ = (int64_t) newC4;

        C5_ = newC5 << 8;
        C6_ = newC6;
    }

    /** Get the C PROM calibration constants.
     * 
     * @param C1
     * @param C2
     * @param C3
     * @param C4
     * @param C5
     * @param C6
     */
    void GetC(int & oldC1, int & oldC2, int & oldC3,
            int & oldC4, int & oldC5, int & oldC6) {
        oldC1 = (int) (C1_ >> 15);
        oldC2 = (int) (C2_ >> 16);
        oldC3 = (int) C3_;
        oldC4 = (int) C4_;
        oldC5 = C5_ >> 8;
        oldC6 = C6_;
    }

    /**
     * @warning This function is for unit testing only!
     * @param newD1 The raw pressure value
     * @param newD2 The raw temperature value
     */
    void TEST_SetD(const int newD1, const int newD2) {
        D1_ = newD1;
        D2_ = newD2;
        newData_ = true;
    }

    void Calculate(void) {
        //These equations are straight from the MS5611 datasheet.
        int dT = D2_ - C5_;
        temperature_ = 2000 + ((dT * C6_) >> 23);

        int64_t T2 = 0;
        int64_t OFF2 = 0;
        int64_t SENS2 = 0;

        if (temperature_ < 2000) {

            int64_t dT64 = dT;

            T2 = (dT64 * dT64) >> 31;
            OFF2 = (5 * (temperature_ - 2000) * (temperature_ - 2000)) >> 1;
            SENS2 = OFF2 >> 1;

            if (temperature_ < -1500) { //Very low temperature
                OFF2 = OFF2 + (7 * (temperature_ + 1500) * (temperature_ + 1500));
                SENS2 = SENS2 + ((11 * (temperature_ + 1500) * (temperature_ + 1500)) >> 1);
            }
        }


        int64_t OFF = C2_ + ((C4_ * dT) >> 7);
        int64_t SENS = C1_ + ((C3_ * dT) >> 8);

        temperature_ = temperature_ - T2;
        OFF = OFF - OFF2;
        SENS = SENS - SENS2;

        pressure_ = (int) ((((((int64_t) D1_) * SENS) >> 21) - OFF) >> 15);
    }

    int ExpandReading(const char data[]) const {
        //MS5611 returns a 24 bit unsigned number.
        return data[0] << 16 | data[1] << 8 | data[2];
    }

    I2C * bus_;
    Stopwatch timer;

    // These variables are straight from the MS5611 datasheet.
    int64_t C1_, C2_, C3_, C4_;
    int C5_, C6_;

    int D1_, D2_;
    int temperature_, pressure_;

    bool newData_;
    bool convertingTemperature_;
    bool status_;

    //unsigned int conversionValidCNT_;


    const static char kConvertD1OSR4096 = 0x48; //D1 is the pressure value
    const static char kConvertD2OSR4096 = 0x58; //D2 is the temperature value
    const static char kADCRead = 0x00;
    const static char kReset = 0b00011110;

    unsigned char device_address;

    void SetAddress(const AddressLSB address) {
        if (address == LSB_0) {
            device_address = 0b11101100;
        } else if (address == LSB_1) {
            device_address = 0b11101110;
        }
    }


public:
    friend class UnityTests;

};

#endif // LIBPROPELLER_MS5611_H_
