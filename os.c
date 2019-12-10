#include "runtime.h"
#include <pthread.h>
#include <stdlib.h>

void mstart_stub(m *mp)
{
	settls(&mp->tls);
	mp->tls.ptr[0] = (uintptr)mp->g0;
	mstart();
}

void sysmon()
{
	while (1) {
		printf("sysmon\n");
		sleep(1);
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