#include "runtime.h"
#include "runtime2.h"

m *allocm(p *_p_, void *fn);
void newm1(m *mp);
void newm(void *f, p *_p_);
void startm(p *_p_, bool spiinning);

p *pidleget() {
  for (int i = 0; i < MAXPORC; i++) {
    if (allp[i]->mp == NULL) {
      return allp[i];
    }
  }
  return (p *)malloc(sizeof(p));
}

void wakep() { startm(NULL, true); }

void startm(p *_p_, bool spiinning) {
  if (_p_ == NULL) {
    _p_ = pidleget();
  }

  newm(NULL, _p_);
  return;
}

void newm(void *f, p *_p_) {
  printf("newm\n");
  m *mp = allocm(_p_, f);
  newm1(mp);
}

m *allocm(p *_p_, void *fn) {
  m *mp = (m *)malloc(sizeof(m));
  mp->g0 = malg();
  mp->g0->mp = mp;
  mp->p = _p_;
  return mp;
}

void newm1(m *mp) { newosproc(mp); }