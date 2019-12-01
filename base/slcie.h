#ifndef SLICE_H
#define SLICE_H
#include <assert.h>
#include <stdlib.h>
#include <string.h>
typedef struct {
  void *ptr;
  size_t used;
  size_t cap;
} slice;

#define append(sli, elemptr)                                                   \
  append_slice(sli, (void *)elemptr, sizeof(*elemptr))

#define slice_len(sli, type) (sli.used / sizeof(type))
#define slice_get(sli, index, elemptr)                                         \
  index_slice(sli, index, elemptr, sizeof(*elemptr))

#define slice_set(sli, index, elemptr)                                         \
  index_slice_set(sli, index, elemptr, sizeof(*elemptr))

#define slice_new(type, used, cap)                                             \
  make_slice(used * sizeof(type), cap * sizeof(type));

static slice make_slice(size_t used, size_t cap) {
  void *ptr = malloc(cap);
  memset(ptr, 0, cap);
  slice sli = {ptr, used, cap};
  return sli;
}

static slice grow_slice(slice sli, size_t grow) {
  if (sli.used + grow <= sli.cap) {
    return sli;
  }
  size_t newcap = 1.2 * (sli.cap + grow);
  void *ptr = malloc(newcap);
  memset(ptr, 0, newcap);

  memcpy(ptr, sli.ptr, sli.used);
  return sli;
}

static slice append_slice(slice sli, void *elemptr, size_t elemsize) {
  sli = grow_slice(sli, elemsize);
  memcpy(sli.ptr + sli.used, elemptr, elemsize);
  sli.used += elemsize;
  return sli;
}

static void index_slice(slice sli, int index, void *elemptr, size_t elemsize) {
  int offset = index * elemsize;
  assert(offset < sli.used);
  memcpy(elemptr, sli.ptr + offset, elemsize);
}

static void index_slice_set(slice sli, int index, void *elemptr,
                            size_t elemsize) {
  int offset = index * elemsize;
  assert(offset < sli.used);
  memcpy(sli.ptr + offset, elemptr, elemsize);
}

#endif