#ifndef RUNTIME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "context.h"


typedef struct
{
    void *fn;
} Func;

typedef struct
{
    void *args;
} Args;

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
    Args args;
    Stack stack;
} g;

extern g *allCo[1024];

g *getg();



#endif