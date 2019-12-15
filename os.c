#include "runtime.h"
#include <pthread.h>
#include <stdlib.h>
#include"time2.h"
#include"runtime2.h"
void mstart_stub(m *mp)
{
	settls(&mp->tls);
	mp->tls.ptr[0] = (uintptr)mp->g0;
	mstart();
}

void sysmon()
{
	int timeslice=10*Millisecond;
	while (1) {
		usleep(200);
		if(!preempt_enable()){
			continue;
		}
		int64 now=nanotime();
		for (int i = 0; i < MAXPROC; i++) {
			p *_p_=allp[i];
			if(!_p_){
				printf("p nil\n");
				continue;
			}
			if(_p_->sched_when+timeslice<now){
				m*_m_=_p_->m;
				if(!_m_){
					printf("m nil\n");
					continue;
				}
				if (_m_->curg){
					_m_->curg->preempt=true;
				}
			}
		}
	}
}

void newosproc(m *mp)
{
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	uintptr stacksize;
	if (pthread_attr_getstacksize(&attr, &stacksize) != 0) {
		debugf("pthread_attr_getstacksize error\n");
		exit(1);
	}
	// fp->stack.hi = stacksize; //???

	if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0) {
		printf("pthread_attr_setdetachstate error\n");
		exit(1);
	}

	if (pthread_create(&mp->thread, &attr, (void *)mstart_stub, mp) != 0) {
		debugf("pthread_create error\n");
		exit(1);
	}
}