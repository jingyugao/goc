
#include "runtime.h"

#include "time.h"
#include <pthread.h>

#define _StackReserve (1 << 20)
#define _StackMin (2 << 10)

#define ALIGN(p, alignbytes)                                                   \
  ((void *)(((unsigned long)(p) + (alignbytes)-1) & ~((alignbytes)-1)))

m *m0;
g *g0;
g *allgs[1024];

pthread_mutex_t bigmutex = PTHREAD_MUTEX_INITIALIZER;

void lockall() {
  int ret = pthread_mutex_lock(&bigmutex);
  if (ret != 0) {
    printf("pthread_mutex_lock error:%d\n", ret);
    abort();
  }
}

void unlockall() {
  int ret = pthread_mutex_unlock(&bigmutex);
  if (ret != 0) {
    printf("pthread_mutex_unlock error:%d\n", ret);
    abort();
  }
}

static pthread_key_t VirFSReg = 0; // pointer to m.tls
void settls(tls *ptr) {
  int ret = pthread_setspecific(VirFSReg, ptr);
  if (ret != 0) {
    printf("pthread_setspecific error:%d\n", ret);
    abort();
  }
}

tls *gettls() { return (tls *)pthread_getspecific(VirFSReg); }

p *getP() {
  return getg()->mp->p;
};

// ctx must be within g struct
void gogo(Context *ctx) {
  g *gp = (g *)((void *)ctx - (void *)(&((g *)NULL)->ctx));
  tls *tls = gettls();
  tls->ptr[0] = gp;
  GetContext(ctx);
}

void SwitchTo(g *from, g *to) {
  to->mp = from->mp;
  to->mp->curg = to;
  // printf("switch %d to %d\n", from->id, to->id);
  int ret = SaveContext(&from->ctx);
  if (ret == 0) {
    gogo(&to->ctx);
  }
}

static int gid = 1;
int AllocGID() {
  gid++;
  return gid;
}
void allgadd(g *gp) {
  for (int i = 0; i < 1024; i++) {
    if (allgs[i] == NULL) {
      allgs[i] = gp;
      return;
    }
  }
  assert(0);
  return;
}

g *malg() {
  int stackSize = _StackReserve;
  uintptr stackTop;

  int ret = posix_memalign(&stackTop, stackSize, stackSize);
  if (ret != 0) {
    return NULL;
  }
  uintptr stackBase = stackTop + stackSize;
  g *c = (g *)malloc(sizeof(g));
  memset(&c->ctx, 0, sizeof(Context));

  c->stack.lo = stackTop;
  // align
  stackBase = ALIGN(stackBase, 16);
  stackBase = stackBase - 8;
  *(long *)stackBase = goexit;

  c->stack.hi = stackBase;
  c->ctx.reg.rsp = stackBase;

  return c;
}

void runqput(p *p, g *g) {
  if (g->id == 0) {
    backtrace();
    abort();
  }
  int h = p->runqhead;
  int t = p->runqtail;

  int size = (sizeof(p->runq) / sizeof(p->runq[0]));
  if (t - h < size) {
    p->runq[t % (size)] = g;
    p->runqtail = t + 1;
  }

  // put to global runq
};

g *runqget(p *p) {
  if (p->runqhead == p->runqtail) {
    return NULL;
  }
  int size = (sizeof(p->runq) / sizeof(p->runq[0]));
  g *c = p->runq[p->runqhead % size];
  p->runqhead++;
  return c;
}

void casgstatus(g *gp, uint32 oldval, uint32 newval) {
  gp->atomicstatus = newval;
}

void mcall(void (*f)(g *)) { f(getg()); }

void goexit0(g *gp) {
  casgstatus(gp, gp->atomicstatus, _Gdead);
  // todo schedule
  gogo(&gp->mp->g0->ctx);
}

void goexit1() { mcall(goexit0); }

void goexit() {
  printf("g %d exit\n", getg()->id);
  goexit1();
}

void Gosched() {
  g *curg = getg();
  p *p = getP();
  runqput(p, curg);
  casgstatus(curg, curg->atomicstatus, _Grunnable);
  SwitchTo(curg, curg->mp->g0);
  return;
};

g *getg() { return ((g *)gettls()->ptr[0]); }

void systemstack(Func fn) {

  printf("systemstack\n");
  g0->fn = fn;
  int ret = SaveContext(&g0->ctx);
  printf("ret:%d\n", ret);
  if (ret == 0) {
    printf("%d\n", g0->fn.f);
    g0->fn.f(g0->fn.arg);
    GetContext(&g0->ctx);
  }
}

g *newproc1(Func fn) {
  printf("newproc1\n");
  g *newg = malg();
  allgadd(newg);
  int gid = AllocGID();
  newg->id = gid;
  newg->fn = fn;
  newg->ctx.reg.rdi = newg->fn.arg;
  newg->ctx.reg.pc_addr = newg->fn.f;
  casgstatus(newg, newg->atomicstatus, _Grunnable);
  runqput(getP(), newg);
  return newg;
}

void newproc(void (*f)(void *), void *arg) {
  Func fn;
  fn.f = f;
  fn.arg = arg;
  newproc1(fn);
  return;
}

void schedinit() {
  printf("schedinit\n");
  g *_g_ = getg();
  for (int i = 0; i < MAXPORC; i++) {
    allp[i] = (p *)malloc(sizeof(p));
  }

  _g_->mp->p = allp[0];
  allp[0]->mp = _g_->mp;
}

int main_main();

g *findRunnable() {
  while (1) {
    int64 now = nanotime();
    g *nextg = runqget(getP());
    if (nextg == NULL) {
      return NULL;
    }
    // return nextg;
    if (nextg->when < now) {
      return nextg;
    }
    usleep(200);
    runqput(getP(), nextg);
  }
}

// must on g0
void schedule(void *arg) {
  printf("main_sched\n");
  while (1) {
    g *nextg = findRunnable();
    if (nextg == NULL) {
      sleep(1);
      printf("no co to run\n");
      continue;
    }
    SwitchTo(getg(), nextg);
  }

  exit(0);
}

void timeSleep(int64 ns) {
  if (ns <= 0) {
    return;
  }
  g *gp = getg();
  gp->when = nanotime() + ns;
  casgstatus(gp, gp->atomicstatus, _Gwaiting);
  Gosched();
}

void mstart1() { schedule(NULL); }

void mstart() {
  mstart1();
  pthread_exit(0);
}

int main();
// really main
int rt0_go() {
  printf("asm main\n");
  memset(allgs, 0, 1024 * sizeof(uintptr));
  int ret = pthread_key_create(&VirFSReg, NULL);
  if (ret != 0) {
    printf("pthread_key_create error:%d\n", ret);
    abort();
  }
  // init g0
  Func fg0;
  fg0.f = schedule;
  g0 = malg();
  m *m0 = (m *)malloc(sizeof(m));
  settls(&m0->tls);
  m0->tls.ptr[0] = g0;
  g0->mp = m0;
  m0->g0 = g0;
  m0->curg = g0;
  allgs[0] = g0;
  g0->id = 0;
  g0->fn = fg0;
  g0->ctx.reg.pc_addr = g0->fn.f;

  schedinit();
  newproc(main, NULL);
  printf("x\n");
  // wakep();
  // sleep(1000);
  mstart();

  abort();
  return 0;
}

int main() {
  if (!g0) {
    printf("main must called after rt0_go\n");
    abort();
  }
  main_main();

  exit(0);
  *(int *)0;
  return 0;
}
