#ifndef TIMER_H
#define TIMER_H
#include "runtime.h"
#include "time2.h"

typedef struct {
	Func fn;
	int64 when;
} timer;

static void siftdown_timer(vector *v, int i);
static void siftup_timer(vector *v, int i);
static void push_timers(vector *v, timer *t)
{
	vector_add(v, t);
	siftup_timer(v, vector_count(v) - 1);
}

static timer *pop_timers(vector *v)
{
	int n = vector_count(v);
	if (n == 0) {
		return NULL;
	}
	timer *ret = vector_get(v, 0);
	vector_swap(v, 0, n - 1);
	vector_delete(v, n - 1);
	siftdown_timer(v, 0);

	return ret;
}

static void siftup_timer(vector *v, int i)
{
	assert(i < vector_count(v));

	timer *t = (timer *)vector_get(v, i);
	int64 w = t->when;
	while (1) {
		if (i == 0) {
			break;
		}
		int par_idx = (i - 1) / 2;
		timer *par = (timer *)vector_get(v, par_idx);
		if (par->when < t->when) {
			break;
		}
		vector_swap(v, par_idx, i);
		i = par_idx;
	}
	return;
}

static void siftdown_timer(vector *v, int i)
{
	if (vector_count(v) == 0) {
		return;
	}
	assert(i < vector_count(v));

	int last_idx = vector_count(v) - 1;
	while (1) {
		timer *t = (timer *)vector_get(v, i);
		int l_idx = 2 * i + 1;
		int r_idx = 2 * i + 2;

		if (l_idx > last_idx) {
			break;
		}
		timer *lt = (timer *)vector_get(v, l_idx);
		if (l_idx == last_idx) {
			if (t->when > lt->when) {
				vector_swap(v, i, l_idx);
			}
			break;
		}
		timer *rt = (timer *)vector_get(v, r_idx);

		if (t->when < lt->when && t->when < rt->when) {
			break;
		}

		if (lt->when < rt->when) {
			vector_swap(v, l_idx, i);
			i = l_idx;
		} else {
			vector_swap(v, r_idx, i);
			i = r_idx;
		}
	}
	return;
}

#endif