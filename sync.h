#include "mess.h"
#include "runtime.h"
#include <pthread.h>
typedef struct {
  pthread_mutex_t lock;
  int count;
  listhead wait;
} semaphore;

static void semaphore_init(semaphore *sem, int val) {
  sem->lock = PTHREAD_MUTEX_INITIALIZER;
  sem->count = val;
  list_head_init(&sem->wait);
}

typedef struct {
  g *gp;
  listhead list;
} sudog;

static void semaphore_down(semaphore *sem) {
  pthread_mutex_lock(&sem->lock);
  if (sem->count >= 1) {
    sem->count--;
    pthread_mutex_unlock(&sem->lock);
    return;
  }
  g *gp = getg();
  sudog *sg = newT(sudog);
  sg->gp = gp;
  list_head_init(&sg->list);
  list_add_tail(&sg->list, &sem->wait);
  pthread_mutex_unlock(&sem->lock);
  
  
  
}
