#ifndef libpropeller_quadrature_encoder_h__
#define libpropeller_quadrature_encoder_h__

#include <propeller.h>

extern char _load_start_quadrature_encoder_cog[];

/** 
 * This object stores encoder ticks in a signed 4 byte int sized 
 * variable. This means that there is some chance of overflow the encoder is
 * used in a single direction continuously. To calculate the maximum distance in
 * a single direction use the following equation:
 * 
 * maxDistanceInMiles = (2^31)* wheelDiameterInFeet * pi / ticksPerWheelRevolution /5280
 * 
 * For a robot with 6" diameter wheels and 144 ticks per wheel revolution the
 * maximum distance in a single direction is 4,436 mi.
 * 
 * However, for a robot with 6" diameter wheels, a 2048 tick encoder on the
 * motor shaft and a 100:1 gearbox, the maximum distance is 3.1 miles.
 * 
 * Therefore, you should chose your encoder, gearbox, and wheel combination to
 * match the precision that you need (distance per tick) while minimize the
 * total ticks per revolution.
 * 
 * Copyright (c) 2013 Kenneth Bedolla  (libpropeller@kennethbedolla.com)
 */

class QuadratureEncoder {
public:

    /** Start a new cog with the encoder monitor.
     * 
     * @param basePin          First pin of encoder 1. Second pin is basePin+1.
     *                          Additional pins for other encoders are
     *                          contiguous starting with basePin+2.
     * @param numberOfEncoders Number of encoders [1..16] to monitor.
     * @return                 True if a cog is successfully started, false
     *                          otherwise.
     */
    bool Start(const int basePin, const int numberOfEncoders) {
        volatile void * asm_reference = NULL;
        __asm__ volatile ( "mov %[asm_reference], #QuadratureEncoder_Entry \n\t"
                : [asm_reference] "+r" (asm_reference));

        Stop();

        this->numberOfEncoders = numberOfEncoders;
        this->basePin = basePin;
        cog = cognew(_load_start_quadrature_encoder_cog, &this->basePin) + 1;

        return cog != 0;
    }

    void Stop(void) {
        if (cog > 0) {
            cogstop(cog - 1);
        }
    }

    int GetReading(const int index) {
        return readings[index];
    }

private:
    // Don't reorder these three volatile variables! They are used by the GAS cog.
    volatile int basePin, numberOfEncoders, readings[16];

    int cog;
};


/*
 // This function is untested!
int32_t QuadratureEncoder::Readdelta(int32_t Encid)
{
  int32_t Deltapos = 0;
  Deltapos = ((0 + ((Encid < Totdelta) * (-((int32_t *)Pos)[((*(int32_t *)&dat[196]) + Encid)]))) + (((int32_t *)Pos)[((*(int32_t *)&dat[196]) + Encid)] = ((int32_t *)Pos)[Encid]));
  return Deltapos;
}
 */

#endif // libpropeller_quadrature_encoder_h__
