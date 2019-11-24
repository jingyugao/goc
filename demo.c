
#include <stdlib.h>

#include "time.h"

#include <stdio.h>

typedef struct {
  long f;
  long args[5];
} arg_list;

arg_list realCreate(long f, arg0, arg1, arg2, arg3, arg4) {
  arg_list arg;
  arg.f = f;
  arg.args[0] = arg0;
  arg.args[1] = arg1;
  arg.args[2] = arg2;
  arg.args[3] = arg3;
  arg.args[4] = arg4;
  return arg;
}

#define CREATE_6(f, arg0, arg1, arg2, arg3, arg4)            realCreate(f, arg0, arg1, arg2, arg3, arg4)
#define CREATE_5(f, arg0, arg1, arg2, arg3)             CREATE_6(f, arg0, arg1, arg2, arg3, 0)

#define CREATE_4(f, arg0, arg1, arg2) CREATE_5(f, arg0, arg1, arg2, 0)
#define CREATE_3(f, arg0, arg1) CREATE_4(f, arg0, arg1, arg2)
#define CREATE_2(f, arg0) CREATE_3(f, arg0, 0)
#define CREATE_1(f) CREATE_2(f, 0)
#define CREATE_0() CREATE_1(0)

#define FUNC_CHOOSER(_f1, _f2, _f3,_f4,_f5,_f6, ...) _f6
#define FUNC_RECOMPOSER(argsWithParentheses) FUNC_CHOOSER argsWithParentheses
#define CHOOSE_FROM_ARG_COUNT(...)                                             \
  FUNC_RECOMPOSER((__VA_ARGS__, CREATE_6, CREATE_5,CREATE_4, CREATE_3,CREATE_2, CREATE_1, ))
#define NO_ARG_EXPANDER() , , CREATE_0
#define MACRO_CHOOSER(...) CHOOSE_FROM_ARG_COUNT(NO_ARG_EXPANDER __VA_ARGS__())
#define create(...) MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)

#define TM_PRINTF(f_, ...) printf((f_), ##__VA_ARGS__)

int main() {
  arg_list x = create();
  printf("%p,%p\n", x.args[0], x.args[1]);
  arg_list x2 = create(10);
  printf("x2 %ld,%ld\n",  x2.args[0], x2.args[1]);
  x = create(10, 20);
  printf("%ld,%ld\n", x.args[0], x.args[1]);

  // create(30, 30, 30);  // Compilation error
  return 0;
}
