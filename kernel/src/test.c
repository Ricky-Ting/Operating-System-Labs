#include <common.h>
#include <klib.h>

void mytest(void){
	char * * s = pmm->alloc(100);
	assert(s!=NULL);
	*s = "This is s\n";
	printf("s=%d\n",s);
	//*a = 5;
	//assert( (*a == 5));
	//pmm->free(s);
	int * b = pmm->alloc(sizeof(int));
	assert(b!=NULL);
	printf("b=%d\n",b);	
	printf("hello\n");	
	//pmm->free(s);
	pmm->free(b);


	int * c = pmm->alloc(sizeof(int));
	printf("c=%d\n",c);
	pmm->free(c);
	pmm->free(s);
	return;
}
