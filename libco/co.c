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

struct co {
	ucontext_t uc;
	int co_index;
	char __stack[30 MB];	
};

struct co * co_array[CO_MAX+1];
int co_counter;
int current;
void co_init() {
	srand(time(NULL));
	co_counter=0;
	current =-1;
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
	makecontext(&(new_co->uc), (void(*) (void ) )func, 1, (void *)arg);
	new_co->co_index = co_counter;
	assert(co_counter<100);
	co_array[co_counter++] = new_co;
	printf("co_start: create %s, counter=%d",name,counter);
  return new_co;
}

void co_yield() {
	int ccurrent = current;
	int next_co;
	do {
		next_co = rand()%co_counter;
	} while(co_array[next_co]==NULL);
	current = next_co;
	//getcontext(&(co_array[ccurrent]->uc));
	//setcontext(&(co_array[next_co]->uc));	
	swapcontext(&(co_array[ccurrent]->uc)  , &(co_array[next_co]->uc));
	return ;	
}

void co_wait(struct co *thd) {
	int ccurrent = current;
	ucontext_t * thisuc = malloc(sizeof(ucontext_t));  
	(thd->uc).uc_link = thisuc;
	for(int i=0; i<co_counter; i++) {
		if(co_array[i]==thd)
			current = i;
	}
	swapcontext(thisuc, &(thd->uc));
	current = ccurrent;	
	free(thisuc);
	co_array[thd->co_index]=NULL;
	free(thd);
	return;	
}

