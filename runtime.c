
#include "runtime.h"

#define _StackMin (1 << 20)

g *allgs[1024];
static g *g0;

p *getP() {
  static p p;
  return &p;
};

void SwitchTo(g *from, g *to) {
  printf("switch %d to %d\n", from->id, to->id);
  int ret = SaveContext(&from->ctx);
  if (ret == 0) {
    GetContext(&to->ctx);
  }
}

int AllocGID() {
  for (int i = 1; i < 1024; i++) {
    if (allgs[i] == NULL) {
      return i;
    }
  }
  return -1;
}

g *malg(Func fn) {
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

  allgs[gid] = c;
  c->id = gid;
  c->fn = fn;
  c->stack.lo = stackTop;
  // align
  stackBase = stackBase - (long)stackBase % 16 - 8;
  c->stack.hi = stackBase;
  c->ctx.reg.rsp = stackBase;

  c->ctx.reg.pc_addr = c->fn.f;

  return c;
}

void CoStart(g *c) {
  printf("co start %d,%d\n", c, g0);
  Context ctx;
  int ret = SaveContext(&g0->ctx);
  printf("save ret:%d\n", ret);

  if (ret == 0) {
    printf("get ctx\n");
    GetContext(&c->ctx);
  }
  printf("costart ret:%d\n", ret);
  return;
}

void runqput(p *p, g *g) {
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

g *runqget(p *p) {
  if (p->runqhead == p->runqtail) {
    return NULL;
  }
  int size = (sizeof(p->runq) / sizeof(p->runq[0]));
  g *c = p->runq[p->runqhead % size];
  p->runqhead++;
  return c;
}

void yield() {
  printf("yield\n");
  g *curg = getg();
  p *p = getP();
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

void f(void *arg) {
  printf("f");
  static int n = 0;
  for (int i = 0; i < 10; i++) {
    usleep(1000);
    n++;
    int gid = getg()->id;
    printf("gid %d", gid);
    printf("co%d is runing %d\n", gid, i);
    yield();
  }
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

g *newproc1(Func fn) {
  printf("newproc1\n");
  g *newg = malg(fn);
  // newg->fn = fn;
  // newg->ctx.reg.pc_addr = newg->fn.f;
  // int gid = AllocGID();
  // allgs[gid] = newg;
  // newg->id = gid;
  runqput(getP(), newg);
  return newg;
}

void newproc(void (*f)(void *), void *arg) {
  printf("new proc\n");
  Func fn;
  Func *fn2 = malloc(sizeof(Func));
  fn2->f = f;
  fn2->arg = arg;
  fn.arg = fn2;
  fn.f = newproc1;
  systemstack(fn);
  printf("systemstack end\n");
}

void schedinit() { printf("schedinit\n"); }

void mstart() {
  g *_g_ = getg();
  int ret = SaveContext(&_g_->ctx);
  if (ret == 0) {
    GetContext(&_g_->ctx);
  }
}

void main_main(void *arg) {
  printf("main_main\n");
  GetContext(&g0->ctx);
  return;
  for (int i = 0; i < 4; i++) {
    newproc(f, NULL);
    printf("newproc end\n");
  }
  yield();
  printf("main return\n");
}

int main(int argc) {
  memset(allgs, 0, 1024 * sizeof(uintptr));
  p *p = getP();
  memset(p, 0, sizeof(p));

  Func fg0;
  g0 = malg(fg0);
  allgs[0] = g0;
  schedinit();

  Func fmm;

  fmm.f = main_main;
  g *gmm = malg(fmm);
  printf("save ctx\n");
  int ret = SaveContext(&g0->ctx);
  printf("ret:%d\n",ret);
  if (ret == 0) {
    GetContext(&gmm->ctx);
  }

  printf("exit\n");
  return 0;

  newproc(main_main, NULL);
  CoStart(runqget(p));
  return 0;
  g *ng = runqget(p);

  GetContext(&ng->ctx);
}
