#include "runtime.h"

int num = 0;

void f2(int *arg) {
  (*arg)++;
  int gid = getg()->id;
};

void f(void *arg) {
  for (int i = 0; i < 10; i++) {
    usleep(10000);
    num++;
    int gid = getg()->id;
    printf("g%d is runing %d\n", gid, i);
    yield();
    newproc(f2, &i);
  }
}

int main() __asm__("_main_main");

// user main go routinue
int main() {
  for (int i = 0; i < 10; i++) {
    newproc(f, NULL);
  }
  for (int i = 0; i < 1000; i++) {
    yield();
  }
  printf("ret :%d\n", num);
}
