#ifndef HBTIME_H
#define HBTIME_H

#include <stdint.h>

struct TimeKeeper
{
    TimeKeeper();

    double get_delta_time_s();

    uint64_t perf_freq;
    uint64_t perf_cnt;
};

#endif // include_guard
