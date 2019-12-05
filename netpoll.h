#ifndef NETPOLL_H
#define NETPOLL_H

#include "base/list.h"
#include "mess.h"
#include "type.h"

typedef struct {
	listhead list;
	int fd;
	bool r;
	bool w;
} pollDesc;

void pollInit();

void pollAdd(int fd, char mode);

void pollDel(int fd, char mode);

listhead *netpoll(bool block);
#endif