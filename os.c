#include <pthread.h>

#include "runtime2.h"

void mstart_stub(){

}


void newosproc(m *mp) {
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  uintptr stacksize;
  if (pthread_attr_getstacksize(&attr, &stacksize) != 0) {
    printf("pthread_attr_getstacksize error\n");
    exit(1);
  }
  mp->g0->stack.hi = stacksize; //???
  if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
    printf("pthread_attr_setdetachstate error\n");
    exit(1);
  }
    
  if (pthread_create(&mp->thread, &attr, mstart_stub, NULL) != 0) {
    printf("pthread_create error\n");
    exit(1);
  }
}