#include "runtime2.h"
#include <stdlib.h>

p *pidleget() { return (p *)malloc(sizeof(p)); }

void wakep() { startm(NULL, true); }

void startm(p *_p_, bool spiinning) {
  if (_p_ == NULL) {
    _p_ = pidleget();
  }
  m *mp = (m *)malloc(sizeof(m));
}
