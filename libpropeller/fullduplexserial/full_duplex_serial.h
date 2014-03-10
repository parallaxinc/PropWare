#ifndef libpropeller_full_duplex_serial_h_
#define libpropeller_full_duplex_serial_h_


#include <propeller.h>
#include <string.h>

static inline int Rotl__(unsigned int a, unsigned int b) {
    return (a << b) | (a >> (32 - b));
}

static inline int Lookup__(int x, int b, int a[], int n) {
    int i = (x)-(b);
    return ((unsigned) i >= n) ? 0 : (a)[i];
}

extern char _load_start_full_duplex_serial_cog[];


//Warning: To change the buffer size you must change it here and in the .S file
#define buffersize 256
#define buffermask (buffersize - 1)

/** This driver, once started, implements a serial port in one cog.
 * 
 * Hardware:
 * To read 3.3v signals just connect the signal directly to any I/O pin. To read
 * a 5v signal connect it to any I/O pin through a 3.3kOhm (or higher) resistor.
 * 
 * Software:
 * The maximum standard baud rate on an 80MHz clock is 230400 baud.
 * 
 * 
 * 
 * This object is derived from FullDuplexSerial.spin v1.2.1 by Chip Gracey, Jeff
 * Martin, and Daniel Harris. Thanks!
 * 
 * Copyright (c) 2013 Kenneth Bedolla  (libpropeller@kennethbedolla.com)
 * 
 */

class FullDuplexSerial {
public:

    /** Start serial driver in new cog.
     * 
     * @param Rxpin Input to the Propeller
     * @param Txpin Output from the Propeller
     * @param Mode  Bitwise mode configuration variable:
     * - mode bit 0 = invert rx
     * - mode bit 1 = invert tx
     * - mode bit 2 = open-drain/source tx
     * - mode bit 3 = ignore tx echo on rx
     * @param Baudrate Any rate will work, up to the maximum baud
     */
    void Start(int Rxpin, int Txpin, int Mode, int Baudrate) {
        volatile void * asm_reference = NULL;
        __asm__ volatile ( "mov %[asm_reference], #FullDuplexSerial_Entry \n\t"
                : [asm_reference] "+r" (asm_reference));


        Stop();

        Rx_head = 0;
        Rx_tail = 0;
        Tx_head = 0;
        Tx_tail = 0;

        Rx_pin = Rxpin;
        Tx_pin = Txpin;
        Rxtx_mode = Mode;

        Bit_ticks = (CLKFREQ / Baudrate);
        Buffer_ptr = Rx_buffer;

        Cog = cognew(_load_start_full_duplex_serial_cog, &Rx_head) + 1;

    }

    /** Stop the serial driver (if running) and free a cog.
     */
    void Stop(void) {
        if (Cog > 0) {
            cogstop(Cog - 1);
            Cog = 0;
        }

        Rx_head = 0;
        Rx_tail = 0;
        Tx_head = 0;
        Tx_tail = 0;
        Rx_pin = 0;
        Tx_pin = 0;
        Rxtx_mode = 0;
        Bit_ticks = 0;
        Buffer_ptr = 0;
    }

    /** Clear the receive buffer of all bytes.
     * 
     * After calling this function the receive buffer has 0 bytes in it, and a
     * call to Get() without a timeout will wait until a byte is received.
     * 
     */
    void GetFlush(void) {
        while (CheckBuffer() >= 0) {
        }
    }

    /** Receive a byte of data.
     * 
     * If there are bytes in the buffer this function returns immediately.
     * Otherwise, it waits as specified by the timeout parameter.
     * 
     * @param timeout The maximum duration to wait for a byte in milliseconds.
     *                  A timeout of -1 indicates no wait.
     * @return -1 if a timeout, otherwise the byte received.
     */
    int Get(const int timeout = -1) {
        int Rxbyte = 0;
        if (timeout < 0) {
            while ((Rxbyte = CheckBuffer()) < 0) {
            }
        } else {
            int T = CNT;
            while (!(((Rxbyte = CheckBuffer()) >= 0)
                    || (((CNT - T) / (CLKFREQ / 1000)) > timeout))) {
            }
        }
        return Rxbyte;
    }

    /** Transmit a byte of data.
     * 
     * @param Txbyte the byte to send.
     */
    void Put(const char Txbyte) {
        while (!(Tx_tail != ((Tx_head + 1) & buffermask))) {
        }
        Tx_buffer[Tx_head] = Txbyte;
        Tx_head = ((Tx_head + 1) & buffermask);
        if (Rxtx_mode & 0x8) {
            Get();
        }
    }

    /** Transmit a C string.
     * 
     * Note: the null at the end is not transmitted.
     * 
     * @param string The null terminated string to transmit.
     */
    void Put(const char * string) {
        const int length = strlen(string);
        for (int i = 0; i < length; i++) {
            Put(string[i]);
        }

    }



    //The Dec, Hex, and Bin functions are left out in the hopes of getting a separate object to do the conversion.

    /** Transmit the decimal representation of a number.
     * 
     * @warning This function is provided only for short term convenience. SRLM
     * plans on taking it away sometime and to replace it with a proper
     * formatting object for the conversion.
     * 
     * @param Value the number whose value to print as a decimal
     * @return 
     */
    int PutDec(int Value) {
        int I;
        int result = 0;
        if (Value == (int) 0x80000000U) {
            //TODO(SRLM): take care of the MAX negative value (it can't be made positive...)
        }
        if (Value < 0) {
            Value = -Value;
            Put('-');
        }
        I = 1000000000;
        {
            int _idx__0043;
            for (_idx__0043 = 1; _idx__0043 <= 10; (_idx__0043 = (_idx__0043 + 1))) {
                if (Value >= I) {
                    Put((((Value / I) + '0')));
                    Value = (Value % I);
                    result = -1;
                } else {
                    if ((result) || (I == 1)) {
                        Put('0');
                    }
                }
                I = (I / 10);
            }
        }
        return result;
    }
private:

    void PutHex(int Value, int Digits) {
        static int look__0044[] = {48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 65, 66, 67, 68, 69, 70,};

        Value = (Value << ((8 - Digits) << 2));
        {
            int _idx__0045;
            int _limit__0046 = Digits;
            for (_idx__0045 = 1; _idx__0045 <= _limit__0046; (_idx__0045 = (_idx__0045 + 1))) {
                Put(Lookup__(((Value = (Rotl__(Value, 4))) & 0xf), 0, look__0044, 16));
            }
        }
    }

    void PutBin(int Value, int Digits) {
        Value = (Value << (32 - Digits));
        {
            int _idx__0047;
            int _limit__0048 = Digits;
            for (_idx__0047 = 1; _idx__0047 <= _limit__0048; (_idx__0047 = (_idx__0047 + 1))) {
                Put((((Value = (Rotl__(Value, 1))) & 0x1) + '0'));
            }
        }

    }

    int Cog;

    //Warning: the order of these variables is important! Do not change.
    volatile int Rx_head;
    volatile int Rx_tail;
    volatile int Tx_head;
    volatile int Tx_tail;
    volatile int Rx_pin;
    volatile int Tx_pin;
    volatile int Rxtx_mode;
    volatile int Bit_ticks;
    volatile char * Buffer_ptr;
    volatile char Rx_buffer[buffersize];
    volatile char Tx_buffer[buffersize];

    int CheckBuffer(void) {
        int Rxbyte = -1;
        if (Rx_tail != Rx_head) {
            Rxbyte = Rx_buffer[Rx_tail];
            Rx_tail = (Rx_tail + 1) & buffermask;
        }
        return Rxbyte;
    }
};

#endif // libpropeller_full_duplex_serial_h_
