#ifndef LIBPROPELLER_MCP3208_H_
#define LIBPROPELLER_MCP3208_H_

#include <propeller.h>

extern char _load_start_mcp3208_cog[];

/** MCP3208 8 channel ADC Class.
 * 
 * When the MCP3208 is configured for 3.3v operation each bit represents 0.00080566406v.
 * 
 * The MCP3208 requires three connections to the Propeller: clock, chip select,
 * and data (tie together Dout and Din).
 * 
 * This object optionally provides two DACs as well. To enable these you need to
 * have one or two Propeller I/O pins available. Each DAC I/O pin needs to have
 * a low pass RC filter. I reccommend R = 1k and C = 0.1uF. With this
 * configuration you should avoid changing the DAC more than once a millisecond
 * (the cut off frequency is 1591Hz).
 * 
 * This driver does continuous sampling in the background.
 * 
 * This object is based on MCP3208.spin v1.0 written by Chip Gracey. Thanks!
 * 
 * Copyright (c) 2013 Kenneth Bedolla  (libpropeller@kennethbedolla.com)
 */
class MCP3208 {
public:
    

    /** Start the ADC driver in a new cog.
     * 
     * Be sure to give the driver sufficient time to start up.
     * 
     * @param dataPin The data in and data out pins on the MCP3208
     * @param clockPin The clock pin on the MCP3208
     * @param selectPin The select pin on the MCP3208
     * @param mode Channel enables in bits [0..7], differential mode enable in
     *                  bits [8..15]. Most applications should set this to 0xFF
     * @param dacAPin If desired, the pin number for a DAC (see discussion above)
     * @param dacBPin If desired, the pin number for a DAC (see discussion above)
     */
    void Start(const int dataPin, const int clockPin,
            const int selectPin, const int mode = 0xFF,
            const int dacAPin = -1, const int dacBPin = -1) {

        if (dacAPin == -1 && dacBPin == -1) {
            //No DACs
            Startx(dataPin, clockPin, selectPin, mode, 0);
        } else if (dacAPin != -1 && dacBPin == -1) {
            //One DAC
            Startx(dataPin, clockPin, selectPin, mode, (dacAPin & 0x1f) | 0x80);
        } else {
            //Two DACs
            Startx(dataPin, clockPin, selectPin, mode,
                    (((dacBPin & 0x1f) | 0x80) << 8) + ((dacAPin & 0x1f) | 0x80));
        }
    }

    /** Stop the driver and free a cog.
     */
    void Stop(void) {
        if (Cog > 0) {
            cogstop(Cog - 1);
            Cog = 0;
        }
    }

    /** Read a channel input.
     * 
     * @param Channel The channel [0..7] to read
     * @return The ADC reading in the range [0..4096]
     */
    int In(const int Channel) {

        // Get pull the 16 bit word out of the 32 byte word.
        int result = Ins[Channel / 2];

        // If index is odd then pull the upper word, if even pull the lower word.
        result = Channel & 0x1 ? result >> 16 : result;

        // Make sure that we get just what we are interested in.
        return result & 0xFFFF;
    }

    /** Sample a channel n times and average the results.
     * 
     * This function is useful for reducing noisy readings.
     * 
     * @param Channel The channel [0..7] to read
     * @param N The number of samples to average
     */
    int Average(const int Channel, const int N) {
        //TODO(SRLM): What is C and Count?
        int sampleTotal = 0;
        int C = Count;
        for (int i = 0; i < N; i++) {
            while (C == Count) {
            }
            sampleTotal += In(Channel);
            C++;

        }

        return sampleTotal / N;
    }

    /** Output analog values (if enabled during Start)
     * 
     * The range is 0 (ground) to 65535 (3.3v) for the output.
     * 
     * @param aOutput The output value for channel a
     * @param bOutput The output value for channel b
     */
    void Out(const short aOutput, const short bOutput = -1) {
        Dacx = aOutput << 16;
        Dacy = bOutput << 16;
    }
private:
    int Cog;

    //Warning: do not rearrange these variables. The assembly relies on the order.
    int volatile Ins[4];
    int volatile Count;
    int volatile Dacx, Dacy;

    void Startx(const int dataPin, const int clockPin,
            const int selectPin, const int mode, const int Dacmode) {

        volatile void * asm_reference = NULL;
        __asm__ volatile ("mov %[asm_reference], #MCP3208_Entry\n\t"
                : [asm_reference] "+r" (asm_reference));

        Stop();

        Ins[0] = dataPin;
        Ins[1] = clockPin;
        Ins[2] = selectPin;
        Ins[3] = mode;

        Count = Dacmode;

        Cog = cognew(_load_start_mcp3208_cog, Ins) + 1;
    }
};

#endif // LIBPROPELLER_MCP3208_H_
