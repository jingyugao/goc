#ifndef CALL_H
#define CALL_H

typedef struct {
    int a;
    int b;
}AddArg;

typedef struct {
    int c;
}AddRet;
void call_add(AddArg *arg,AddRet *ret);


#endif