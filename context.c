

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef __GNUC__
#define __asm__ asm
#endif

typedef void *uintptr;

typedef struct {
  uintptr rbx, rsp, rbp, r12, r13, r14, r15, pc_addr;
} Register;

typedef union {
  uintptr buffer[8];
  Register reg;
} Context;

int SaveContext(Context *ctx) {
  __asm__("movq %%rbp, %%rsp;\n\t"
          "popq %%rbp;\n\t"
          "pop  %%rsi;\n\t"
          "xorl %%eax,%%eax;\n\t"
          "movq %%rbx,(%0);\n\t"
          "movq %%rsp,8(%0);\n\t"
          "pushq %%rsi;\n\t"
          "movq %%rbp,16(%0);\n\t"
          "movq %%r12,24(%0);\n\t"
          "movq %%r13,32(%0);\n\t"
          "movq %%r14,40(%0);\n\t"
          "movq %%r15,48(%0);\n\t"
          "movq %%rsi,56(%0);\n\t"
          // "popq	%%rbp;\n\t"
          "ret;\n\t"
          :
          : "r"(ctx->buffer)
          : "%rax");
};

int GetContext(Context *ctx) {
  register Context *p1 __asm__("rdi") = ctx;
  __asm__("movq %%rbp, %%rsp;\n\t"
          "popq %%rbp;\n\t"
          "movl $1,%%eax;\n\t"
          "movq (%%rdi),%%rbx;\n\t"
          "movq 8(%%rdi),%%rsp;\n\t"
          "movq 16(%%rdi),%%rbp;\n\t"
          "movq 24(%%rdi),%%r12;\n\t"
          "movq 32(%%rdi),%%r13;\n\t"
          "movq 40(%%rdi),%%r14;\n\t"
          "movq 48(%%rdi),%%r15;\n\t"
          "jmp *56(%%rdi);\n\t"
          :
          :
          : "rdi");
};

typedef struct {
  void *fn;
} Func;

typedef struct {
  void *args;
} Args;

typedef struct {
  uintptr lo;
  uintptr hi;
} Stack;

typedef struct {
  int id;
  Context ctx;
  Func fn;
  Args args;
  Stack stack;
} Coroutine;

static Coroutine *allCo[1024];

void SwitchTo(Coroutine *from, Coroutine *to) {
  printf("switch %d to %d\n",from->id,to->id);
  int ret = SaveContext(&from->ctx);
  if (ret == 0) {
    GetContext(&to->ctx);
  }
}

int getGID();

int AllocGID() {
  for (int i = 1; i < 1024; i++) {
    if (allCo[i] == NULL) {
      return i;
    }
  }
  return -1;
}

Coroutine *NewCoroutine(void *fn) {
  Coroutine *c = malloc(sizeof(Coroutine));
  memset(&c->ctx, 0, sizeof(Context));

  int gid = AllocGID();
  if (gid == -1) {
    return NULL;
  }
  allCo[gid] = c;

  c->id = gid;
  c->fn.fn = fn;
  int stackSize = (1 << 20);
  uintptr stackTop = malloc(stackSize);
  printf("%d:%p,%p\n", gid, stackTop, stackTop + stackSize);
  c->stack.lo = stackTop;
  c->stack.hi = stackTop + stackSize;
  c->ctx.reg.rsp = c->stack.hi;
  c->ctx.reg.pc_addr = c->fn.fn;

  return c;
}
void yield();

int CoStart(Coroutine *c) { return GetContext(&c->ctx); }


static int i=0;

void f() {
  while (1) {
    i++;
   int gid= getGID();
    printf("co%d %dis runing\n",gid ,i);
    yield();
  }
}

typedef struct WaitNode {
  Coroutine *c;
  struct WaitNode *next;
} WaitNode;

typedef struct {
  int runqhead;
  int runqtail;
  Coroutine *runq[256];
} scheduler;

void runqput(scheduler *p, Coroutine *g) {
  int h = p->runqhead;
  int t = p->runqtail;
  printf("runqput:%d,%d\n",h,t);

  int size=(sizeof(p->runq) / sizeof(p->runq[0]));
  if (t - h < size) {
    p->runq[t%(size)] = g;
    p->runqtail = t + 1;
  }

  printf("%d %d\n", p->runqhead, p->runqtail);
  // put to global runq
};

Coroutine *runqget(scheduler *p) {
  printf("%d %d\n", p->runqhead, p->runqtail);
  if (p->runqhead == p->runqtail) {
    return NULL;
  }
  int size=(sizeof(p->runq) / sizeof(p->runq[0]));
  Coroutine *c=p->runq[p->runqhead%size];
  p->runqhead++;
  return c;
}

scheduler *getP() {
  static scheduler p;
  return &p;
};

void yield() {
  printf("yield\n");
  int gid = getGID();
  scheduler *p = getP();
  Coroutine *c = allCo[gid];
  runqput(p, c);

  while (1) {
    Coroutine *c2 = runqget(p);
    if (c2 == NULL) {
      sleep(1);
      printf("no co to run");
      continue;
    }
    SwitchTo(c, c2);
    break;
  }
};

int getGID() {
  printf("get gid begin\n");
  sleep(1);
  printf("get gid end\n");

  uintptr addr;
  addr = &addr;

  for (int i = 0; i < 1024; i++) {
    Coroutine *c = allCo[i];
    if (!c) {
      continue;
    }
    if (c->stack.lo < addr && c->stack.hi > addr) {
      printf("id:%d\n",c->id);
      return c->id;
    }
  }
  return -1;
}

int main() {  
  sleep(1); // sleep需要某个  
  memset(allCo, 0, 1024 * sizeof(uintptr));
  scheduler *p = getP();
  memset(p, 0, sizeof(scheduler));
  for (int i=0;i<10;i++){
    runqput(p, NewCoroutine(f));
  }


  CoStart(runqget(p));
}