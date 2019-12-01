
#include "../base/slcie.h"
#include <assert.h>
#include <stdio.h>
int main() {
  slice sli = slice_new(int, 4, 6);
  for (int i = 0; i < 4; i++) {
    int data = 200 + i;
    sli = append(sli, &data);
  }
  for (int i = 0; i < 4; i++) {
    int data = 100 + i;
    slice_set(sli, i, &data);
  }
  assert(slice_len(sli, int) == 8);
  int expected[8] = {100, 101, 102, 103, 200, 201, 202, 203};
  for (int i = 0; i < slice_len(sli, int); i++) {
    int ret;
    slice_get(sli, i, &ret);
    assert(ret == expected[i]);
  }
  return 0;
}