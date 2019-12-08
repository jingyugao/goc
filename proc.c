#include "os.h"
#include "runtime.h"
#include "runtime2.h"
#include <pthread.h>
#include "mess.h"
m *allocm(p *_p_, uintptr f);
void newm1(m *mp);
void startm(p *_p_, bool spiinning);

p *pidleget()
{
	p *_p_ = sched.pidle;
	if (_p_ != NULL) {
		sched.pidle = _p_->link;
		return _p_;
	}
	return NULL;
}

void wakep()
{
	startm(NULL, true);
}

void startm(p *_p_, bool spiinning)
{
	pthread_mutex_lock(&sched.lock);
	if (_p_ == NULL) {
		_p_ = pidleget();
	}
	pthread_mutex_unlock(&sched.lock);
	if (_p_ == NULL) {
		return;
	}
	printf("newm on p%d\n", _p_->id);
	newm(0, _p_);
	return;
}

void newm(uintptr f, p *_p_)
{
	printf("newm");
	m *mp = allocm(_p_, f);
	newm1(mp);
}

m *allocm(p *_p_, uintptr f)
{
	m *mp = newT(m);
	g*newg= malg();
	allgadd(newg);
	int gid = allocGID();
	newg->id=gid;
	mp->g0 =newg;
	mp->g0->m = mp;
	mp->p = _p_;
	mp->mstartfn.f = f;
	return mp;
}

void newm1(m *mp)
{
	newosproc(mp);
}

void dropg()
{
	g *_g_ = getg();
	assert(_g_ == _g_->m->g0);
	_g_->m->curg->m = NULL;
	_g_->m->curg = NULL;
}

void execute(g*gp){
	g*	_g_ = getg();
	_g_->m->curg = gp;
	gp->m = _g_->m;
	casgstatus(gp, _Grunnable, _Grunning);
	gogo(&gp->ctx);
}

void park_m(g *gp)
{
	g*_g_=getg();
	casgstatus(gp, _Grunning, _Gwaiting);
	dropg();

	bool (*fn)(g*,void *)  = _g_->m->waitunlockf;
	if( fn != NULL ){
		bool ok = fn(gp, _g_->m->waitlock);
		_g_->m->waitunlockf = NULL;
		_g_->m->waitlock = NULL;
		if (!ok) {
			casgstatus(gp, _Gwaiting, _Grunnable);
			execute(gp) ;
		}
	}
	schedule();
}