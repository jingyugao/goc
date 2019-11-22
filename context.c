#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


typedef void *uintptr;

typedef struct
{
    uintptr rbx, rsp, rbp, r12, r13, r14, r15, pc_addr;
} Register;

typedef union {
    uintptr buffer[8];
    Register reg;
} Context;

int SaveContext(Context *ctx)
{
    __asm__(
            "leaveq;\n\t"
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
            "ret;\n\t"
            :
            : "r"(ctx->buffer)
            : );
    // never reach
    assert(0);
    return 0;
};

void GetContext(Context *ctx)
{
    // rdi is the first parameter
    __asm__("leaveq;\n\t"
            "movq (%%rdi),%%rbx;\n\t"
            "movq 8(%%rdi),%%rsp;\n\t"
            "movq 16(%%rdi),%%rbp;\n\t"
            "movq 24(%%rdi),%%r12;\n\t"
            "movq 32(%%rdi),%%r13;\n\t"
            "movq 40(%%rdi),%%r14;\n\t"
            "movq 48(%%rdi),%%r15;\n\t"
            "jmpq *56(%%rdi);\n\t"
            :
            :
            : );
};

typedef struct
{
    void *fn;
} Func;

typedef struct
{
    void *args;
} Args;

typedef struct
{
    uintptr lo;
    uintptr hi;
} Stack;

typedef struct
{
    int id;
    Context ctx;
    Func fn;
    Args args;
    Stack stack;
} g;

static g *allCo[1024];

void SwitchTo(g *from, g *to)
{
    printf("switch %d to %d\n", from->id, to->id);
    int ret = SaveContext(&from->ctx);
    if (ret == 0)
    {
        GetContext(&to->ctx);
    }
}


g *getg();
int AllocGID()
{
    for (int i = 1; i < 1024; i++)
    {
        if (allCo[i] == NULL)
        {
            return i;
        }
    }
    return -1;
}

g *Newg(void *fn)
{
    int gid = AllocGID();
    if (gid == -1)
    {
        return NULL;
    }

    int stackSize = (1 << 20);
    uintptr stackTop;

    int ret= posix_memalign(&stackTop, stackSize,stackSize);
    if (ret!=0){
        return NULL;
    }
    uintptr stackBase = stackTop+stackSize;
    stackBase = stackBase - sizeof(g);
    stackBase = stackBase - (long)stackBase%16-8;

    g *c = stackBase;
    memset(&c->ctx, 0, sizeof(Context));

    allCo[gid] = c;
    c->id = gid;
    c->fn.fn = fn;
    c->stack.lo = stackTop;
    c->stack.hi = stackBase;
    // align
    c->ctx.reg.rsp = stackBase;

    c->ctx.reg.pc_addr = c->fn.fn;

    return c;
}
void yield();

void CoStart(g *c) { return GetContext(&c->ctx); }

typedef struct WaitNode
{
    g *c;
    struct WaitNode *next;
} WaitNode;

typedef struct
{
    int runqhead;
    int runqtail;
    g *runq[256];
} scheduler;

void runqput(scheduler *p, g *g)
{
    int h = p->runqhead;
    int t = p->runqtail;

    int size = (sizeof(p->runq) / sizeof(p->runq[0]));
    if (t - h < size)
    {
        p->runq[t % (size)] = g;
        p->runqtail = t + 1;
    }

    // printf("%d %d\n", p->runqhead, p->runqtail);
    // put to global runq
};

g *runqget(scheduler *p)
{
    if (p->runqhead == p->runqtail)
    {
        return NULL;
    }
    int size = (sizeof(p->runq) / sizeof(p->runq[0]));
    g *c = p->runq[p->runqhead % size];
    p->runqhead++;
    return c;
}

scheduler *getP()
{
    static scheduler p;
    return &p;
};

void coExit()
{
    int gid=getg()->id;
    g *curg=allCo[gid];
    free(curg->stack.lo);
    allCo[gid]=NULL;

    printf("co %d exit\n", gid);
    scheduler *p = getP();
    while (1)
    {
        g *nextg = runqget(p);
        printf(" co null\n");
        if (nextg == NULL)
        {
            sleep(1);
            printf("no co to run\n");
            continue;
        }
        CoStart(nextg);
    }
}

void yield()
{
    printf("yield\n");
    g *curg=getg();

    scheduler *p = getP();

    runqput(p,  curg);
    while (1)
    {
        g *nextg = runqget(p);
        if (nextg == NULL)
        {
            sleep(1);
            printf("no co to run\n");
            continue;
        }
        SwitchTo(curg, nextg);
        break;
    }
};



#define ALIGN(p,alignbytes) ((void*)(((unsigned long)(p)+(alignbytes)-1)&~((alignbytes)-1)))


g *getg(){
    uintptr *c;
    __asm__(
            "movq %%rsp, %0;"
            :
            :"m"(c)
            :
           );

    c= ALIGN(c,1<<20);
    c=(long)c -0x10 -(long)sizeof(g);
    return c;
}


void f()
{
    static int n = 0;
    for (int i = 0; i < 10; i++)
    {
        usleep(1000);
        char buf[10];
        n++;
        int gid = getg()->id;
        allCo[gid];
        printf("gid %d",gid);
        printf("co%d is runing %d\n",gid,i);
        yield();
    }
    coExit();
}

int main(int argc)
{
    memset(allCo, 0, 1024 * sizeof(uintptr));
    scheduler *p = getP();
    memset(p, 0, sizeof(scheduler));
    for (int i = 0; i < 4; i++)
    {
        runqput(p, Newg(f));
    }
    CoStart(runqget(p));
    printf("main return\n");
}
