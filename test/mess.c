#include "../mess.h"
#include "test.h"
typedef struct {
	int xx;
} XX;
void xxx()
{
	panicf("xx%d", 1);
};

int main()
{
	XX x;
	XX *pX = container_of(&x.xx, XX, xx);
	assert(pX = &x);
	xxx();

	test_ok;
}