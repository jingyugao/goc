#include "../runtime.h"
#include "../time2.h"
#include "test.h"
#include "../sync.h"

semaphore sema;
int semaval = 0;

semaphore waitgroup;
int num = 0;
_Atomic int atmicnum = 0;

void f()
{
	for (int i = 0; i < 500; i++) {
		// usleep(500 * 1000);
		// printf("runing\n");
		num++;
		atomic_fetch_add(&atmicnum, 1);
		semaphore_down(&sema);
		timeSleep(1);
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

	printf("%d,%d,%D\n", num, atmicnum, semaval);
	assert(num <= atmicnum);

	test_ok;
}
