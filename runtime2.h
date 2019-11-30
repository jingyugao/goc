#ifndef RUNTIME2_H
#define RUNTIME2_H

#include <pthread.h>

#include "context.h"
#include "mess.h"
#define MAXPORC (4)

typedef struct {
  uintptr f;
  uintptr arg;
} Func;

struct m;

typedef struct {
  uintptr lo;
  uintptr hi;
} Stack;

typedef struct {
  uint32 atomicstatus;
  int id;
  Context ctx;
  Func fn;
  Stack stack;
  int64 when;
  struct m *mp;
} g;

typedef struct p {
  int id;
  struct m *mp;

  // m protect runq*
  mutex mu;
  int runqhead;
  int runqtail;
  g *runq[256];

  g *curg;
} p;

typedef struct {
  uintptr ptr[6];
} tls;

typedef struct m {
  Func mstartfn;
  g *g0;
  tls tls;
  g *curg;
  p *p;
  int64 id;
  pthread_t thread;
} m;

extern m *allm;
extern p *allp[MAXPORC];
extern pthread_mutex_t allpLock;

#endif