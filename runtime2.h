#include <pthread.h>


#include "runtime.h"


typedef struct
{
    g *g0;
    uintptr tls[6];
    g *curg;
    p* p;
    int64 id;

    pthread_t thread;
} m;