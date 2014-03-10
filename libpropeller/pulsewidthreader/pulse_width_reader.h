#ifndef libpropeller_readpulsewidths_h__
#define libpropeller_readpulsewidths_h__
#include <propeller.h>

extern char _load_start_pulsewidthreader_cog[];

/** Record high and low times simultaniously on any number of pins.
 *  
 * Based on ReadPulseWidths.spin v1.0 by David Carrier. Thanks!
 * 
 * 
 * Hardware:
 * To read 3.3v signals just connect the signal directly to any I/O pin. To read
 * a 5v signal connect it to any I/O pin through a 3.3kOhm (or higher) resistor.
 * To read a higher voltage, use the following equation to determine the proper
 * series voltage:
 * resistance = (voltage - 3.3v - 0.6v) / 500uA
 * 
 * 
 * Example:
 * To use this object, first construct a mask of the pins you are interested in
 * monitoring (in this case pins 2, 10, and 21):
 * mask = (1 << 2) | (1 << 10) | (1 << 21)
 * Then start it:
 * PulseWidthReader pwr(); pwr.Start(mask); 
 *
 * When you want the data simply call:
 * pwr.GetHighTime(2);
 * 
 * The index is the index of the monitored pin out of the count total. In this
 * example the index is:
 * Index | pin
 *      0|   2
 *      1|  10
 *      2|  21
 * 
 * Copyright (c) 2013 Kenneth Bedolla  (libpropeller@kennethbedolla.com)
 * 
 * @todo(SRLM): Make members of this class static (since it monitors all the pins...)
 * @todo(SRLM): Get rid of the terrible "index" system and create a more
 *              intuitive start.
 * @todo(SRLM): Are there any restrictions? Maximum time? Minimum pulse width? etc.
 * @todo(SRLM): This class seems to have intermittent test failures when run for the first time. A bug?
 */
class PulseWidthReader {
public:

    PulseWidthReader() {
        Cog = 0;
    }

    /** Launch a new cog to watch and record pulse times.
     * 
     * @param Inputmask The mask of pins to watch.
     */
    void Start(unsigned int Inputmask) {
        volatile void * asm_reference = NULL;
        __asm__ volatile ("mov %[asm_reference], #PulseWidthReader_Entry \n\t"
                : [asm_reference] "+r" (asm_reference));
        Stop();

        pinTimes[0] = Inputmask;
        Cog = cognew(_load_start_pulsewidthreader_cog, pinTimes) + 1;
    }

    /** Free a cog if running.
     */
    void Stop(void) {
        if (Cog != 0) {
            cogstop(Cog - 1);
            Cog = 0;
        }
    }

    /** Get the most recent high pulse duration. Index starts at 0.
     * 
     * @param index The index (from low to high) of the desired pin (out of all 
     *          the monitored pins)
     * @return The number of clock cycles of the last high pulse.
     */
    int getHighTime(int index){
        return pinTimes[index * 2];
    }

    /** Get the most recent low pulse duration. Index starts at 0.
     * 
     * @param index The index (from low to high) of the desired pin (out of all 
     *          the monitored pins)
     * @return The number of clock cycles of the last low pulse.
     */
    int getLowTime(int index){
        return pinTimes[index * 2 + 1];
    }

private:
    int Cog;
    volatile int pinTimes[64];
    

};

#endif // libpropeller_readpulsewidths_h__
