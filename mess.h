#ifndef MESS_H
#define MESS_H
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define newT(T) (T *)(zalloc(sizeof(T)))

static void *zalloc(size_t size) {
  void *p = malloc(size);
  memset(p, 0, size);
  return p;
}

typedef struct {
  pthread_mutex_t key;
} mutex;

static void lock(mutex *m) { pthread_mutex_lock(&m->key); }
static void unlock(mutex *m) { pthread_mutex_unlock(&m->key); }

#endif