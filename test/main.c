#include "../runtime.h"
#include "../time2.h"
#include "test.h"
#include "../sync.h"

//-------------------------------------------------------
// todo:move to runtime_api.h
int main() __asm__("_main_main");
void __attribute__((no_instrument_function))
__cyg_profile_func_enter(void *this_func, void *call_site)
{
	// printf("%s\n",this_func);
}

void __attribute__((no_instrument_function))
__cyg_profile_func_exit(void *this_func, void *call_site)
{
	
}
//-------------------------------------------

semaphore sema;
int semaval = 0;

semaphore waitgroup;
int num = 0;
_Atomic int atomicnum = 0;

void f()
{
	for (int i = 0; i < 5000; i++) {
		num++;
		atomic_fetch_add(&atomicnum, 1);
		semaphore_down(&sema);
		semaval++;
		semaphore_up(&sema);
	}
	semaphore_up(&waitgroup);
}

// user main go routinue
int main()
{
	semaphore_init(&sema, 1);
	semaphore_init(&waitgroup, 0);
	int n = 20;
	for (int i = 0; i < n; i++) {
		go(f, NULL);
		timeSleep(Second);
	}

	for (int i = 0; i < n; i++) {
		semaphore_down(&waitgroup);
	}

	printf("%d,%d,%D\n", num, atomicnum, semaval);
	assert(num <= atomicnum);

	test_ok;
}
