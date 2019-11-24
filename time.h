#ifndef TIME_H
#define TIME_H

#include "type.h"
#include <time.h>
// int64 cputicks() {
//     int64 val;
//     __asm__ __volatile__("rdtsc" : "=A" (val));
//     return val;
// }

int64 nanotime() {
  struct timespec ts;
  int ret = clock_gettime(CLOCK_MONOTONIC, &ts);
  if (ret == 0) {
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
  }
  *(int64 *)(NULL);
  return 0;
}

#endif