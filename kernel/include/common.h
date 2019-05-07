#ifndef __COMMON_H__
#define __COMMON_H__

#include <kernel.h>
#include <nanos.h>
#include <trace.h>


#define STACK_SIZE 4096
#define MAXCPU 16
typedef unsigned int uint;

struct task {
	const char *name;
	_Context context;
	uint8_t fence1[32];
	uint8_t stack[STACK_SIZE];
	uint8_t fence2[32];
};
struct spinlock {
	uint locked; // Is the lock held?
	
	// For debugging:
	char *name;				//Name of the lock
	int cpu;	// The cpu holding the lock;
	uint pcs[10]; 		// The call stack (an array of program counters)
										// that locked the lock.
	
};
struct semaphore {};



struct handler_node {
	int seq;
	int event;
	handler_t handler;	
	struct handler_node* next;
};
#endif
