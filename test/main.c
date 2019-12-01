#include "../runtime.h"
#include "../time2.h"
#include <stdatomic.h>

int num = 0;
_Atomic int atmicnum = 0;

void f(void *arg) {
  for (int i = 0; i < 1000; i++) {
    // usleep(500 * 1000);
    num++;
<<<<<<< HEAD
    atomic_fetch_add(&atmicnum, 1);
=======
    atomic_fetch_add(&atmicnum,1);
  }

int main() __asm__("_main_main");

// user main go routinue
int main() {
  for (int i = 0; i < 250; i++) {
    go(f, NULL);
  }

  timeSleep(5 * Second);
<<<<<<< HEAD
  printf("ret :%d,%d,\n", num, atmicnum);
=======
  printf("ret :%d,%d,\n", num,atmicnum);
>>>>>>> 06fe6b9aeb0b86db5b7265985c996c7b4ebe9ad2
  assert(num <= atmicnum);
}
