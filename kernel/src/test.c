#include <common.h>
#include <klib.h>

void mytest(void){
	int * a = pmm->alloc(sizeof(int));
	assert(a!=NULL);
	*a = 5;
	assert( (*a == 5));
	return;
	
	


}
