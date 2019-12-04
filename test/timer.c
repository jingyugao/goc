#include "../timer.h"
#include "test.h"
int main() {
  vector v;
  vector_init(&v);
  for (int i = 0; i < 100; i++) {
    timer *t = newT(timer);
    t->when = i + 1;
    push_timers(&v, t);
  }

  for (int i = 0; i < 10; i++) {
    assert(pop_timers(&v)->when = i + 1);
  }
  test_ok;
}