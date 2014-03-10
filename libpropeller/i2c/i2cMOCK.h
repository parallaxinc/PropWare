#ifndef SRLM_PROPGCC_I2C_MOCK_H__
#define SRLM_PROPGCC_I2C_MOCK_H__

#include <stdio.h>

#ifdef UNIT_TEST

/*

Notes:
- This is a MOCK object to allow for unit testing of the L3GD20 object.

 */
class I2C {
public:


    //  static const int kAck = 1;
    //  static const int kNak = 0;

    char putStack[100];
    int putStackTop;


    char xyz[6];

    void Init(const int SCLPin, const int SDAPin) {
        putStackTop = -1;
    }

    bool Ping(unsigned char device) {
        return true;
    }

    bool Put(unsigned char device, unsigned char address, char byte) {
        putStack[++putStackTop] = byte;
        return true;
    }

    //TODO(SRLM): For some reason, this function locks up in the for loop when optimized.
#pragma GCC optimize ("0")

    bool Get(unsigned char device, unsigned char address, char * bytes, int size) {
        if (address == (0x28 | 0x80)) //OUT_X_L with autoincrement bit set
        {
            for (volatile int i = 0; i < size; ++i) {
                bytes[i] = xyz[i];
                //			printf("\nbytes[%i] = 0x%X  ", i, bytes[i]);
            }
        } else if (address == (0x03 | 0x80)) //OUT_X_L with autoincrement bit set
        {
            for (volatile int i = 0; i < size; ++i) {
                bytes[i] = xyz[i];
                //			printf("\nbytes[%i] = 0x%X  ", i, bytes[i]);
            }
        }
        else {
            return false;
        }

        return true;

    }

    // -----------------------------------------------------------------------------

    int GetPutStack() {
        if (putStackTop == -1) return -1;
        else return putStack[putStackTop--];
    }

    void SetXYZ(char * bytes, int size) {
        for (int i = 0; i < size; i++) {
            xyz[i] = bytes[i];
            //		printf("\nxyz[%i] = 0x%X  ", i, xyz[i]);
        }
    }
};

#endif // Unit_Test

#endif // SRLM_PROPGCC_I2C_MOCK_H__
