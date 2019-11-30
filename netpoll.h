#ifndef NETPOLL_H
#define NETPOLL_H

#include <sys/select.h>
#include <base/list.h>
#include <mess.h>
#include <errno.h>
typedef struct
{
    listhead list;
    int fd;
    bool r;
    bool w;
} pollDesc;

fd_set pollrset;
fd_set pollwset;
int pollfd;

void pollInit()
{
    FD_ZERO(&pollrset);
    FD_ZERO(&pollwset);
}

void pollAdd(int fd, char mode)
{
    switch (mode)
    {
    case 'r':
        FD_SET(fd, &pollrset);
        break;
    case 'w':
        FD_SET(fd, &pollwset);
        break;
    default:
        error err;
        sprintf(err.str, "polladd bad op:%c", mode);
        panic(err);
    }
}

void pollDel(int fd, char mode)
{
    switch (mode)
    {
    case 'r':
        FD_CLR(fd, &pollrset);
        break;
    case 'w':
        FD_CLR(fd, &pollwset);
        break;
    default:
        error err;
        sprintf(err.str, "pollDel bad op:%c", mode);
        panic(err);
    }
}

listhead *netpoll(bool block)
{
    listhead *list = newT(listhead);
    init_list_head(list);
    while (1)
    {
        struct timeval *timeout = NULL;
        if (!block)
        {
            struct timeval t;
            timeout = &t;
        }
        int ret = select(1024, &pollrset, &pollwset, NULL, timeout);
        if (ret == -1)
        {
            error err;
            sprintf(err.str, "poll err:%d, %s", ret, strerror(errno));
            panic(err);
        }
        if (ret == 0)
        {
            if (block)
            {
                continue;
            }
            return NULL;
        }
        for (int i = 0; i < 1024; i++)
        {
            pollDesc *pd;
            if (FD_ISSET(i, &pollrset))
            {
                if (pd == NULL)
                {
                    pd = newT(pollDesc);
                }
                pd->fd = i;
                pd->r = true;
            }
            if (FD_ISSET(i, &pollwset))
            {
                if (pd == NULL)
                {
                    pd = newT(pollDesc);
                }
                pd->fd = i;
                pd->w = true;
            }
            if (pd != NULL)
            {
                list_add(list, &pd->list);
            }
        }
    }
    return list;
}

#endif