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

void call_fn(Func fn)
{
	((void (*)(uintptr))(fn.f))(fn.arg);
}

m *m0;
g *g0;
g *allgs[1024];
_Atomic int main_started;
static pthread_key_t VirFSReg = 0; // pointer to m.tls
void settls(tls *ptr)
{
	int ret = pthread_setspecific(VirFSReg, ptr);
	if (ret != 0) {
		debugf("pthread_setspecific error:%d\n", ret);
		abort();
	}
}

tls *gettls()
{
	return (tls *)pthread_getspecific(VirFSReg);
}

// ctx must be within g struct
void gogo(Context *ctx)
{
	g *gp = container_of(ctx, g, ctx);
	tls *tls = gettls();
	tls->ptr[0] = (uintptr)gp;
	GetContext(ctx);
}

int allocGID()
{
	static int gid = 1;
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
	assert(readgstatus(g) == _Grunnable);
	pthread_mutex_lock(&p->mu);
	int h = p->runqhead;
	int t = p->runqtail;

	int size = (sizeof(p->runq) / sizeof(p->runq[0]));
	if (t - h < size) {
		p->runq[t % (size)] = g;
		p->runqtail = t + 1;
		pthread_mutex_unlock(&p->mu);
		return;
	}

	pthread_mutex_unlock(&p->mu);
	printf("runq overflow\n");

	// put to global runq
};

g *runqget(p *_p_)
{
	debugf("runqget:%p\n", _p_);
	pthread_mutex_lock(&_p_->mu);
	if (_p_->runqhead == _p_->runqtail) {
		pthread_mutex_unlock(&_p_->mu);
		return NULL;
	}
	int size = (sizeof(_p_->runq) / sizeof(_p_->runq[0]));
	g *c = _p_->runq[_p_->runqhead % size];
	_p_->runqhead++;
	pthread_mutex_unlock(&_p_->mu);
	debugf("runqget end\n");
	if (readgstatus(c) != _Grunnable) {
		printf("g:%d,%p\a", readgstatus(c), c->m);
	}
	assert(readgstatus(c) == _Grunnable);
	return c;
};

uint32 readgstatus(g *gp)
{
	return atomic_load(&gp->atomicstatus);
}

void casgstatus(g *gp, uint32 oldval, uint32 newval)
{
	// printf("cas g%d from %d to %d\n",gp->id,oldval,newval);
	atomic_store(&gp->atomicstatus, newval);
}

void mcall(void (*f)(g *))
{
	g *gp = getg();
	assert(gp->m);
	g *g0 = gp->m->g0;
	g0->ctx.reg.pc = (uintptr)f;
	g0->ctx.reg.rdi = (uintptr)gp;
	int ret = SaveContext(&gp->ctx);
	if (ret == 0) {
		tls *tls = gettls();
		tls->ptr[0] = (uintptr)g0;
		GetContext(&g0->ctx);
		panicf("bad mcall\n");
	}
}

void goexit0(g *gp)
{
	assert(gp == getg()->m->curg);
	debugf("goexit0\n");
	assert(readgstatus(gp) == _Grunning);
	casgstatus(gp, _Grunning, _Gdead);
	dropg();
	// put g to cache
	schedule();
}

void goexit1()
{
	mcall(goexit0);
}

void goexit()
{
	debugf("g %d exit\n", getg()->id);
	goexit1();
}

void goschedImpl(g *gp)
{
	assert(gp == getg()->m->curg);
	assert(readgstatus(gp) == _Grunning);
	casgstatus(gp, _Grunning, _Grunnable);
	runqput(gp->m->p, gp);
	dropg();
	schedule();
}

void Gosched()
{
	mcall(goschedImpl);
	return;
};

g *getg()
{
	return ((g *)gettls()->ptr[0]);
}

void systemstack(Func fn)
{
	debugf("systemstack call\n");
	// todo
	call_fn(fn);
}

g *newproc1(Func fn)
{
	g *newg = malg();
	allgadd(newg);
	int gid = allocGID();
	newg->id = gid;
	newg->fn = fn;
	newg->ctx.reg.rdi = (uintptr)newg->fn.arg;
	newg->ctx.reg.pc = (uintptr)newg->fn.f;
	casgstatus(newg, newg->atomicstatus, _Grunnable);
	runqput(getg()->m->p, newg);

	if (atomic_load(&sched.npidle) != 0 &&
	    atomic_load(&main_started) == 1) {
		wakep();
	}
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
	debugf("schedinit\n");
	g *_g_ = getg();
	void *xx;
	for (int i = 0; i < MAXPROC; i++) {
		p *_p_ = newT(p);
		// memset(allp[i], 0, sizeof(p));
		pthread_mutex_init(&_p_->mu, 0);
		_p_->id = i;
		_p_->link = sched.pidle;
		if (i != 0) {
			atomic_fetch_add(&sched.npidle, 1);
			sched.pidle = _p_;
		}
		allp[i] = _p_;
	}

	_g_->m->p = allp[0];
	allp[0]->m = _g_->m;
}

int main_main();

g *runqsteal(p *_p_, p *p2, bool stealRunNextG)
{
	debugf("runqsteal\n");
	if (p2 == NULL || p2 == _p_) {
		return NULL;
	}
	g *nextg = runqget(p2);
	if (nextg != NULL) {
		return nextg;
	}
	debugf("runqsteal end\n");
	return NULL;
}

void check_timers(p *pp, int64 ns);

g *findRunnable()
{
	debugf("findrunnable\n");
	p *_p_ = getg()->m->p;
	check_timers(_p_, 0);
	g *nextg = runqget(_p_);
	if (nextg != NULL) {
		return nextg;
	}
	for (int i = 0; i < MAXPROC; i++) {
		nextg = runqsteal(_p_, allp[i], true);
		if (nextg != NULL) {
			debugf("%d steal %d from %d\n", _p_->id, nextg->id,
			       allp[i]->id);
			break;
		}
	}
	return nextg;
}

void check_timers(p *pp, int64 ns)
{
	debugf("check_timers,%p\n", pp);
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
	debugf("check_timers end\n");
	pthread_mutex_unlock(&pp->timerslock);
}

// must on g0
void schedule()
{
	assert(getg()->is_g0);
	debugf("schedule %d\n", getg()->m->p->id);
	while (1) {
		g *nextg = findRunnable();
		if (nextg == NULL) {
			usleep(200 * 1000);
			debugf("p%d no co to run:\n", getg()->m->p->id);
			continue;
		}
		assert(readgstatus(nextg) == _Grunnable);
		execute(nextg);
		panicf("return from execute\n");
	}
}

void wakeg(g *gp)
{
	p *_p_ = getg()->m->p;
	casgstatus(gp, _Gwaiting, _Grunnable);
	runqput(_p_, gp);
}

void timeSleep(int64 ns)
{
	if (ns <= 0) {
		return;
	}

	g *gp = getg();
	casgstatus(gp, gp->atomicstatus, _Gwaiting);
	timer *t = newT(timer);
	t->when = nanotime() + ns;
	t->fn.arg = (uintptr)gp;
	t->fn.f = (uintptr)wakeg;
	p *_p_ = gp->m->p;
	push_timers(&_p_->timers, t);
	gopark(NULL, NULL, 0);
	debugf("switch end\n");
}

void mstart1()
{
	g *_g_ = getg();
	Func fn = _g_->m->mstartfn;
	if (fn.f != 0) {
		((void (*)(uintptr))(fn.f))(fn.arg);
	}

	schedule();
}

void mstart()
{
	mstart1();
	// never return
	abort();
	// pthread_exit(0);
}

int main();
// really main
int rt0_go()
{
	debugf("asm main\n");
	memset(allgs, 0, 1024 * sizeof(uintptr));
	int ret = pthread_key_create(&VirFSReg, NULL);
	if (ret != 0) {
		debugf("pthread_key_create error:%d\n", ret);
		abort();
	}
	// init g0
	Func fg0;
	fg0.f = (uintptr)schedule;
	g0 = malg();
	g0->id = -g0->id;
	g0->is_g0 = true;
	m *m0 = newT(m);
	settls(&m0->tls);
	m0->tls.ptr[0] = (uintptr)g0;
	g0->m = m0;
	m0->g0 = g0;
	m0->curg = g0;
	allgs[0] = g0;
	g0->id = 0;
	g0->fn = fg0;
	g0->ctx.reg.pc = (long)g0->fn.f;

	schedinit();
	newproc((uintptr)main, 0);
	newm((uintptr)sysmon, 0);
	// sleep(1000);
	mstart();

	abort();
	return 0;
}

int main()
{
	if (!g0) {
		debugf("main must called after rt0_go\n");
		abort();
	}
	atomic_store(&main_started, 1);
	main_main();
	exit(0);
	return 0;
}
