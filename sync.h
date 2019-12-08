#include "mess.h"
#include "runtime.h"
#include "proc.h"
typedef struct {
	// todo: use spinlock
	pthread_mutex_t lock;
	int count;
	void *wait;
} semaphore;

static void semaphore_init(semaphore *sem, int val)
{
	pthread_mutex_init(&sem->lock, NULL);
	sem->count = val;
	sem->wait = NULL;
}

typedef struct {
	g *gp;
	// todo use queuq
	void *next;
} sudog;

static void semaphore_down(semaphore *sem)
{
	pthread_mutex_lock(&sem->lock);
	sem->count--;
	if (sem->count >= 0) {
		pthread_mutex_unlock(&sem->lock);
		return;
	}
	g *gp = getg();
	sudog *sg = newT(sudog);
	sg->gp = gp;
	sg->next = sem->wait;
	sem->wait = sg;
	goparkunlock(&sem->lock, 1);
}

static void semaphore_up(semaphore *sem)
{
	pthread_mutex_lock(&sem->lock);
	sem->count++;
	sudog *sg = sem->wait;
	if (sg) {
		sem->wait = sg->next;
	}
	pthread_mutex_unlock(&sem->lock);
	if (sg != NULL) {
		goready(sg->gp);
	}
}
