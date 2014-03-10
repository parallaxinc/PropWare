#ifndef LIBPROPELLER_STOPWATCH_H_
#define LIBPROPELLER_STOPWATCH_H_


#ifndef UNIT_TEST
#include <propeller.h>
#endif

// Yes, there is an compile warning here for redefine...
// It's because Unity includes propeller.h, which has CNT and CLKFREQ defined.
#ifdef UNIT_TEST
#define CNT unit_CNT
#define CLKFREQ unit_CLKFREQ
extern unsigned int unit_CNT;
extern unsigned int unit_CLKFREQ;
#endif

/** Utility class for timing event duration.
 * 
 * @warning The maximum time that can be recorded is (2^32/CLKFREQ) seconds. At
 * 80Mhz that is a bit over 53 seconds. Longer durations will rollover and make
 * the stopwatch operate incorrectly.
 * 
 * @author srlm (srlm@srlmproductions.com)
 * 
 */
class Stopwatch {
public:

    /**
     * 
     */
    Stopwatch() {
        Start(); //This call suppresses a warning.
        Reset();
    }

    /** Stop timing.
     */
    void Reset(void) {
        started_ = false;
    }

    /** Start timing. Can be called without calling reset first.
     */
    void Start(void) {
        start_CNT_ = CNT;
        started_ = true;
    }

    /** Get current stopwatch time (when started).
     * 
     * @returns the number of elapsed milliseconds since start.
     */
    int GetElapsed(void) const {
        if (started_ == true) {
            return (CNT - start_CNT_) / (CLKFREQ / 1000);
        } else {
            return 0;
        }
    }

    /** Get started state.
     * 
     * @return  true if started, false otherwise.
     */
    bool GetStarted(void) const {
        return started_;
    }

private:
    unsigned int start_CNT_;
    bool started_;

};




#endif // LIBPROPELLER_STOPWATCH_H_
