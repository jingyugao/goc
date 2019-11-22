

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef void *uintptr;


enum register_t {
  rbx,
  rsp,
  rbp,
  r12,
  r13,
  r14,
  r15,
  pc_addr,
};

typedef struct {
  uintptr buffer[8];
} Context;

int SaveContext(Context *ctx) {
  asm("movq %%rbp, %%rsp;\n\t"
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
  register Context *p1 asm("rdi") = ctx;
  asm("movq %%rbp, %%rsp;\n\t"
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

void SwitchTo(Coroutine *from, Coroutine *to) {
  int ret = SaveContext(&from->ctx);
  if (ret == 0) {
    GetContext(&to->ctx);
  }
}

static int ids[1024];

int AllocGID() {
  int i = 1;
  for (i = 1; i < 1024; i++) {
    if (ids[i] == 0) {
      ids[i]=1;
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
  c->id=gid;
  c->fn.fn = fn;
  int stackSize = (1 << 20);
  uintptr stackTop = malloc(stackSize);

  c->stack.lo = stackTop;
  c->stack.hi = stackTop + stackSize;
  c->ctx.buffer[rsp] = c->stack.hi;
  c->ctx.buffer[pc_addr] = c->fn.fn;

  return c;
}

int CoStart(Coroutine *c) { return GetContext(&c->ctx); }

static Coroutine *g0;

void cmain() {
  printf("cmain\n");
  sleep(1);
}

void schedinit() {
  Coroutine *g0 = NewCoroutine(cmain);
  CoStart(g0);
};

static Coroutine *c1;
static Coroutine *c2;

void f1() {
  while (1) {
    printf("f1 %d is runing\n", getGID());
    sleep(1);
    SwitchTo(c1, c2);
  }
}

void f2() {
  while (1) {
    printf("f2 %d is runing\n", getGID());
    sleep(1);
    SwitchTo(c2, c1);
  }
}

int getGID();


typedef struct {
  Coroutine * allCo[1024];
} scheduler;


static scheduler p;

void yield(){
  
};


int getGID() {
  uintptr addr;
  addr = &addr;
  printf("addr:%p\n",addr);
  int i;
  for (i=0; i<1024; i++) {
    Coroutine *c = p.allCo[i];
    if (!c){
      continue;
    }
    if (c->stack.lo < addr && c->stack.hi > addr) {
      return c->id;
    }
  }
  return -1;
}

int main() {
  memset(ids,0,1024*sizeof(int));
  memset(&p, 0, sizeof(p));
  printf("newc\n");
  c1 = NewCoroutine(f1);
  c2 = NewCoroutine(f2);
  p.allCo[c1->id]=c1;
  p.allCo[c2->id]=c1;



  CoStart(c1);
}