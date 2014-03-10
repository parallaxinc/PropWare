#ifndef LIBPROPELLER_SCHEDULER_H_
#define LIBPROPELLER_SCHEDULER_H_


#ifndef UNIT_TEST
#include <propeller.h>
#endif

#ifdef UNIT_TEST
#define CNT unit_CNT
#define CLKFREQ unit_CLKFREQ
extern unsigned volatile int unit_CNT;
extern unsigned volatile int unit_CLKFREQ;
#endif

/** A pollable timer to help schedule events.
 *
 * @warning: This class does not guarentee a minimum seperation between true 
 * @a Run() calls. Instead, it guarentees that, on average, a true Run() will 
 * happen with the specified frequency. Depending on how often Run() is called, 
 * there may be some jitter (if it is not called frequently enough).
 * 
 * @author SRLM
 */
class Scheduler {
public:

    /** Create a schedule keeper that returns true with the specified frequency.
     * 
     * The frequency is specified in units of 0.1Hz. So, to create a scheduler with
     * a frequency of 150Hz you pass in 1500. To make a scheduler with a frequency
     * of 0.5Hz (once every two seconds), you pass in 5 (0.1Hz * 5 = 0.5Hz). A
     * scheduler with a frequency of 1Hz takes a parameter of 10.
     *
     * @warning @a deci_hz must be 1 or more! Setting it less will result in a runtime
     *          error.
     * @param deci_hz The frequency (in 10x true return values per second).
     */
    Scheduler(const int deci_hz) {
        period_ticks_ = GetTicksPerPeriod(deci_hz);
        start_CNT_ = CNT;
    }

    /** Check to see if the time period has passed yet.
     *
     * This function is not blocking. This class keeps an internal state that
     * watches the system counter, and if the next "period" has begun returns true
     * the next time that this function is called. It then returns false for the
     * rest of the current period.
     *   
     * This function is useful for scheduling reseting watchdog timers,
     * determining when to poll external sensors, and so on.
     *
     * The function returns true for each time period, so if it is not called for
     * multiple time periods it returns true for each (up to hz times).
     *
     * @returns true if the time period has passed, false otherwise.
     */
    bool Run(void) {
        if ((CNT - start_CNT_) >= period_ticks_) {
            start_CNT_ += period_ticks_;
            return true;
        } else {
            return false;
        }
    }

    /** Calculates how many clock cycles are in each period.
     * 
     * For testing only!
     * 
     * @param deci_hz The frequency is deci-hz (hz * 10)
     * @return The number of clock cycles (ticks) per period.
     */
    static unsigned int GetTicksPerPeriod(const int deci_hz) {
        return (CLKFREQ * 10) / deci_hz;
    }

private:
    unsigned int next_read_time_;
    unsigned int start_CNT_;
    unsigned int period_ticks_;
};

#endif // LIBPROPELLER_SCHEDULER_H_
