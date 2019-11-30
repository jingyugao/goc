#include "mess.h"
#include "time2.h"
#include <time.h>
int64 nanotime() {
  struct timespec ts;
  int ret = clock_gettime(CLOCK_MONOTONIC, &ts);
  if (ret == 0) {
    return ts.tv_sec * 1000000000 + ts.tv_nsec;
  }
  panicf("nanotime err:%d", ret);
  return 0;
}

int64 cputicks() {
  int64 val;
  __asm__ __volatile__("rdtsc" : "=A"(val));
  return val;
}
