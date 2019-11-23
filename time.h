#ifndef TIME_H
#define TIME_H

#include "type.h"

int64 cputicks() {
    int64 val;
    __asm__ __volatile__("rdtsc" : "=A" (val));
    return val;
}




#endif