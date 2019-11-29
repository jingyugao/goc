#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "context.h"
#include "runtime2.h"
#include "type.h"

#define _Gidle     (0）
#define _Grunnable (1)
#define _Grunning (2)
#define _Gsyscall (3)
#define _Gwaiting (4)
#define _Gdead (6)

extern g *allgs[1024];
g *getg();
g *malg();

void lockall();
void unlockall();

void timeSleep(int64 ns);
void newproc(void (*f)(void *), void *arg);
void goexit();
void settls(tls *ptr);
tls *gettls();
void newm(void *f, p *_p_) ;

#endif