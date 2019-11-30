#include "../runtime.h"
#include "../time2.h"

int num = 0;

void doSomeThingBusy();

void f2(int *arg) {
  (*arg)++;
  int gid = getg()->id;
};

void f(void *arg) {
  for (int i = 0; i < 10; i++) {
    usleep(500 * 1000);
    num++;
    int gid = getg()->id;
    printf("g%d is runing on p%d\n", gid, getg()->mp->p->id);
    go(f2, &i);
  }
}

int main() __asm__("_main_main");

// user main go routinue
int main() {
  for (int i = 0; i < 10; i++) {
    go(f, NULL);
  }

  timeSleep(10*Second);
  printf("ret :%d\n", num);
  assert(num <= 60);
}
