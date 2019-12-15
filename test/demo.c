#include "test.h"

#ifdef XX
void f(){
	printf("xx\n");
}
#else
void f(){
	printf("no xx\n");
}
#endif

int main()
{
	f();
	return 0;
}