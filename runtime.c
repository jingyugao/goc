
#include "runtime.h"

#define _StackMin 1 << 20

g *allCo[1024];

void SwitchTo(g *from, g *to) {
  printf("switch %d to %d\n", from->id, to->id);
  int ret = SaveContext(&from->ctx);
  if (ret == 0) {
    GetContext(&to->ctx);
  }
}

int AllocGID() {
  for (int i = 1; i < 1024; i++) {
    if (allCo[i] == NULL) {
      return i;
    }
  }
  return -1;
}

g *malg(void *fn) {
  int gid = AllocGID();
  if (gid == -1) {
    return NULL;
  }

  int stackSize = _StackMin;
  uintptr stackTop;

  int ret = posix_memalign(&stackTop, stackSize, stackSize);
  if (ret != 0) {
    return NULL;
  }
  uintptr stackBase = stackTop + stackSize;
  g *c = stackTop;
  memset(&c->ctx, 0, sizeof(Context));

  allCo[gid] = c;
  c->id = gid;
  c->fn.fn = fn;
  c->stack.lo = stackTop;
  c->stack.hi = stackBase;
  // align
  stackBase = stackBase - (long)stackBase % 16 - 8;
  c->ctx.reg.rsp = stackBase;

  c->ctx.reg.pc_addr = c->fn.fn;

  return c;
}
void yield();

void CoStart(g *c) { return GetContext(&c->ctx); }

typedef struct WaitNode {
  g *c;
  struct WaitNode *next;
} WaitNode;

typedef struct {
  int runqhead;
  int runqtail;
  g *runq[256];
} scheduler;

void runqput(scheduler *p, g *g) {
  int h = p->runqhead;
  int t = p->runqtail;

  int size = (sizeof(p->runq) / sizeof(p->runq[0]));
  if (t - h < size) {
    p->runq[t % (size)] = g;
    p->runqtail = t + 1;
  }

  // printf("%d %d\n", p->runqhead, p->runqtail);
  // put to global runq
};

g *runqget(scheduler *p) {
  if (p->runqhead == p->runqtail) {
    return NULL;
  }
  int size = (sizeof(p->runq) / sizeof(p->runq[0]));
  g *c = p->runq[p->runqhead % size];
  p->runqhead++;
  return c;
}

scheduler *getP() {
  static scheduler p;
  return &p;
};

void coExit() {
  int gid = getg()->id;
  g *curg = allCo[gid];
  free(curg->stack.lo);
  allCo[gid] = NULL;

  printf("co %d exit\n", gid);
  scheduler *p = getP();
  while (1) {
    g *nextg = runqget(p);
    printf(" co null\n");
    if (nextg == NULL) {
      sleep(1);
      printf("no co to run\n");
      continue;
    }
    CoStart(nextg);
  }
}

void yield() {
  printf("yield\n");
  g *curg = getg();

  scheduler *p = getP();

  runqput(p, curg);
  while (1) {
    g *nextg = runqget(p);
    if (nextg == NULL) {
      sleep(1);
      printf("no co to run\n");
      continue;
    }
    SwitchTo(curg, nextg);
    break;
  }
};

#define ALIGN(p, alignbytes)                                                   \
  ((void *)(((unsigned long)(p) + (alignbytes)-1) & ~((alignbytes)-1)))

#define GETRSP(v) __asm__("movq %%rsp, %0;" : : "m"(c) :)

g *getg() {
  __asm__(
      "movq $-1<<20,%%rax\n\t"
      "andq %%rsp, %%rax;\n\t"
      "popq	%%rbp;\n\t"
	  "retq;\n\t"
      : 
      :
      :);

  return NULL;
}

void f() {
  static int n = 0;
  for (int i = 0; i < 10; i++) {
    usleep(1000);
    n++;
    int gid = getg()->id;
    printf("gid %d", gid);
    printf("co%d is runing %d\n", gid, i);
    yield();
  }
  coExit();
}

void rt0_lib_go() {
  uintptr c;
  GETRSP(c);
  c = ALIGN(c, _StackMin);
  printf("%p", c);
}

int main(int argc) {
  //   rt0_lib_go();
  //   return 0;
  memset(allCo, 0, 1024 * sizeof(uintptr));
  scheduler *p = getP();
  memset(p, 0, sizeof(scheduler));
  for (int i = 0; i < 4; i++) {
    runqput(p, malg(f));
  }
  CoStart(runqget(p));
  printf("main return\n");
}
