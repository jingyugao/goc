#include "os.h"
#include "runtime.h"
#include "runtime2.h"
m *allocm(p *_p_, uintptr f);
void newm1(m *mp);
void startm(p *_p_, bool spiinning);

p *pidleget()
{
	for (int i = 0; i < MAXPORC; i++) {
		if (allp[i]->mp == NULL) {
			return allp[i];
		}
	}
	return newT(p);
}

void wakep()
{
	startm(NULL, true);
}

void startm(p *_p_, bool spiinning)
{
	if (_p_ == NULL) {
		_p_ = pidleget();
	}

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