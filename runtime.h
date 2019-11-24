#ifndef RUNTIME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "context.h"
#include "type.h"

#define _Gidle     (0）
#define _Grunnable (1)
#define _Grunning (2)
#define _Gsyscall (3)
#define _Gwaiting (4)
#define _Gdead (6)

typedef struct
{
    void (*f)(void *);
    void *arg;
} Func;

typedef struct
{
    uintptr lo;
    uintptr hi;
} Stack;

typedef struct
{
    uint32 atomicstatus;
    int id;
    Context ctx;
    Func fn;
    Stack stack;
    int64 when;
} g;

typedef struct
{
    g *g0;
    int runqhead;
    int runqtail;
    g *runq[256];
    g *curg;
} p;

extern g *allgs[1024];

g *getg();

void gsleep(int64 sec);

#endif