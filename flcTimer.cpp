// AoC 2021 - timer utility
// ========================

// date:  2021-12-04
// by:    Joseph21 (Joseph21-6147)

#include "flcTimer.h"

// ==============================/ Class FrameTimer /==============================

// see: https://www.techiedelight.com/measure-elapsed-time-program-chrono-library/

// the constructer already stores the start frame time stamp
flcTimer::flcTimer() {
    StartTiming();
}

// default destructor
flcTimer::~flcTimer() {}

// Function to give the start timing a timestamp. Is called by constructor.
void flcTimer::StartTiming() { m_start_timing = std::chrono::steady_clock::now(); }

// Each call to this function times the end frame time stamp, and returns the difference with the
// start frame time stamp in microseconds (!!). Also the start frame time stamp is set to
// the time stamp just measured.
double flcTimer::TimeDuration() {
    // get current time stamp
    m_stop_timing  = std::chrono::steady_clock::now();
    // calculate the time elapsed between last start and stop timings
//    int nMilliSeconds = std::chrono::duration_cast<std::chrono::microseconds>(m_stop_timing - m_start_timing).count();
    double nMilliSeconds = std::chrono::duration<double>(m_stop_timing - m_start_timing).count() * 1000.0;
    m_start_timing = m_stop_timing;

    return nMilliSeconds;
}

void flcTimer::TimeReport( std::string sMsg1, std::string sMsg2 ) {
    std::cout << sMsg1 << TimeDuration() << sMsg2 << std::endl;
}
