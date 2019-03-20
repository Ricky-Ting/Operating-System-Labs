#include <common.h>
#include <klib.h>

void mytest(void){
	char * s = pmm->alloc(100);
	assert(a!=NULL);
	s = "This is s\n";
	printf("a=%d\n",a);
	printf("%c",s);
	//*a = 5;
	//assert( (*a == 5));

	int * b = pmm->alloc(sizeof(int));
	assert(b!=NULL);
	printf("b=%d\n",b);	
	printf("hello\n");	
	pmm->free(a);
	pmm->free(b);
	return;
}
