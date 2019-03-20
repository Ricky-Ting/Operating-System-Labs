#include <common.h>
#include <klib.h>

void mytest(void){
	int * a = pmm->alloc(sizeof(int));
	assert(a!=NULL);
	printf("%d\n",a);
	*a = 5;
	assert( (*a == 5));
	return;
	
	


}
