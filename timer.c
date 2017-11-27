#include "timer.h"

#include <stdio.h>

#ifdef _WIN32
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>

typedef LARGE_INTEGER app_timer_t;

#define timer(t_ptr) QueryPerformanceCounter(t_ptr)

void elapsed_time(app_timer_t start, app_timer_t stop)
{
    double etime;
    LARGE_INTEGER clk_freq;
    QueryPerformanceFrequency(&clk_freq);
    etime = (stop.QuadPart - start.QuadPart) /
            (double)clk_freq.QuadPart;
    printf("CPU (total!) time = %.3f ms )\n",
           etime * 1e3);
}

#else

#include <time.h> /* requires linking with rt library
(command line option -lrt) */

typedef struct timespec app_timer_t;

#define timer(t_ptr) clock_gettime(CLOCK_MONOTONIC, t_ptr)

void elapsed_time(app_timer_t start, app_timer_t stop)
{
    double etime;
    etime = 1e+3 * (stop.tv_sec - start.tv_sec) +
            1e-6 * (stop.tv_nsec - start.tv_nsec);
    printf("CPU (total!) time = %.3f ms\n", etime);
}

#endif
