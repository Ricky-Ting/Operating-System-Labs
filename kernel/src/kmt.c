#include <common.h>
#include <klib.h>

int holding(spinlock_t *lock);
void pushcli(void);
void popcli(void);

struct {
	int ncli;
	int storedint;
}	int_stack[MAXCPU];

void kmt_init() {
	
	memset(int_stack, 0, sizeof(int_stack));
}


int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void * arg) {
	TRACE_ENTRY;
		
	task->name = name;
			

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
	if(holding(lk))
		panic("acquired!");	
	
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
	if(!holding(lk))
		panic("release");

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
	if(_intr_read())
		panic("popcli - interruptible");
	if(--int_stack[_cpu()].ncli < 0)	
		panic("popcli");
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
