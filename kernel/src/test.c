#include <common.h>
#include <klib.h>

void mytest(void){
	int * a = pmm->alloc(sizeof(int));
	assert(a!=NULL);
	printf("a=%d\n",a);
	*a = 5;
	assert( (*a == 5));
	return;

	int * b = pmm->alloc(sizeof(int));
	assert(b!=NULL);
	printf("b=%d\n",b);	
	


}
