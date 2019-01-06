#ifndef HBTIME_H
#define HBTIME_H

#include <stdint.h>

struct TimeKeeper
{
    TimeKeeper();

    double get_delta_time_s();
    double get_delta_time_s_no_reset();

    uint64_t perf_freq;
    uint64_t perf_cnt;
};

#ifdef FAST_BUILD
#include "time.cpp"
#endif

#endif // include_guard
