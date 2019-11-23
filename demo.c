
#include <stdlib.h>


#include "time.h"

int f(){
  int a=1;
  return a;
}

int main() {
  exit(1);
  for (int i=0;i<100;i++){
  int64 t=cputicks();
  printf("%lld\n",t);
  sleep(1);
  }
return 0;
}