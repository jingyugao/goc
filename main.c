#include "runtime.h"
#include "time.h"

int num = 0;

void doSomeThingBusy();

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
    Gosched();
    newproc(f2, &i);
  }
}

int main() __asm__("_main_main");

// user main go routinue
int main() {
  for (int i = 0; i < 10; i++) {
    newproc(f, NULL);
  }


  timeSleep(5*Minute);
  printf("ret :%d\n", num);
}
