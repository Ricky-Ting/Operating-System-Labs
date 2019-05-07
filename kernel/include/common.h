#ifndef __COMMON_H__
#define __COMMON_H__

#include <kernel.h>
#include <nanos.h>
#include <trace.h>


#define STACK_SIZE (4096*1024)
#define MAXCPU 16
typedef unsigned int uint;
#define TASK_READY 0
#define TASK_RUNNING 1
#define TASK_SLEEP 2
#define MAXQ 20000
struct task {
	const char *name;
	_Context *context;
	int bind_cpu;
	int status; /* 0: ready  1: running 2: sleep*/
	uint8_t fence1[32];
	//uint8_t stack[STACK_SIZE];
	void * stack;
	uint8_t fence2[32];
	struct task * prev;
	struct task * next;	
};
struct spinlock {
	int locked; // Is the lock held?
	
	// For debugging:
	const char *name;				//Name of the lock
	int cpu;	// The cpu holding the lock;
	uint pcs[10]; 		// The call stack (an array of program counters)
										// that locked the lock.
	
};
struct semaphore {
	spinlock_t lock;
	int count;
	const char *name;
	task_t* queue[MAXQ];		
	int head;
	int tail;
};



struct handler_node {
	int seq;
	int event;
	handler_t handler;	
	struct handler_node* next;
};
#endif
