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
	if (!preempt_enable()) {
		return;
	}
	g *_g_ = getg();
	if (!_g_->preempt) {
		return;
	}
    _g_->preempt=false;
	Gosched();
}

void __attribute__((no_instrument_function))
__cyg_profile_func_exit(void *this_func, void *call_site)
{
}
//-------------------------------------------

_Atomic int atomicnum = 0;


void ff(){

}

void loop()
{
	for (int i = 0; i < Second/Microsecond; i++) {
		usleep(1);
		#ifdef CALL_FF
		ff();		
		#endif
	}
}

void xx()
{
	printf("test ok:%s\n", __FILE__);                                      
	exit(0);
}

// user main go routinue
int main()
{
	go(xx, NULL);
	loop();
	exit(6);
}
