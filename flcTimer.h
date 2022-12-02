// AoC 2021 - timer utility
// ========================

// date:  2021-12-04
// by:    Joseph21 (Joseph21-6147)

#ifndef FLCTIMER_H
#define FLCTIMER_H

#include <iostream>
#include <chrono>

class flcTimer {
public:
    // the constructer already stores the start frame time stamp
    flcTimer();
    ~flcTimer();

    void StartTiming();

    // each call times the end frame time stamp, and returns the difference
    // with the start frame time stamp. Also the start frame time stamp is set to
    // the time stamp just measured.
    // NOTE: time is returned in milli(!)seconds
    double TimeDuration();

    void TimeReport( std::string sMsg1, std::string sMsg2 = " msec" );

private:
    // timing stuff
    std::chrono::time_point<std::chrono::steady_clock> m_start_timing,  // to store start of timing period
                                                       m_stop_timing;   // to store end of timing period
};

#endif // FLCTIMER_H
