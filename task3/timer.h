

#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>

#define GET_TIME(now)                        \
    do {                                     \
        struct timeval tv;                   \
        gettimeofday(&tv, NULL);             \
        now = tv.tv_sec + tv.tv_usec*1.0e-6; \
    } while (0)

#endif /* TIMER_H */
