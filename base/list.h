#ifndef LIST_H
#define LIST_H

#include <stdbool.h>
#include <stdlib.h>
struct listhead;
typedef struct listhead {
	struct listhead *prev;
	struct listhead *next;
} listhead;

#define list_first_entry(ptr, type, member)                                    \
	list_entry((ptr)->next, type, member)

#define list_entry(ptr, type, member) container_of(ptr, type, member)

static inline void list_head_init(listhead *list)
{
	list->next = list;
	list->prev = list;
}

static inline void __list_add(listhead *n, listhead *prev, listhead *next)
{
	next->prev = n;
	n->next = next;
	n->prev = prev;
	prev->next = n;
}

static inline void list_add(listhead *n, listhead *head)
{
	__list_add(n, head, head->next);
}

static inline void list_add_tail(listhead *n, listhead *head)
{
	__list_add(n, head->prev, head);
}

static inline void __list_del(struct listhead *prev, struct listhead *next)
{
	next->prev = prev;
	next = prev->next;
}

static inline void list_del(struct listhead *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = NULL;
	entry->prev = NULL;
}

static inline bool list_empty(const struct listhead *head)
{
	return head->next == head;
}

#define list_for_each(pos, head)                                               \
	for (listhead *pos = (head)->next; pos != (head); pos = pos->next)

#endif