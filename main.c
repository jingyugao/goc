#include "runtime.h"


void f(void *arg) {
  printf("f");
  static int n = 0;
  for (int i = 0; i < 10; i++) {
    usleep(1000);
    n++;
    int gid = getg()->id;
    printf("gid %d", gid);
    printf("co%d is runing %d\n", gid, i);
    yield();
  }
  printf("g %d exit\n", getg()->id);
}

int main() __asm__("_main_main");

// user main go routinue
int main() {
  for (int i = 0; i < 4; i++) {
    newproc(f, NULL);
    printf("newproc end\n");
  }
  for (int i = 0; i < 10; i++) {
    yield();
  }
}

