#include "runtime2.h"
#include <pthread.h>
#include <stdlib.h>

void mstart_stub() {
  g *g0 = getg();
  SaveContext(&g0->ctx);
  mstart();
}

void sysmon() {
  while (1) {
    sleep(1);
  }
}

void newosproc(m *mp);

void newm1(m *mp) { newosproc(mp); }

void newm(void *f, p *_p_) {
  m *mp = (m *)malloc(sizeof(m));

  newm1(mp);
}

void newosproc(m *mp) {
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  uintptr stacksize;
  if (pthread_attr_getstacksize(&attr, &stacksize) != 0) {
    printf("pthread_attr_getstacksize error\n");
    exit(1);
  }
  mp->g0->stack.hi = stacksize; //???
  if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
    printf("pthread_attr_setdetachstate error\n");
    exit(1);
  }

  if (pthread_create(&mp->thread, &attr, mstart_stub, NULL) != 0) {
    printf("pthread_create error\n");
    exit(1);
  }
}