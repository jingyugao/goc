#include "test.h"

int main()
{
	int a = 1;
	int *b = NULL;
	memcpy(&a, b, 0);
	printf("xx");
	return 0;
}