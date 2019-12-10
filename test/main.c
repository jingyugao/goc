#include "../runtime.h"
#include "../time2.h"
#include "test.h"
#include "../sync.h"

semaphore sema;
int semaval = 0;

semaphore waitgroup;
int num = 0;
_Atomic int atomicnum = 0;

void f()
{
	for (int i = 0; i < 5000; i++) {
		// usleep(500 * 1000);
		int t = atomic_fetch_add(&atomicnum, 1);
		// printf("runing %d\n",t );
		num++;

		semaphore_down(&sema);
		// timeSleep(1);
		Gosched();
		semaval++;
		semaphore_up(&sema);
	}
	semaphore_up(&waitgroup);
}

int main() __asm__("_main_main");
// user main go routinue
int main()
{
	semaphore_init(&sema, 1);
	semaphore_init(&waitgroup, 0);
	int n = 10;
	for (int i = 0; i < n; i++) {
		go(f, NULL);
	}

	for (int i = 0; i < n; i++) {
		semaphore_down(&waitgroup);
	}

	printf("%d,%d,%D\n", num, atomicnum, semaval);
	assert(num <= atomicnum);

	test_ok;
}
