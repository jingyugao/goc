#include "netpoll.h"
#include "type.h"
#include<assert.h>
#include <errno.h>
#include <sys/select.h>
fd_set pollrset;
fd_set pollwset;

void pollInit() {
  FD_ZERO(&pollrset);
  FD_ZERO(&pollwset);
}

void pollAdd(int fd, char mode) {
  switch (mode) {
  case 'r':
    FD_SET(fd, &pollrset);
    break;
  case 'w':
    FD_SET(fd, &pollwset);
    break;
  default:

    panicf("polladd bad op:%c", mode);
  }
}

void pollDel(int fd, char mode) {
  switch (mode) {
  case 'r':
    FD_CLR(fd, &pollrset);
    break;
  case 'w':
    FD_CLR(fd, &pollwset);
    break;
  default:
    panicf("pollDel bad op:%c", mode);
  }
}

listhead *netpoll(bool block) {
  listhead *list = newT(listhead);
  init_list_head(list);
  while (1) {
    fd_set rset, wset;
    FD_COPY(&pollrset, &rset);
    FD_COPY(&pollrset, &wset);

    struct timeval *timeout = NULL;
    struct timeval t;
    if (!block) {
      t.tv_sec = 0;
      t.tv_usec = 0;
      timeout = &t;
    }
    int ret = select(1024, &rset, &wset, NULL, timeout);
    printf("ret:%d\n", ret);
    if (ret == -1) {
      panicf("poll err:%d, %s", ret, strerror(errno));
    }
    if (ret == 0) {
      if (block) {
        continue;
      }
      return NULL;
    }
    for (int i = 0; i < 1024; i++) {
      pollDesc *pd = NULL;
      if (FD_ISSET(i, &rset)) {
        if (pd == NULL) {
          pd = newT(pollDesc);
        }
        pd->fd = i;
        pd->r = true;
      }
      if (FD_ISSET(i, &wset)) {
        if (pd == NULL) {
          pd = newT(pollDesc);
        }
        pd->fd = i;
        pd->w = true;
      }
      if (pd != NULL) {
        pd->fd = i;
        printf("list_add %d\n", i);
        list_add(&pd->list, list);
      }
    }
    return list;
  }
  assert(0);
}