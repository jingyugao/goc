#include<stdio.h>
#include"call.h"

typedef struct Request
{
    void *arg;
    void *ret;
}Request;


void call(Request *req){
    void * p=req;
    asm ( 
          "subq $16, %%rsp;"
          "movq %0, %%rdi;"
	      "callq	_add;"
          "addq $16, %%rsp;"
          :             /* output */
          :"r"(p)              /* input */
          :"%rax"         /* clobbered register */
);
   
    return ;
}



void call_add(AddArg *arg,AddRet *ret){
    Request req;
    req.arg=arg;
    req.ret=ret;
    call(&req);
}

void add(Request *req){
    AddArg *arg=req->arg;
    AddRet *ret=req->ret;
    ret->c=arg->a+arg->b;
}

 