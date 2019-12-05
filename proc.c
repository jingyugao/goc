#include "os.h"
#include "runtime.h"
#include "runtime2.h"
#include <pthread.h>
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
	m *mp = allocm(_p_, f);
	newm1(mp);
}

m *allocm(p *_p_, uintptr f)
{
	m *mp = newT(m);
	mp->g0 = malg();
	mp->g0->mp = mp;
	mp->p = _p_;
	mp->mstartfn.f = f;
	return mp;
}

void newm1(m *mp)
{
	newosproc(mp);
}