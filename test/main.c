#include "../runtime.h"
#include "../time2.h"
#include "test.h"
#include "../sync.h"

semaphore sema;
int semaval;

int num = 0;
_Atomic int atmicnum = 0;

void f()
{
	printf("f on g%d\n", getg()->id);
	for (int i = 0; i < 1000; i++) {
		// usleep(500 * 1000);
		num++;
		atomic_fetch_add(&atmicnum, 1);
		printf("down\n");
		semaphore_down(&sema);
		printf("timeSleep\n");
		timeSleep(1);
		semaval++;
		printf("up\n");
		semaphore_up(&sema);
		printf("up end");
	}
}

int main() __asm__("_main_main");

// user main go routinue
int main()
{
	semaphore_init(&sema, 1);
	for (int i = 0; i < 1000; i++) {
		go(f, NULL);
	}
	// f();
	timeSleep(10 * Second);
	printf("%d,%d,%D\n", num, atmicnum, semaval);
	assert(num <= atmicnum);

	test_ok;
}
