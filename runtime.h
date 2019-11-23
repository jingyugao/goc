#ifndef RUNTIME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "context.h"


typedef struct
{
    void (*f)(void*);
    void *arg;
} Func;


typedef struct
{
    uintptr lo;
    uintptr hi;
} Stack;

typedef struct
{
    int id;
    Context ctx;
    Func fn;
    Stack stack;
} g;

typedef struct {
  g *g0;
  int runqhead;
  int runqtail;
  g *runq[256];
} p;


extern g *allgs[1024];

g *getg();



#endif