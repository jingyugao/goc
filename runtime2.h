#ifndef RUNTIME2_H
#define RUNTIME2_H

#include <pthread.h>

#include "context.h"
#include "mess.h"
#define MAXPORC (4)

typedef struct {
	uintptr f;
	uintptr arg;
} Func;

struct m;

typedef struct {
	uintptr lo;
	uintptr hi;
} Stack;

typedef struct {
	uint32 atomicstatus;
	int id;
	Context ctx;
	Func fn;
	Stack stack;
	int64 when;
	struct m *mp;
} g;

typedef struct p {
	int id;
	struct m *mp;
	struct p *link;
	// m protect runq*
	pthread_mutex_t mu;
	int runqhead;
	int runqtail;
	g *runq[256];

	g *curg;

	vector timers;
	pthread_mutex_t timerslock;

} p;

typedef struct {
	uintptr ptr[6];
} tls;

typedef struct m {
	Func mstartfn;
	g *g0;
	tls tls;
	g *curg;
	p *p;
	pthread_t thread;
} m;

typedef struct {
	pthread_mutex_t lock;
	_Atomic int npidle;
	p *pidle;
} schedt;
extern schedt sched;
extern m *allm;
extern p *allp[MAXPORC];

#endif