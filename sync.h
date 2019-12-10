#include "mess.h"
#include "runtime.h"
#include "proc.h"
typedef struct {
	// todo: use spinlock
	pthread_mutex_t lock;
	int count;
	void *wait;
} semaphore;

typedef struct {
	g *gp;
	// todo use queuq
	void *next;
} sudog;

void semaphore_init(semaphore *sem, int val);

void semaphore_down(semaphore *sem);
void semaphore_up(semaphore *sem);