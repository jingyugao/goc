#include "runtime2.h"
#include <stdlib.h>

m *allm;
p *allp[MAXPORC];
pthread_mutex_t allpLock;
