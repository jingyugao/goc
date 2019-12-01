#ifndef CONTEXT_H
#define CONTEXT_H
#include "type.h"
#include <assert.h>

typedef struct {
  uintptr rbx, rsp, rbp, r12, r13, r14, r15, pc_addr, rdi;
} Register;


typedef union {
  uintptr buffer[9];
  Register reg;
} Context;

int SaveContext(Context *ctx);

void GetContext(Context *ctx);

#endif