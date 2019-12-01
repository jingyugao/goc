#include "../base/heap.h"
#include"test.h"
bool cmp(void *x, void *y) { return (*(int *)x) < (*(int *)y); }

int main() {

  heap *p = (heap *)malloc(sizeof(heap));
  memset(p, 0, sizeof(heap));
  p->sli = make_slice(0, 0);
  p->cmp = cmp;
  int elemsize = sizeof(int);

  assert(heap_empty(p));

  for (int i = 0; i < 10; i++) {
    int d = i;
    heap_push(p, &d, elemsize);
  }

  for (int i = 0; i < 10; i++) {
    int d = *(int *)heap_top(p, elemsize);
    assert(d==i);
    heap_pop(p, elemsize);
  }

  test_ok;
}

/*
[3 2 0 1 4]

    3
  2    0
1  4

    3
  4    0
1  2

*/