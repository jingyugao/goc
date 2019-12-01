
#include "../time2.h"
#include"test.h"
int f() {
  int a = 1;
  int b = 2;
  int tmp;
  tmp = a;
  a = b;
  b = tmp;
  return a;
}

int main() {
  int64 t1 = cputicks();
  int64 t2 = cputicks();
  assert(t2 > t1);

  test_ok;
}