#ifndef PROC_H
#define PROC_H
#include "runtime2.h"
void wakep();
void goparkunlock(pthread_mutex_t *lock, int reason);
void goready(g *gp);
void execute(g *gp);
void park_m(g *gp);
void dropg();
#endif