
#include "../base/slice.h"
#include "test.h"
int main() {
  slice sli = slice_new(int, 4, 6);
  for (int i = 0; i < 4; i++) {
    int data = 200 + i;
    sli = append(sli, &data);
  }
  for (int i = 0; i < 4; i++) {
    int *d = slice_get(sli, i, int);
    *d = 100 + i;
  }

  assert(slice_len(sli, int) == 8);
  int expected[8] = {100, 101, 102, 103, 200, 201, 202, 203};
  for (int i = 0; i < slice_len(sli, int); i++) {
    int *d = slice_get(sli, i, int);
    assert(*d == expected[i]);
  }

  sli = slice_pop(sli, 1, int);
  assert(*slice_end(sli, int) == 202);
  assert(sli.cap == 33);

  test_ok;
}