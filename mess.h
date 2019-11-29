#ifndef MESS_H
#define MESS_H
#include <stdlib.h>
#include <string.h>

#define newT(T) (T *)(zalloc(sizeof(T)))

static void *zalloc(size_t size)
{
    void *p = malloc(size);
    memset(p, 0, size);
    return p;
}

#endif