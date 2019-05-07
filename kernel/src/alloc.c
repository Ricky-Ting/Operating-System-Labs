#include <common.h>
#include <klib.h>

static uintptr_t pm_start, pm_end;

#define U(x)  ((uintptr_t)(x))
//#define ALI_F(x) ( ( U(x) & 0x7)?( ( ( U(x) >>3)<<3)+ 0x8 ) : U(x)  )
//#define ALI_F(x) func(U(x)) 
#define SIZE(x)  (sizeof(x))

struct node_t{
	struct node_t * next;
	struct node_t * prev;
	int size;
	int used;
};


struct node_t * myhead;
//int mylock;
spinlock_t alloc_lk;
uintptr_t func(uintptr_t x) {
	return (x & 0x7)? (((x>>3)<<3)+0x8) : x;
} 


static void pmm_init() {
  pm_start = (uintptr_t)_heap.start;
  pm_end   = (uintptr_t)_heap.end;
	printf("In pmm_init(): pmstart=%d, pmend=%d\n",pm_start,pm_end);
	myhead = (struct node_t *)(func(U(pm_start)));
	myhead->next= myhead->prev = NULL;
	myhead->used = 0;
	myhead->size = pm_end - func(U(pm_start)) - func(U(SIZE(struct node_t))) ;
	//mylock=0;
	kmt->spin_init(&alloc_lk, "alloc-lock");
}


int atomic_xchg(volatile int *addr, int newval) {
	int result;
	asm volatile ("lock xchg %0, %1": "+m"(*addr), "=a"(result): "1"(newval) : "cc");
	return result;
}

void lock(int * lk) {
	//cli();
	while(atomic_xchg(lk,1))  ;
}

void unlock(int * lk) {
	atomic_xchg(lk, 0);
	//sti();
}

int kcounter = 0;

static void *kalloc(size_t size) {
	//lock(&mylock);
	kmt->spin_lock(&alloc_lk);
	printf("In kalloc\n");
	kcounter++;
	void * ret=NULL;
	struct node_t * tmp = myhead;
	while( (tmp!=NULL) && ( tmp->size < size ||  tmp->used==1) ) {
		tmp=tmp->next;			
	}
	if(tmp!=NULL) {
		ret = (void *)(tmp) + func( U(SIZE(struct node_t)) );
		tmp->used = 1;
		if(  ((tmp->next==NULL)?pm_end:(uintptr_t)(tmp->next)) - (uintptr_t)ret - size > 2* func(U(SIZE(struct node_t)))   ) {
			struct node_t * tmp2 = (struct node_t *)func(U(ret+size));
			tmp2->next = tmp->next;
			if(tmp2->next != NULL)
				tmp2->next->prev = tmp2;
			tmp2->used = 0;
			tmp->next = tmp2;
			tmp2->size = ((tmp2->next==NULL)?pm_end:(uintptr_t)(tmp2->next))  - func( U(tmp2) ) ;
		}
		
	}
	assert(kcounter==1);
	kcounter--; 
	printf("Out kalloc\n");
	//unlock(&mylock);
	kmt->spin_unlock(&alloc_lk);
	if(ret == NULL) {
		printf("Out of memory\n");
		assert(0);
	}
	return ret;
}

static void kfree(void *ptr) {
	if(ptr==NULL)
		return;
	//lock(&mylock);
	kmt->spin_lock(&alloc_lk);
	struct node_t * midd = (struct node_t *)(ptr - func(U(SIZE(struct node_t))) );
	midd->used = 0;
	struct node_t * tmp = midd;
	while( tmp->next!=NULL && tmp->next->used==0  ) {
		tmp->next = tmp->next->next;
		if(tmp->next->next!=NULL) 
			tmp->next->next->prev = tmp;
		tmp->size =  ((tmp->next->next==NULL)?pm_end:(uintptr_t)(tmp->next->next)) - func( U(tmp) );
		
	}
	tmp = midd;
	while( tmp->prev!=NULL && tmp->prev->used==0  ) {
		struct node_t * tmp2 = tmp->prev;
		tmp2->next = tmp->next;
		if(tmp->next!=NULL)
			tmp->next->prev = tmp2;	
		tmp2->size =  ((tmp->next==NULL)?pm_end:(uintptr_t)(tmp->next)) - func( ( U(tmp2) ) );
		tmp = tmp2;
	}
	//unlock(&mylock);
	kmt->spin_unlock(&alloc_lk);
	return;
}

MODULE_DEF(pmm) {
  .init = pmm_init,
  .alloc = kalloc,
  .free = kfree,
};
