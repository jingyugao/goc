
#include "runtime.h"

#include "time.h"
#include <pthread.h>

#define _StackMin (1 << 20)
#define ALIGN(p, alignbytes)                                                   \
  ((void *)(((unsigned long)(p) + (alignbytes)-1) & ~((alignbytes)-1)))

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

g *allgs[1024];
static g *g0;

p *getP() {
  static p p;
  return &p;
};

void SwitchTo(g *from, g *to) {
  
  // printf("switch %d to %d\n", from->id, to->id);
  int ret = SaveContext(&from->ctx);
  if (ret == 0) {
    GetContext(&to->ctx);
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
  int stackSize = _StackMin;
  uintptr stackTop;

  int ret = posix_memalign(&stackTop, stackSize, stackSize);
  if (ret != 0) {
    return NULL;
  }
  uintptr stackBase = stackTop + stackSize;
  g *c = stackTop;
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
    exit(1);
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

void mcall(void (*p)(g *)) { p(getg()); }

void goexit0(g *gp) {
  casgstatus(gp, gp->atomicstatus, _Gdead);
  GetContext(&g0->ctx);
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
  SwitchTo(curg, g0);
  return;
};

g *getg() {
  __asm__("movq $-1<<20,%%rax\n\t"
          "andq %%rsp, %%rax;\n\t"
          "popq	%%rbp;\n\t"
          "retq;\n\t"
          :
          :
          :);
  *(int *)(NULL) = 1;
  return NULL;
}

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

g *newproc1(Func fn);

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
  g *gp = getg();
  Func fn;
  fn.f = f;
  fn.arg = arg;
  newproc1(fn);
  return;
}

void schedinit() { printf("schedinit\n"); }

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

void sched(void *arg) {
  printf("main_main\n");

  while (1) {
    g *nextg = findRunnable();
    if (nextg == NULL) {
      sleep(1);
      printf("no co to run\n");
      continue;
    }
    SwitchTo(g0, nextg);
  }

  printf("main return\n");
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

void mstart1() { GetContext(&g0->ctx); }

void mstart() {
  mstart1();
  exit(0);
}

int main();
// really main
int rt0_go() {
  printf("asm main\n");
  memset(allgs, 0, 1024 * sizeof(uintptr));
  p *p = getP();
  memset(p, 0, sizeof(p));

  // init g0
  Func fg0;
  fg0.f = sched;
  g0 = malg();
  allgs[0] = g0;
  g0->id = 0;
  g0->fn = fg0;
  g0->ctx.reg.pc_addr = g0->fn.f;

  schedinit();
  newproc(main, NULL);

  mstart();
  abort();
  return 0;
}

int main() {
  if (!g0) {
    printf("main must called after rt0_go\n");
    exit(1);
  }
  main_main();

  exit(0);
  *(int *)0;
  return 0;
}
