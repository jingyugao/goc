#ifndef TIMER_H
#define TIMER_H
#include "runtime.h"
#include "time.h"

typedef struct {
  Func fn;
  int64 when;
} timer;

typedef struct {
  g *gp;
  bool created;
  bool sleeping;
  bool rescheduling;
  int64 sleepUntil;
  timer *t;
  int num;
} timersBucket;

extern timersBucket timers[1];

#endif