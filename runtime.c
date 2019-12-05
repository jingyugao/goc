#include "runtime.h"
#include "os.h"
#include "proc.h"
#include "time2.h"
#include "timer.h"
#include <pthread.h>
#define _StackReserve (1 << 20)
#define _StackMin (2 << 10)

#define ALIGN(p, alignbytes)                                                   \
	((void *)(((unsigned long)(p) + (alignbytes)-1) & ~((alignbytes)-1)))

m *m0;
g *g0;
g *allgs[1024];

static pthread_key_t VirFSReg = 0; // pointer to m.tls
void settls(tls *ptr)
{
	int ret = pthread_setspecific(VirFSReg, ptr);
	if (ret != 0) {
		printf("pthread_setspecific error:%d\n", ret);
		abort();
	}
}

tls *gettls()
{
	return (tls *)pthread_getspecific(VirFSReg);
}

p *getP()
{
	return getg()->mp->p;
};

// ctx must be within g struct
void gogo(Context *ctx)
{
	g *gp = container_of(ctx, g, ctx);
	// g *gp = (g *)((void *)ctx - (void *)(&((g *)NULL)->ctx));
	tls *tls = gettls();
	tls->ptr[0] = (uintptr)gp;
	GetContext(ctx);
}

void SwitchTo(g *from, g *to)
{
	to->mp = from->mp;
	to->mp->curg = to;
	// printf("switch %d to %d\n", from->id, to->id);
	int ret = SaveContext(&from->ctx);
	if (ret == 0) {
		gogo(&to->ctx);
	}
}

static int gid = 1;
int allocGID()
{
	gid++;
	return gid;
}

void allgadd(g *gp)
{
	for (int i = 0; i < 1024; i++) {
		if (allgs[i] == NULL) {
			allgs[i] = gp;
			return;
		}
	}
	assert(0);
	return;
}

g *malg()
{
	int stackSize = _StackReserve;
	uintptr stackTop;

	int ret = posix_memalign((void **)&stackTop, stackSize, stackSize);
	if (ret != 0) {
		return NULL;
	}
	uintptr stackBase = stackTop + stackSize;
	g *c = newT(g);
	c->stack.lo = stackTop;
	// align
	stackBase = (uintptr)ALIGN(stackBase, 16);
	stackBase = stackBase - 8;
	*(uintptr *)stackBase = (uintptr)goexit;

	c->stack.hi = stackBase;
	c->ctx.reg.rsp = stackBase;

	return c;
}

void runqput(p *p, g *g)
{
	if (g->id == 0) {
		abort();
	}
	lock(&p->mu);
	int h = p->runqhead;
	int t = p->runqtail;

	int size = (sizeof(p->runq) / sizeof(p->runq[0]));
	if (t - h < size) {
		p->runq[t % (size)] = g;
		p->runqtail = t + 1;
	}
	unlock(&p->mu);

	// put to global runq
};

g *runqget(p *p)
{
	lock(&p->mu);
	if (p->runqhead == p->runqtail) {
		unlock(&p->mu);
		return NULL;
	}
	int size = (sizeof(p->runq) / sizeof(p->runq[0]));
	g *c = p->runq[p->runqhead % size];
	p->runqhead++;
	unlock(&p->mu);
	return c;
}

void casgstatus(g *gp, uint32 oldval, uint32 newval)
{
	gp->atomicstatus = newval;
}

void mcall(void (*f)(g *))
{
	f(getg());
}

void goexit0(g *gp)
{
	casgstatus(gp, gp->atomicstatus, _Gdead);
	// todo schedule
	gogo(&gp->mp->g0->ctx);
}

void goexit1()
{
	mcall(goexit0);
}

void goexit()
{
	printf("g %d exit\n", getg()->id);
	goexit1();
}

void Gosched()
{
	g *curg = getg();
	p *p = getP();
	runqput(p, curg);
	casgstatus(curg, curg->atomicstatus, _Grunnable);
	SwitchTo(curg, curg->mp->g0);
	return;
};

g *getg()
{
	return ((g *)gettls()->ptr[0]);
}

void systemstack(Func fn)
{
	printf("systemstack\n");
	g0->fn = fn;
	int ret = SaveContext(&g0->ctx);
	printf("ret:%d\n", ret);
	if (ret == 0) {
		((void (*)(uintptr))(g0->fn.f))(g0->fn.arg);
		GetContext(&g0->ctx);
	}
}

g *newproc1(Func fn)
{
	g *newg = malg();
	allgadd(newg);
	int gid = allocGID();
	newg->id = gid;
	newg->fn = fn;
	newg->ctx.reg.rdi = (uintptr)newg->fn.arg;
	newg->ctx.reg.pc_addr = (uintptr)newg->fn.f;
	casgstatus(newg, newg->atomicstatus, _Grunnable);
	runqput(getP(), newg);
	return newg;
}

void newproc(uintptr f, uintptr arg)
{
	Func fn;
	fn.f = f;
	fn.arg = arg;
	newproc1(fn);
	return;
}

void schedinit()
{
	printf("schedinit\n");
	g *_g_ = getg();
	for (int i = 0; i < MAXPORC; i++) {
		allp[i] = newT(p);
		memset(allp[i], 0, sizeof(p));
		allp[i]->id = i;
	}

	_g_->mp->p = allp[0];
	allp[0]->mp = _g_->mp;
}

int main_main();

g *runqsteal(p *_p_, p *p2, bool stealRunNextG)
{
	if (p2 == NULL || p2 == _p_) {
		return NULL;
	}
	g *nextg = runqget(p2);
	if (nextg != NULL) {
		return nextg;
	}
	return NULL;
}

void check_timers(p *pp, int64 ns);

g *findRunnable()
{
	p *_p_ = getg()->mp->p;
	check_timers(_p_, 0);
	g *nextg = runqget(_p_);
	if (nextg == NULL) {
		for (int i = 0; i < MAXPORC; i++) {
			nextg = runqsteal(_p_, allp[i], true);
			if (nextg != NULL) {
				printf("%d steal %d from %d\n", _p_->id,
				       nextg->id, allp[i]->id);
				break;
			}
		}
	}
	if (nextg == NULL) {
		return NULL;
	}
	return nextg;
}
void call_fn(Func fn)
{
	((void (*)(uintptr))(fn.f))(fn.arg);
}
void check_timers(p *pp, int64 ns)
{
	pthread_mutex_lock(&pp->timerslock);
	ns = nanotime();
	while (1) {
		if (vector_count(&pp->timers) == 0) {
			break;
		}
		timer *t0 = vector_get(&pp->timers, 0);
		if (t0->when > ns) {
			break;
		}
		call_fn(t0->fn);
		pop_timers(&pp->timers);
	}

	pthread_mutex_unlock(&pp->timerslock);
}

// must on g0
void schedule()
{
	printf("schedule\n");
	while (1) {
		g *nextg = findRunnable();
		if (nextg == NULL) {
			usleep(200 * 1000);
			printf("p%lld no co to run:\n", getg()->mp->id);
			continue;
		}
		SwitchTo(getg(), nextg);
	}

	exit(0);
}

void wakeg(g *gp)
{
	p *_p_ = getg()->mp->p;
	runqput(_p_, gp);
}

void timeSleep(int64 ns)
{
	if (ns <= 0) {
		return;
	}

	g *gp = getg();
	gp->when = nanotime() + ns;
	casgstatus(gp, gp->atomicstatus, _Gwaiting);
	timer *t = newT(timer);
	t->when = nanotime() + ns;
	t->fn.arg = (uintptr)gp;
	t->fn.f = (uintptr)wakeg;
	p *_p_ = gp->mp->p;
	push_timers(&_p_->timers, t);
	SwitchTo(gp, gp->mp->g0);
}

void mstart1()
{
	g *_g_ = getg();
	Func fn = _g_->mp->mstartfn;
	if (fn.f != 0) {
		((void (*)(uintptr))(fn.f))(fn.arg);
	}

	schedule();
}

void mstart()
{
	mstart1();
	pthread_exit(0);
}

int main();
// really main
int rt0_go()
{
	printf("asm main\n");
	memset(allgs, 0, 1024 * sizeof(uintptr));
	int ret = pthread_key_create(&VirFSReg, NULL);
	if (ret != 0) {
		printf("pthread_key_create error:%d\n", ret);
		abort();
	}
	// init g0
	Func fg0;
	fg0.f = (uintptr)schedule;
	g0 = malg();
	m *m0 = newT(m);
	settls(&m0->tls);
	m0->tls.ptr[0] = (uintptr)g0;
	g0->mp = m0;
	m0->g0 = g0;
	m0->curg = g0;
	allgs[0] = g0;
	g0->id = 0;
	g0->fn = fg0;
	g0->ctx.reg.pc_addr = (long)g0->fn.f;

	schedinit();
	newproc((uintptr)main, 0);
	wakep();
	newm((uintptr)sysmon, 0);
	// sleep(1000);
	mstart();

	abort();
	return 0;
}

int main()
{
	if (!g0) {
		printf("main must called after rt0_go\n");
		abort();
	}
	main_main();

	exit(0);
	return 0;
}
