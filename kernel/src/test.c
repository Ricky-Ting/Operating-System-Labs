#include <common.h>
#include <klib.h>

void mytest(void){
	int * a = pmm->alloc(sizeof(int));
	assert(a!=NULL);
	//s = "This is s\n";
	printf("a=%d\n",a);
	//*a = 5;
	//assert( (*a == 5));

	int * b = pmm->alloc(sizeof(int));
	assert(b!=NULL);
	printf("b=%d\n",b);	
	printf("hello\n");	
	pmm->free(s);
	pmm->free(b);
	return;
}
