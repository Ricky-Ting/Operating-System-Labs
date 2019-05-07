#include <common.h>
#include <klib.h>

int holding(spinlock_t *lock);
void pushcli(void);
void popcli(void);
_Context* kmt_context_save(_Event event, _Context * context);
_Context* kmt_context_switch(_Event event, _Context * context);

struct {
	int ncli;
	int storedint;
}	int_stack[MAXCPU];

struct task_t current_task[MAXCPU];
#define current (current_task[_cpu()]) 

struct tasks_on_cpu{
	//int cpu;
	int status;  /* 0:ready, 1:wait*/
	const char *task_name;
	task_t *task;
	struct tasks_on_cpu *next; 
	struct tasks_on_cpu *prev;
};
struct tasks_on_cpu* task_head[MAXCPU];
struct tasks_on_cpu* task_tail[MAXCPU];

void kmt_init() {
	
	for(int i=0; i<MAXCPU; i++) {
		current_task[i] = NULL;	
		task_head[i] = NULL;
		task_tail[i] = NULL;
	}
	memset(int_stack, 0, sizeof(int_stack));

	os->on_irq(INT32_MIN, _EVENT_NULL, kmt_context_save); 
	os->on_irq(INT32_MAX, _EVENT_NULL, kmt_context_switch);
}

_Context* kmt_context_save(_Event event, _Context * context) {
	if(current)	
		current->context = *context;
	return context;				
}

_Context* kmt_context_switch(_Event event, _Context * context) {
	struct tasks_on_cpu *iter = task_head[_cpu()];
  
	while(iter!=NULL && iter->status!=0) {
		iter = iter->next;
	}	
	if(iter==NULL) {
		printf("NO other tasks on this cpu\n");
		return context;
	} else {
		if(iter->prev!=NULL)
			iter->prev->next = iter->next;
		if(iter->next!=NULL)
			iter->next->prev = iter->prev;
		iter->next = NULL;
		iter->prev = task_tail[_cpu()];
		task_tail[_cpu()] = iter;
		return &(iter->task->context);	
	}
}


int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void * arg) {
	TRACE_ENTRY;
		
	task->name = name;
	task->stack = pmm->alloc(STACK_SIZE);
	//task->bind_cpu = rand() % MAXCPU;	
	task->bind_cpu = _cpu();	
	struct tasks_on_cpu *this_task = pmm->alloc(sizeof(struct tasks_on_cpu));
	this_task->task_name = name;
	this_task->task = task;
	this_task->status = 0; 
	
	this_task->next = task_head[task->bind_cpu];
	this_task->prev = NULL;
	if(task_head[task->bind_cpu] != NULL) {
		task_head[task->bind_cpu]->prev = this_task;
	} else {
		task_tail[task->bind_cpu] = this_task; 
	}
	task_head[task->bind_cpu] = this_task;			

	return -1;
	TRACE_EXIT;
}

void kmt_teardown(task_t *task) {

}


void kmt_spin_init(spinlock_t *lk, const char *name) {
	lk->name = name;
	lk->locked = 0;
	lk->cpu = -1;
}

void kmt_spin_lock(spinlock_t *lk) {
	TRACE_ENTRY;
	pushcli();  // disable interrupts to avoid deadlock.
	if(holding(lk)) {
		printf("acquired!\n");
		assert(0);	
	}
	
	// The xchg is atomic
	while(_atomic_xchg(&lk->locked, 1) != 0)
		;

	// Tell the C compiler and the processor to not move loads or stores
	// past this point, to ensure that the critical section's memory
	// references happen after the lock is acquired.
	__sync_synchronize();

	// Record info about lock acquisition for debugging.
	lk->cpu = _cpu();
		
	//getcallerpcs(&lk, lk->pcs);
	TRACE_EXIT;	
}

void kmt_spin_unlock(spinlock_t *lk) {
	TRACE_ENTRY;
	if(!holding(lk)) {
		printf("release\n");
		assert(0);	
	}

	//lk->pcs[0] = 0;
	lk->cpu = -1;

	// Tell the C compiler and the processor to not move loads or stores
	// past this point, to ensure that the critical section's memory
	// references happen after the lock is acquired.
	__sync_synchronize();


	// Release the lock, equivalent to lk->locked = 0.
	// This code can't use a C assignment, since it might
	// not be atomic. A real OS would use C atmoics here.
	asm volatile("movl $0, %0" : "+m" (lk->locked) :);
	
	popcli();
	TRACE_EXIT;
}

void kmt_sem_init(sem_t *sem, const char *name, int value){

}

void kmt_sem_wait(sem_t *sem) {

}

void kmt_sem_signal(sem_t *sem) {

}


int holding(spinlock_t *lock) {
	int r;
	pushcli();
	r = lock->locked && lock->cpu == _cpu();
	popcli();
	return r;
}

void pushcli(void) {
	int original_IF = _intr_read();			
	_intr_write(0);
	if(int_stack[_cpu()].ncli == 0) 
		int_stack[_cpu()].storedint = original_IF;
	int_stack[_cpu()].ncli += 1;
	
}

void popcli(void) {
	if(_intr_read()) {
		printf("popcli - interruptible\n");
		assert(0);
	}
	if(--int_stack[_cpu()].ncli < 0){
		printf("popcli\n");
		assert(0);
	}
	if(int_stack[_cpu()].ncli == 0 && int_stack[_cpu()].storedint)
		_intr_write(1);
}




MODULE_DEF(kmt) {
	.init =	kmt_init, 
	.create = kmt_create,
	.teardown = kmt_teardown,
	.spin_init = kmt_spin_init,
	.spin_lock = kmt_spin_lock,
	.spin_unlock = kmt_spin_unlock,
	.sem_init = kmt_sem_init,
	.sem_wait = kmt_sem_wait,
	.sem_signal = kmt_sem_signal,
};
