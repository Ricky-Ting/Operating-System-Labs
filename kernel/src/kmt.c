#include <common.h>
#include <klib.h>
int holding(spinlock_t *lock);
void pushcli(void);
void popcli(void);
_Context* kmt_context_save(_Event event, _Context * context);
_Context* kmt_context_switch(_Event event, _Context * context);
void kmt_spin_init(spinlock_t *lk, const char *name);
void kmt_spin_lock(spinlock_t *lk);
void kmt_spin_unlock(spinlock_t *lk);
struct {
	int ncli;
	int storedint;
}	int_stack[MAXCPU];

task_t* current_task[MAXCPU];
#define current (current_task[_cpu()]) 

task_t* task_head[MAXCPU];
task_t* task_tail[MAXCPU];
spinlock_t create_lock;
int next_cpu = 0;
void kmt_init() {
	
	for(int i=0; i<MAXCPU; i++) {
		current_task[i] = NULL;	
		task_head[i] = NULL;
		task_tail[i] = NULL;
	}
	memset(int_stack, 0, sizeof(int_stack));
	kmt_spin_init(&create_lock, "create-lock");
	os->on_irq(INT32_MIN, _EVENT_NULL, kmt_context_save); 
	os->on_irq(INT32_MAX, _EVENT_NULL, kmt_context_switch);
}

_Context* kmt_context_save(_Event event, _Context * context) {
	assert(0);

	if(current)	
		current->context = context;
	return context;				
}

_Context* kmt_context_switch(_Event event, _Context * context) {
	task_t *iter = task_head[_cpu()];
	/*
	for(int i=0; i<MAXCPU; i++) {
		printf("CPU %d:\n",i);
		task_t * it = task_head[i];
		while(it!=NULL) {
			printf("%s \n", it->name);
			it = it->next;
		}
	}
	*/
	//assert(0);
	//printf("This is cpu %d\n",_cpu());
 	//printf("List \n");

	while(iter!=NULL && iter->status!=TASK_READY) {
		//printf("%s\n",iter->name);
		iter = iter->next;
	}
	if(iter==NULL) {
		//printf("Schedule %s\n",current->name);
		return context;	
	}	else {
		if(task_tail[_cpu()]!=iter) {
			if(task_head[_cpu()] == iter && iter->next!=NULL) {
				//printf("Here\n");
				task_head[_cpu()] = iter->next;
			}
			if(iter->prev!=NULL)
				iter->prev->next = iter->next;
			if(iter->next!=NULL)
				iter->next->prev = iter->prev;
			iter->next = NULL;
			if(task_tail[_cpu()] != iter) {
				iter->prev = task_tail[_cpu()];
				task_tail[_cpu()]->next = iter;
			}
			task_tail[_cpu()] = iter;
		}	
		current = iter;
		printf("Schedule %s\n",current->name);
		return (iter->context);	
	}
}


int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void * arg) {
	TRACE_ENTRY;
	kmt_spin_lock(&create_lock);
	printf("In lock\n");
	task->bind_cpu = next_cpu % _ncpu();	
	//next_cpu = (next_cpu + 1) % _ncpu();
	printf("CPU: %d\n", _ncpu());
	printf("next : %d\n", next_cpu);
	//rand();

	printf("%s on cpu%d\n",name, task->bind_cpu);
	_Area this_stack;
	task->name = name;
	task->stack = pmm->alloc(STACK_SIZE);
	this_stack.start = task->stack;
	this_stack.end = task->stack + STACK_SIZE; 
		
		//task->bind_cpu = _cpu();	
	task->status = TASK_READY;
	task->context = _kcontext(this_stack, entry, arg); 
	
	task->next = task_head[task->bind_cpu];
	task->prev = NULL;
	if(task_head[task->bind_cpu] != NULL) {
		task_head[task->bind_cpu]->prev = task;
	} else {
		task_tail[task->bind_cpu] = task; 
	}
	task_head[task->bind_cpu] = task;			
	
	kmt_spin_unlock(&create_lock);
	printf("Out lock\n");
	return 1;
	TRACE_EXIT;
}

void kmt_teardown(task_t *task) {
	pmm->free(task->stack);
}


void kmt_spin_init(spinlock_t *lk, const char *name) {
	lk->name = name;
	lk->locked = 0;
	lk->cpu = -1;
}

void kmt_spin_lock(spinlock_t *lk) {
	//TRACE_ENTRY;
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

	//printf("%s get spin lock %s\n",current->name, lk->name);
	// Record info about lock acquisition for debugging.
	lk->cpu = _cpu();
		
	//getcallerpcs(&lk, lk->pcs);
	//TRACE_EXIT;	
}

void kmt_spin_unlock(spinlock_t *lk) {
	//TRACE_ENTRY;
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
	//printf("%s realease spin lock %s\n", current->name, lk->name);	
	popcli();
	//TRACE_EXIT;
}

void kmt_sem_init(sem_t *sem, const char *name, int value){
	kmt_spin_init(&sem->lock, name);
	sem->count = value;
	sem->name = name;
	for(int i=0;i<MAXQ;i++)
		sem->queue[i]=NULL;
	sem->head = 0;
	sem->tail = 0;
	//printf("Init %s to %d\n",name, value);
}

void kmt_sem_wait(sem_t *sem) {
	kmt_spin_lock(&sem->lock);
	TRACE_ENTRY;
	sem->count--;
	//printf("Here %d %d\n",sem->tail, sem->head);
	if(sem->count<0) {
		// In queue
		if(current->status!=TASK_SLEEP) {
			sem->queue[sem->tail] = current;
			if( (sem->tail + 1)%MAXQ == sem->head ) {
				printf("%d %d %s\n",sem->tail, sem->head, sem->name);
				assert(0);
			}
			sem->tail = (sem->tail + 1) % MAXQ;
			//printf("%d %d %s\n",sem->tail, sem->head, sem->name);

			current->status = TASK_SLEEP;	
			//printf("%s to sleep\n", current->name);
		}
		else 
			assert(0);
		TRACE_EXIT;
		kmt_spin_unlock(&sem->lock);
		while(current->status == TASK_SLEEP)
			_yield();
		kmt_spin_lock(&sem->lock);
		TRACE_ENTRY;
	}
	TRACE_EXIT;
	printf("%s get the lock %s\n", current->name, sem->name);
	kmt_spin_unlock(&sem->lock);
}

void kmt_sem_signal(sem_t *sem) {
	TRACE_ENTRY;
	kmt_spin_lock(&sem->lock);
	sem->count++;
	if(sem->queue[sem->head]!=NULL) {
		assert(sem->count<=0);
		sem->queue[sem->head]->status = TASK_READY;
		//printf("Wake %s\n", sem->queue[sem->head]->name);
		sem->queue[sem->head] = NULL;
		sem->head = (sem->head + 1) % MAXQ;
		//printf("Here %d %d %s\n",sem->tail, sem->head, sem->name);

	}	
	printf("%s release the lock %s\n", current->name, sem->name);

	kmt_spin_unlock(&sem->lock);
	TRACE_ENTRY;
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
