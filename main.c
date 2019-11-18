#include<stdio.h>
#include"call.h"
int main(){
     AddArg arg;
     arg.a=1;
     arg.b=2;
     AddRet ret;
     call_add(&arg,&ret);
     printf("%d\n",ret.c);
}