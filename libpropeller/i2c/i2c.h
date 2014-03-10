

#ifndef LIBPROPELLER_I2C_H_
#define LIBPROPELLER_I2C_H_

#include <stdint.h>

#include "libpropeller/i2c/i2c_base.h"

/** A basic I2C driver.
 * 
 * All "device" fields should be the 7 bit address of the device, with the low bit set to 0 (the 7 addres bits are the upper bits). This applies to both the Put (write) and Get (read) cases.
 * 
 * Put and Get are based on I2C communication specification as described by ST in the LSM303DLHC and L3GD20 datasheets.
 * Terms:
 * 
 *    + ST - Start  
 *    + SAD - Slave Address (device)
 *    + SAK - Slave Acknowledge
 *    + SUB - SubAddress (slave register address)
 *    + SP - Stop
 *    + +W - plus write (lowest device bit set to 0)
 *    + +R - plus read (lowest device bit set to 1)
 *    + NMAK - Master No Acknowledge  
 * 
 * I2C differs based on the device that you use. For functions that might be device specific, there is a reference indicator. These references are:
 * 
 *    + ST - ST Microelectronics, particularly the LSM303DLHC and L3GD20 devices.
 *    + MS - Measurement Specialties, particularly the MS5607 and MS5611 devices.
 * 
 * If you're using the multibyte Get and Put with ST based devices, be sure to bitwise OR the register address with 0x80 (the MSb to 1) in order to turn on the auto-increment function (see datasheet for L3GD20 for example). This is not done automatically by this library.
 * 
 * @author SRLM
 */

class I2C {
public:

    I2CBase base_;

    /** Setup the DIRA, OUTA, and INA registers for scl and sda.
     * 
     * @param scl The I2C SCL pin. Defaults to the Propeller default SCL pin.
     * @param sda The I2C SDA pin. Defaults to the Propeller default SDA pin.
     * @param frequency The frequency in hz to run the bus at.
     */
    void Init(const int scl = 28, const int sda = 29, const int frequency = 400000) {
        base_.Init(scl, sda, frequency);
    }

    /** Test for the Acknowledge of a device by sending start and the slave address.
     * 
     * Useful for polling the bus and seeing what devices are available. Ping uses the following format:

        +--------+----+-------+-----+----+
        | Master | ST | SAD+W |     | SP |
        | Slave  |    |       | SAK |    |
        +--------+----+-------+-----+----+

     * @return true if ack was received, false otherwise.
     */
    bool Ping(const unsigned char device) {
        base_.Start();
        bool result = base_.SendByte(device);
        base_.Stop();
        return result;
    }

    /** Put a single byte with the following format:

        +--------+----+-------+-----+-----+-----+------+-----+----+
        | Master | ST | SAD+W |     | SUB |     | BYTE |     | SP |
        | Slave  |    |       | SAK |     | SAK |      | SAK |    |
        +--------+----+-------+-----+-----+-----+------+-----+----+
    
     * Reference: ST
     * 
     * @param device  the 7 bit slave I2C address (in bits 7-1, with bit 0 set to 0)
     * @param address the 8 bit slave register address
     * @param byte    the 8 bits of data to store at @a address.
     * @return        false if one or more nAcks is received, true otherwise
     */
    bool Put(const unsigned char device, const unsigned char address,
            const char byte) {
        bool result;

        base_.Start();
        result = base_.SendByte(device);
        result &= base_.SendByte(address);
        result &= base_.SendByte(byte);
        base_.Stop();

        return result;
    }

    /** Get a single byte with the following format: 

        +--------+----+-------+-----+-----+-----+----+-------+-----+------+------+----+
        | Master | ST | SAD+W |     | SUB |     | ST | SAD+R |     |      | NMAK | SP |
        | Slave  |    |       | SAK |     | SAK |    |       | SAK | DATA |      |    |
        +--------+----+-------+-----+-----+-----+----+-------+-----+------+------+----+
     * 
     * Reference: ST
     * 
     * @param device  the 7 bit slave I2C address (in bits 7-1, with bit 0 set to 0)
     * @param address the 8 bit slave register address
     * @return       the 8 bits of data read from register @a address.
     */
    unsigned char Get(const unsigned char device, const unsigned char address) {
        bool result;

        base_.Start();
        result = base_.SendByte(device);
        result &= base_.SendByte(address);

        base_.Start();
        result &= base_.SendByte(device | 0x01); //Set read bit
        unsigned char dataByte = base_.ReadByte(false);
        base_.Stop();
        return dataByte;
    }

    /** Put multiple bytes with the following format:

                                                |Repeat for # of bytes    |
        +--------+----+-------+-----+-----+-----+------+-----+------+-----+----+
        | Master | ST | SAD+W |     | SUB |     | DATA |     | DATA |     | SP |
        | Slave  |    |       | SAK |     | SAK |      | SAK |      | SAK |    |
        +--------+----+-------+-----+-----+-----+------+-----+------+-----+----+
     * 
     * Reference: ST

    
     * @param device  the 7 bit slave I2C address (in bits 7-1, with bit 0 set to 0)
     * @param address the 8 bit slave register address
     * @param bytes   the set of bytes to store on @a device, starting at register @a address.
     * @param size    the number of bytes to write
     * @return        false if one or more nAcks is received, true otherwise
     */
    bool Put(const unsigned char device, const unsigned char address,
            const char * bytes, const int size) {
        bool result;
        base_.Start();
        result = base_.SendByte(device);
        result &= base_.SendByte(address);

        for (int i = 0; i < size; ++i) {
            result &= base_.SendByte(bytes[i]);
        }
        base_.Stop();

        return result;
    }

    /** Get multiple bytes with the following format:

                                                                   |Repeat for # of bytes -1 | Last byte   |            
        +--------+----+-------+-----+-----+-----+----+-------+-----+------+-----+------+-----+------+------+----+
        | Master | ST | SAD+W |     | SUB |     | ST | SAD+R |     |      | MAK |      | MAK |      | NMAK | SP |
        | Slave  |    |       | SAK |     | SAK |    |       | SAK | DATA |     | DATA |     | DATA |      |    |
        +--------+----+-------+-----+-----+-----+----+-------+-----+------+-----+------+-----+------+------+----+

    
     * Reference: ST
     * 
     * @param device  the 7 bit slave I2C address (in bits 7-1, with bit 0 set to 0).
     * @param address the 8 bit slave register address.
     * @param bytes   the address to begin storing the read bytes at.
     * @param size    the number of bytes to read.
     * @return        false if one or more nAcks is received, true otherwise
     */
    bool Get(const unsigned char device, const unsigned char address,
            char * bytes, const int size) {
        bool result;
        base_.Start();
        result = base_.SendByte(device);
        result &= base_.SendByte(address); //Assert the read multiple bytes bit (ref: L3GD20 datasheet)
        base_.Start();
        result &= base_.SendByte(device | 0x01);

        int i = 0;
        for (; i < size - 1; ++i) {
            bytes[i] = base_.ReadByte(true); //Send true to keep on reading bytes
        }

        bytes[i] = base_.ReadByte(false); //Trailing NAK
        base_.Stop();

        return result;
    }

    /** Put a single byte, no register address, on the bus with the following format :

        +--------+----+-------+-----+------+-----+----+
        | Master | ST | SAD+W |     | DATA |     | SP |
        | Slave  |    |       | SAK |      | SAK |    |
        +--------+----+-------+-----+------+-----+----+

     * 
     * @warning Notice the lack of a specified register!
     * 
     * Reference: MS    
     * @param device  the 7 bit slave I2C address (in bits 7-1, with bit 0 set to 0)
     * @param byte    the 8 bits of data to send to @a device
     * @return        false if one or more nAcks is received, true otherwise
     */
    bool Put(const unsigned char device, const char byte) {
        //Warning: this method is not unit tested! (it's run, but the MS5611 does
        //not have a register that can be written to and read from).

        base_.Start();
        bool result = base_.SendByte(device);
        result &= base_.SendByte(byte);
        base_.Stop();

        return result;
    }

    /** Get multiple bytes, no register address, on the bus with the following format:

                                    |Repeat      |
        +--------+----+-------+-----+------+-----+------+------+----+
        | Master | ST | SAD+R |     |      | MAK |      | NMAK | SP |
        | Slave  |    |       | SAK | DATA |     | DATA |      |    |
        +--------+----+-------+-----+------+-----+------+------+----+

     * 
     * @warning Notice the lack of a specified register!
     * 
     * Reference: MS
     * 
     * @param device  the 7 bit slave I2C address (in bits 7-1, with bit 0 set to 0)
     * @param bytes   the address to begin storing the read bytes at.
     * @param size    the number of bytes to read.
     * @return        false if one or more nAcks is received, true otherwise
     */

    bool Get(const unsigned char device, char * bytes, const int size) {
        base_.Start();
        bool result = base_.SendByte(device | 0x01);
        int i = 0;
        for (; i < size - 1; ++i) {
            bytes[i] = base_.ReadByte(true);
        }
        bytes[i] = base_.ReadByte(false);
        base_.Stop();

        return result;
    }


    /*
    Pass through methods to base:
     */

    /** Passthrough to base. See i2cBase::Start() for more details. */
    void Start() {
        base_.Start();
    }

    /** Passthrough to base. See i2cBase::Stop() for more details. */
    void Stop() {
        base_.Stop();
    }

    /** Passthrough to base. See i2cBase::SendByte(unsigned char) for more details. */
    int SendByte(const unsigned char byte) {
        return base_.SendByte(byte);
    }

    /** Passthrough to base. See i2cBase::ReadByte(int) for more details. */
    unsigned char ReadByte(const int acknowledge) {
        return base_.ReadByte(acknowledge);
    }

};

#endif // LIBPROPELLER_I2C_H_
