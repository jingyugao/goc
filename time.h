#ifndef TIME_H
#define TIME_H

#include "type.h"
#include <time.h>

#define Nanosecond (long long)1
#define Microsecond (1000 * Nanosecond)
#define Millisecond (1000 * Microsecond)
#define Second (1000 * Millisecond)
#define Minute (60 * Second)
#define Hour (60 * Minute)

// int64 cputicks() {
//     int64 val;
//     __asm__ __volatile__("rdtsc" : "=A" (val));
//     return val;
// }

int64 nanotime();

#endif