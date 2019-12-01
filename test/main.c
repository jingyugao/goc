#include "../runtime.h"
#include "../time2.h"
#include <stdatomic.h>

int num = 0;
_Atomic int atmicnum = 0;

void f(void *arg) {
  for (int i = 0; i < 1000; i++) {
    // usleep(500 * 1000);
    num++;
    atomic_fetch_add(&atmicnum, 1);
    int gid = getg()->id;
    printf("g%d is runing on p%d\n", gid, getg()->mp->p->id);
  }
}

int main() __asm__("_main_main");

// user main go routinue
int main() {
  for (int i = 0; i < 250; i++) {
    go(f, NULL);
  }

  timeSleep(5 * Second);
  printf("ret :%d,%d,\n", num, atmicnum);
  assert(num <= atmicnum);
}
