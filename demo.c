
#include <stdlib.h>

#include "mess.h"
#include "time.h"

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
  int *a = newT(int);
  for (int i = 0; i < 100; i++) {
    int64 t = cputicks();
    printf("%lld\n", t);
    sleep(1);
  }
  return 0;
}