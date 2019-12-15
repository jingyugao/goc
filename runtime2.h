#ifndef RUNTIME2_H
#define RUNTIME2_H

#include <pthread.h>
#include <stdatomic.h>
#include "context.h"
#include "mess.h"

#ifndef MAXPROC
#define MAXPROC (16)
#endif

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
	_Atomic uint32 atomicstatus;
	bool preempt;
	int id;
	Context ctx;
	Func fn;
	Stack stack;
	int64 when;
	struct m *m;

	bool is_g0;
} g;

typedef struct p {
	int id;
	struct m *m;
	struct p *link;
	// m protect runq*
	pthread_mutex_t mu;
	int runqhead;
	int runqtail;
	g *runq[256];

	int64 sched_when;

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

	void *waitlock;
	bool (*waitunlockf)(g *, void *);
} m;

typedef struct {
	pthread_mutex_t lock;
	_Atomic int npidle;
	p *pidle;
	g *gfree;

	_Atomic int preempt_enable;
} schedt;
extern schedt sched;
extern m *allm;
extern p *allp[MAXPROC];

#endif