#include "../mess.h"
#include<assert.h>
typedef struct {
    int xx;
}XX;
void xxx(){};
int main(){
    XX x;
    XX*pX=container_of(&x.xx,XX,xx);
    assert(pX=&x);
    return 0;
}