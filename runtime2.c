#include "runtime2.h"
#include <stdlib.h>
#define new (T)((T *)malloc(sizoef((*(T *)NULL))))

p *pidleget() { return new (p); }

void wakep() { startm(NULL, true); }

void startm(p *_p_, bool spiinning) {
  if (_p_ == NULL) {
    _p_ = pidleget();
  }
  m *mp = new (m);
}
