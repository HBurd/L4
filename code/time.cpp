#include "hb/time.h"
#include "SDL/SDL.h"

TimeKeeper::TimeKeeper()
{
    perf_freq = SDL_GetPerformanceFrequency();
    perf_cnt = SDL_GetPerformanceCounter();
}

double TimeKeeper::get_delta_time_s_no_reset()
{
    uint64_t perf_cnt_new = SDL_GetPerformanceCounter();
    uint64_t delta_perf_cnt = perf_cnt_new - perf_cnt;
    return (double)delta_perf_cnt / perf_freq;
}

double TimeKeeper::get_delta_time_s()
{
    uint64_t perf_cnt_new = SDL_GetPerformanceCounter();
    uint64_t delta_perf_cnt = perf_cnt_new - perf_cnt;
    double delta_t = (double)delta_perf_cnt / perf_freq;

    perf_cnt = perf_cnt_new;

    return delta_t;
}
