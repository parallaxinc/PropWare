#ifndef LIBPROPELLER_EEPROM_H_
#define LIBPROPELLER_EEPROM_H_

#include "libpropeller/i2c/i2c_base.h"

/** Access I2C EEPROMs.
 * 
 * Valid EEPROM data addresses are in the range 0 to 0xFFFF for 64KB EEPROMS, and 0 to 0x7FFF for 32KB EEPROMs.
 * 
 * @warning Requires that the A0, A1, and A2 pins of the EEPROM to be tied to ground.
 * 
 * This driver was written to work with the AT24C512C EEPROM from Atmel, but it
 * may work with other I2C EEPROMs.
 */
class EEPROM {
public:

    /** Initialize an @a EEPROM instance. Does not require a cog.
     * 
     * @todo(SRLM): Does this class interfere with other I2C drivers?
     * 
     * @param scl The I2C SCL pin. Defaults to the Propeller default SCL pin.
     * @param sda The I2C SDA pin. Defaults to the Propeller default SDA pin.
     */


    void Init(const int scl = 28, const int sda = 29) {
        base_.Init(scl, sda);
    }

    /** Put a byte into the EEPROM.
     * 
     * @param  address the two byte address to write to.
     * @param  byte    the single byte of data to write
     * @return         success or failure
     */
    bool Put(const unsigned short address, const char byte) {
        char bytes[] = {byte};
        return Put(address, bytes, 1);
    }

    /** Write a block of data to the EEPROM. There are no restrictions on page alignment.
     *     
     * For optimal efficiency, the data should be aligned in 128 byte blocks starting with an address whose lowest 7 bits are 0000000. The is a slight (one time) performance cost to not aligning the data this way.
     * 
     * @param  address the two byte address to write to.
     * @param  bytes[] the array of bytes to write.
     * @param  size    the number of bytes to write.
     * @return         success or failure
     */
    bool Put(unsigned short address, char bytes [], int size) {
        //The lower seven bits define an EEPROM page, so we need a bit of magic to make
        //sure that if we go over the boundary, we start a new page.

        int bytesWritten = 0;
        while (bytesWritten < size) {
            if (PollForAcknowledge() == false) {
                return false;
            }
            base_.SendByte((address >> 8) & 0xFF); //High address byte
            base_.SendByte(address & 0xFF); //Low address byte
            do {
                base_.SendByte(bytes[bytesWritten]); //Payload	
                bytesWritten++;
                address++;
            } while ((address & 0b1111111) != 0 && bytesWritten < size); //detect rollover

            base_.Stop();
        }

        return true;
    }

    /** Write up to 4 bytes to a memory location. Stores them in little endian order.
     * 
     * This function is useful for storing an int (long, 4 bytes) of data to the EEPROM. It can also be used to store a short (word, 2 bytes).
     * 
     * @param  address the two byte address to write to.
     * @param  bytes   the set of bytes to store. If length is less than 4, @a Put stores the bytes beginning with the LSB.
     * @param  length  the number of bytes to store, up to 4.    
     * @return         success or failure
     */
    bool PutNumber(const unsigned short address, const int bytes, const int length) {

        char temp[4];
        //Even if length is < 4, do them all (easier than a loop). Only the used ones will be written.
        temp[3] = (((unsigned) bytes) & 0xFF000000) >> 24;
        temp[2] = (((unsigned) bytes) & 0xFF0000) >> 16;
        temp[1] = (((unsigned) bytes) & 0xFF00) >> 8;
        temp[0] = (((unsigned) bytes) & 0xFF) >> 0;
        return Put(address, temp, length);
    }

    /** Get a single byte from the EEPROM.
     * 
     * @param  address the two byte address to read from.
     * @return the byte read on success, or -1 on timeout or other failure.
     */
    int Get(unsigned short address) {
        char byte[1];
        int result = Get(address, byte, 1);
        if (result < 0) {
            return result;
        } else {
            return byte[0];
        }
    }

    /** Get a block of bytes from the EEPROM.
     * 
     * Note: This function takes care of the page reads from the EEPROM.
     * 
     * @param address the two byte address to read from.
     * @param bytes   the memory location to store the data.
     * @param length  the number of bytes to read.
     * @return 0 on success, or -1 on timeout or other failure.
     */
    int Get(unsigned short address, char bytes [], const int length) {
        int bytesRead = 0;
        while (bytesRead < length) {
            if (PollForAcknowledge() == false) {
                return -1;
            }
            base_.SendByte((address >> 8) & 0xFF); //High address byte
            base_.SendByte(address & 0xFF); //Low address byte
            base_.Start();
            base_.SendByte(kI2CAddress | 0x01); //device EEPROM read (w/ read bit set)


            while (((address + 1) & 0b1111111) != 0
                    && bytesRead + 1 < length) {
                bytes[bytesRead] = base_.ReadByte(true);
                bytesRead++;
                address++;
            }

            bytes[bytesRead] = base_.ReadByte(false);
            bytesRead++;
            address++;

            base_.Stop();
        }

        return 0;

    }

    /** Get up to 4 bytes from the EEPROM and concatenate them into a single int.
     * 
     * This function is useful for retrieving an integer or a short stored in EEPROM. Bytes must be stored in little endian order in the EEPROM.
     * 
     * @param address the two byte address to read from.
     * @param length  the number of bytes to read.
     * @returns       the number read from the EEPROM. If length is < 4, then the upper bytes are set to 0.
     */
    int GetNumber(unsigned short address, int length) {
        char temp[4];
        Get(address, temp, length);
        int result = 0;

        /*
        // Commented out because of the compiler loop bug.
        for (int i = length - 1; i >= 0; --i) {
            //debug->Put(" Get_C ");
            result = (result << 8) | temp[i];
        }*/

        if (length >= 4) {
            result = (result << 8) | temp[3];
        }
        if (length >= 3) {
            result = (result << 8) | temp[2];
        }
        if (length >= 2) {
            result = (result << 8) | temp[1];
        }
        if (length >= 1) {
            result = (result << 8) | temp[0];
        }

        return result;
    }

private:

    bool PollForAcknowledge() {
        base_.Start();
        int counter = 0;
        while (base_.SendByte(kI2CAddress) == false) { //device EEPROM write
            if (++counter == 100) { //timeout
                return false;
            }
            base_.Stop();
            base_.Start();
        }
        return true;
    }

    I2CBase base_;
    static const unsigned char kI2CAddress = 0b10100000;

};

#endif // LIBPROPELLER_EEPROM_H_
