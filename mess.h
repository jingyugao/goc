#ifndef MESS_H
#define MESS_H
#include "type.h"
#include <execinfo.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static void *zalloc(size_t size) {
  void *p = malloc(size);
  memset(p, 0, size);
  return p;
}

#define newT(T) (T *)(zalloc(sizeof(T)))

typedef struct {
  pthread_mutex_t key;
} mutex;

static void lock(mutex *m) { pthread_mutex_lock(&m->key); }
static void unlock(mutex *m) { pthread_mutex_unlock(&m->key); }

static void panic(error err) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);
  printf("Error: %d, %s\n", err.code, err.str);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

#define container_of(ptr, type, member)                                        \
  (type *)((void *)ptr - (void *)(&((type *)(NULL))->member))
#endif