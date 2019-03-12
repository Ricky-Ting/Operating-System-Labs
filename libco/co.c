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
ucontext_t  thisuc; //= malloc(sizeof(ucontext_t));  
	ucontext_t  beforeuc; //= malloc(sizeof(ucontext_t));  

struct co {
	ucontext_t uc;
	int co_index;
	char thread_name[20];
	char __stack[100 MB];	
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
	new_co->uc.uc_link = &thisuc;
	makecontext(&(new_co->uc), (void(*) (void ) )func, 1, (void *)arg);
	new_co->co_index = co_counter;
	sprintf(new_co->thread_name,"%s",name);
	assert(co_counter<100);
	co_array[co_counter++] = new_co;
	printf("co_start: create %s, counter=%d\n",name,co_counter);
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
	//printf("In co_wait: wait for %s\n",thd->thread_name);
	//ucontext_t * thisuc = malloc(sizeof(ucontext_t));  
	//ucontext_t * beforeuc = malloc(sizeof(ucontext_t));  
	(thd->uc).uc_link = &thisuc;
	for(int i=0; i<co_counter; i++) {
		if(co_array[i]==thd)
			current = i;
	}
	for(int i=0; i<co_counter; i++) {
		if(co_array[i]!=NULL && i!=current) {
			(co_array[i]->uc).uc_link = &beforeuc;
		}	
	}
	getcontext(&beforeuc);
	getcontext(&thisuc);
	printf("In co_wait: wait for %s\n",thd->thread_name);


	int ret=setcontext(&thisuc, &(thd->uc));
	assert(ret!=-1);
	printf("In co_wait: %s returned\n", thd->thread_name);	
	for(int i=0; i<co_counter; i++) {
		if(co_array[i]!=NULL && i!=current) {
			(co_array[i]->uc).uc_link = NULL;
		}	
	}
	current = ccurrent;	
	//free(thisuc);
	//free(beforeuc);
	co_array[thd->co_index]=NULL;
	free(thd);
	return;	
}

