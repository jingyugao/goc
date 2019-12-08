#ifndef MESS_H
#define MESS_H
#include "base/list.h"
#include "base/vector.h"
#include "type.h"
#include <errno.h>
#include <execinfo.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdatomic.h>
static void *zalloc(size_t size)
{
	void *p = malloc(size);
	memset(p, 0, size);
	return p;
}

#define newT(T) (T *)(zalloc(sizeof(T)))

static void panic(error err)
{
	void *array[10];
	size_t size;

	// get void*'s for all entries on the stack
	size = backtrace(array, 10);
	printf("%s\n", err.str);
	backtrace_symbols_fd(array + 2, size, STDERR_FILENO);
	exit(1);
}

#define panicf(format, arg...) __panicf(__FILE__, __LINE__, format, ##arg)

#define debugf(format, arg...) printf("%s %d" format, __FILE__, __LINE__, ##arg)

static void __panicf(const char *fileName, int line, const char *format, ...)
{
	error err;
	int n = sprintf(err.str, "%s:%d: ", fileName, line);
	va_list args;
	va_start(args, format);
	n = vsprintf(err.str + n, format, args);
	va_end(args);
	panic(err);
}
#define container_of(ptr, type, member)                                        \
	(type *)((void *)ptr - (void *)(&((type *)(NULL))->member))
#endif