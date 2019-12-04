#ifndef BASE_HEAP_H
#define BASE_HEAP_H
#include "slice.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// cmp(parent,child) === true
typedef struct {
  slice sli;
  bool (*cmp)(void *, void *);
} heap;

static void heap_free(heap h) {
  free(h.sli.ptr);
  return;
}

static bool heap_empty(heap *h) { return h->sli.used == 0; }

static size_t heap_size(heap *h, size_t elemsize) {
  return len_slice(h->sli, elemsize);
}

static void swap_buf(void *l, void *r, size_t size) {
  int d = *(int *)l;
  *(int *)l = *(int *)r;
  *(int *)r = d;

  // printf("swap_buf:%zu\n", size);
  // // swap
  // void *tmp = malloc(size);
  // printf("tmp:%p\n", tmp);
  // memcpy(tmp, l, size);
  // printf("ed3\n");
  // memcpy(l, r, size);
  // printf("ed2\n");
  // memcpy(r, tmp, size);
  // printf("ed\n");
  // free(tmp);
}

static void *heap_top(heap *h, size_t elemsize) {
  return index_slice(h->sli, 0, elemsize);
}

static void heap_push(heap *h, void *ptr, size_t elemsize) {
  h->sli = append_slice(h->sli, ptr, elemsize);
  int idx = len_slice(h->sli, elemsize);
  idx--; // last one
  while (1) {
    int pidx = (idx - 1) / 2;
    if (idx == 0) {
      return;
    }

    void *parent = index_slice(h->sli, pidx, elemsize);
    void *child = index_slice(h->sli, idx, elemsize);
    if (h->cmp(parent, child)) {
      return;
    }
    swap_buf(parent, child, elemsize);
    idx = pidx;
  }
}

static void heap_pop(heap *h, size_t elemsize) {
  void *root = index_slice(h->sli, 0, elemsize);

  void *last = end_slice(h->sli, elemsize);
  swap_buf(root, last, elemsize);
  h->sli = pop_slice(h->sli, 1, elemsize);

  int idx = 0;
  while (1) {
    int lidx = 2 * idx + 1;
    int ridx = 2 * idx + 2;
    // printf("x:%d\n",idx);
    // no child
    if (lidx >= len_slice(h->sli, elemsize)) {
      return;
    }

    void *parent = index_slice(h->sli, idx, elemsize);
    void *lchild = index_slice(h->sli, lidx, elemsize);

    // only left child
    if (lidx + 1 == len_slice(h->sli, elemsize)) {
      if (h->cmp(parent, lchild)) {
        return;
      }
      swap_buf(parent, lchild, elemsize);
      return;
    }

    void *rchild = index_slice(h->sli, ridx, elemsize);
    if (h->cmp(parent, lchild) && h->cmp(parent, rchild)) {
      return;
    }

    if (h->cmp(lchild, rchild)) {
      //  swap l and p
      swap_buf(lchild, parent, elemsize);
      idx = lidx;
    } else {
      // swap r and p
      swap_buf(rchild, parent, elemsize);
      idx = ridx;
    }
  }

  return;
}

#endif