#include "context.h"

int SaveContext(Context *ctx) {
  __asm__("leaveq;\n\t"
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
          :);
  // never reach
  assert(0);
  return 0;
};

void GetContext(Context *ctx) {
  // rdi is the first parameter
  __asm__("movq (%%rdi),%%rbx;\n\t"
          "movq 8(%%rdi),%%rsp;\n\t"
          "movq 16(%%rdi),%%rbp;\n\t"
          "movq 24(%%rdi),%%r12;\n\t"
          "movq 32(%%rdi),%%r13;\n\t"
          "movq 40(%%rdi),%%r14;\n\t"
          "movq 48(%%rdi),%%r15;\n\t"
          "movl $1,%%eax;\n\t"
          // pass param
          "movq %%rdi,%%rsi;\n\t"
          "movq 64(%%rdi),%%rdi;\n\t"
          "jmpq *56(%%rsi);\n\t"
          :
          :
          :);
};
