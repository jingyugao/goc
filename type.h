#ifndef TYPE_H
#define TYPE_H
#include <stdbool.h>
#include <stdint.h>

#define int64 int64_t
#define int32 int32_t
#define uintptr uintptr_t
#define byte unsigned char
#define uint32 uint32_t

typedef struct {
  int code;
  char str[1024];
} error;

#endif