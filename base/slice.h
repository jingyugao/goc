#ifndef SLICE_H
#define SLICE_H
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct {
	void *ptr;
	size_t used;
	size_t cap;
} slice;

#define append(sli, elemptr)                                                   \
	append_slice(sli, (void *)elemptr, sizeof(*elemptr))

#define slice_len(sli, type) len_slice(sli, sizeof(type))
#define slice_get(sli, index, type)                                            \
	(type *)index_slice(sli, index, sizeof(type))

#define slice_end(sli, type) (type *)end_slice(sli, sizeof(type))

#define slice_pop(sli, num, type) pop_slice(sli, num, sizeof(type))

#define slice_new(type, used, cap)                                             \
	make_slice(used * sizeof(type), cap * sizeof(type));

static slice make_slice(size_t used, size_t cap)
{
	void *ptr = malloc(cap);
	memset(ptr, 0, cap);
	slice sli = { ptr, used, cap };
	return sli;
}

static slice grow_slice(slice sli, size_t grow)
{
	if (sli.used + grow <= sli.cap) {
		return sli;
	}
	size_t newcap = 1.2 * (sli.cap + grow);
	void *ptr = malloc(newcap);
	memset(ptr, 0, newcap);
	if (sli.ptr && sli.used > 0) {
		memcpy(ptr, sli.ptr, sli.used);
	}
	sli.cap = newcap;
	sli.ptr = ptr;
	return sli;
}

static slice append_slice(slice sli, void *elemptr, size_t elemsize)
{
	sli = grow_slice(sli, elemsize);
	memcpy(sli.ptr + sli.used, elemptr, elemsize);
	sli.used += elemsize;
	return sli;
}

static void *index_slice(slice sli, int index, size_t elemsize)
{
	assert(index * elemsize < sli.used);
	assert(index >= 0);
	return sli.ptr + index * elemsize;
}

static size_t len_slice(slice sli, size_t elemsize)
{
	return (sli.used / elemsize);
}

static void *end_slice(slice sli, int elemsize)
{
	return index_slice(sli, len_slice(sli, elemsize) - 1, elemsize);
}

static slice pop_slice(slice sli, int num, int elemsize)
{
	sli.used -= elemsize * num;
	return sli;
}

#endif