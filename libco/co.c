#include <stdio.h>
#include <assert.h>
#include "co.h"
#include <ucontext.h>
#include <time.h>
#include <stdlib.h>
#define KB * 1024LL
#define MB KB * 1024LL
#define GB MB * 1024LL

#define CO_MAX 10
struct co {
	ucontext_t uc;
	int co_index;
	int completed;
	struct co  * wait;
	char name[20];
	char __stack[8 KB] __attribute__((aligned (16)));	
};

int has_thread;
struct co  *  co_array[CO_MAX];
struct co co_main;
int co_counter;
struct co * co_current;
void  myfunc(func_t func, void *arg){
	func(arg);
	co_current -> completed =1;
	//printf("%s has completed\n",co_current->name);
	has_thread--;
	co_yield();	
}




void co_init() {
	srand(time(NULL));
	co_counter=0;
	co_current = &(co_main);
	getcontext(&(co_main.uc));
	co_main.wait = NULL;
	//co_main.name = "main";
	sprintf(co_main.name,"main");
	for(int i=0; i<CO_MAX; i++) 
		co_array[i]=NULL;
	has_thread = 0;
	
	//printf("Has inited\n");
}

struct co* co_start(const char *name, func_t func, void *arg) {
  //func(arg); // Test #2 hangs
	struct co* new_co = malloc(sizeof(struct co));
	getcontext(&(new_co->uc));	
	new_co->wait=NULL;
	new_co->completed=0;
	new_co->uc.uc_stack.ss_sp = new_co->__stack;
	new_co->uc.uc_stack.ss_size = sizeof(new_co->__stack);
	//new_co->uc.uc_link = &(co_main.uc);
	//void * run_func = &(myfunc(func, arg));
	makecontext(&(new_co->uc),  (void (*) (void))myfunc, 2, func ,(void *)arg);
	has_thread ++;
	new_co->co_index = co_counter;
	sprintf(new_co->name,"%s",name);
	assert(co_counter<CO_MAX);
	co_array[co_counter++] = new_co;
	//printf("co_start: create %s, counter=%d\n",name,co_counter);
	co_current = new_co;
	swapcontext(&(co_main.uc), &(new_co->uc));
	//printf("Here\n");
  return new_co;
}

void co_yield() {
	
	struct co *  co_ccurrent = co_current;
	int next_co;
	do {
		next_co = rand()%co_counter;
	} while( (co_array[next_co]==NULL || co_array[next_co]->completed==1  || (co_array[next_co]->wait!=NULL  && co_array[next_co]->wait->completed==0) ) && has_thread>0   );
		
	co_current = (co_array[next_co]);
		
	if( (co_main.wait==NULL || co_main.wait->completed==1) ) {
		if( (rand()%(co_counter)) == 0 ) {
			co_current= &co_main;
			//printf("In\n");
			assert( &(co_current->uc) == (&(co_main.uc)));
		}
		//printf("In main_yield\n");
	}	
	if(has_thread ==0)
		co_current = &co_main;
	//printf("In co_yield: swap %s and %s\n",co_ccurrent->name, co_current->name);
	swapcontext( &(co_ccurrent->uc)  , &(co_current->uc) );

	return ;	
}

void co_wait(struct co *thd) {
	if(co_array[thd->co_index]==NULL)
			assert(0);
	if(co_array[thd->co_index]->completed ==1) {
		co_array[thd->co_index]=NULL;
		free(thd);
		return ; 
	}
	co_current -> wait = thd;
	struct co * co_ccurrent = co_current;	
	//printf("In co_wait: wait for %s\n",thd->name);
	co_current = thd;
	//getcontext(&thisuc);
	//printf("In co_wait: wait for %s\n",thd->thread_name);
	int ret=swapcontext( &(co_ccurrent->uc), &(co_current->uc));
	assert(ret!=-1);
	//printf("In co_wait: %s returned\n", thd->name);	
	co_current -> wait = NULL;
	co_array[thd->co_index] =NULL;
	free(thd);
	return;	
}

