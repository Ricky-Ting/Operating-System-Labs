#include <stdio.h>
#include <assert.h>
#include "co.h"
#include <ucontext.h>
#include <time.h>
#include <stdlib.h>
#define KB * 1024LL
#define MB KB * 1024LL
#define GB MB * 1024LL

#define CO_MAX 100
ucontext_t  main_uc; 

struct co {
	ucontext_t uc;
	int co_index;
	char thread_name[20];
	char __stack[100 MB];	
};

struct co * co_array[CO_MAX];
int co_counter;
ucontext_t * current;
void co_init() {
	srand(time(NULL));
	co_counter=0;
	current = &main_uc;
	for(int i=0; i<CO_MAX; i++) 
		co_array[i]=NULL;

	
	printf("Has inited\n");
}

struct co* co_start(const char *name, func_t func, void *arg) {
  //func(arg); // Test #2 hangs
	struct co* new_co = malloc(sizeof(struct co));
	getcontext(&(new_co->uc));	
	new_co->uc.uc_stack.ss_sp = new_co->__stack;
	new_co->uc.uc_stack.ss_size = sizeof(new_co->__stack);
	new_co->uc.uc_link = &main_uc;
	makecontext(&(new_co->uc), (void(*) (void ) )func, 1, (void *)arg);
	new_co->co_index = co_counter;
	sprintf(new_co->thread_name,"%s",name);
	assert(co_counter<CO_MAX);
	co_array[co_counter++] = new_co;
	printf("co_start: create %s, counter=%d\n",name,co_counter);
	swapcontext(&main_uc, &(new_co->uc));
  return new_co;
}

void co_yield() {
	ucontext_t * ccurrent = current;
	int next_co;
	do {
		next_co = rand()%co_counter;
		
	} while(co_array[next_co]==NULL);
		
	
	current = &(co_array[next_co]->uc);
	swapcontext(ccurrent  , current);
	return ;	
}

void co_wait(struct co *thd) {
	ucontext_t * ccurrent = current;	
	//printf("In co_wait: wait for %s\n",thd->thread_name);
	current = &(thd->uc);
	//getcontext(&thisuc);
	printf("In co_wait: wait for %s\n",thd->thread_name);


	int ret=swapcontext(ccurrent, current);
	assert(ret!=-1);
	printf("In co_wait: %s returned\n", thd->thread_name);	
	current = ccurrent;	
	co_array[thd->co_index]=NULL;
	free(thd);
	return;	
}

