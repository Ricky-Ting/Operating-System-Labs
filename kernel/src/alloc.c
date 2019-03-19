#include <common.h>
#include <klib.h>
#include <pthread.h>

static uintptr_t pm_start, pm_end;

#define ALI_F(x) (((uintptr_t)x & 0x7)?( (((uintptr_t)x>>3)<<3)+ 0x8 ) : (uintptr_t)x)
//#define ALI_F(x) x
#define SIZE(x)  (sizeof(x))

struct node_t{
	struct node_t * next;
	struct node_t * prev;
	int size;
	int used;
};


struct node_t * myhead; 
pthread_mutex_t mylock;


static void pmm_init() {
  pm_start = (uintptr_t)_heap.start;
  pm_end   = (uintptr_t)_heap.end;
	myhead = (struct node_t *)(ALI_F(pm_start));
	myhead->next= myhead->prev = NULL;
	myhead->used = 0;
	myhead->size = pm_end - ALI_F(pm_start) - ALI_F(SIZE(struct node_t));
	pthread_mutex_init(&mylock, NULL);
}




static void *kalloc(size_t size) {
	pthread_mutex_lock(&mylock);
	void * ret=NULL;
	struct node_t * tmp = myhead;
	while(tmp!=NULL && tmp->size < size) {
		tmp=tmp->next;			
	}
	if(tmp!=NULL) {
		ret = (void *)(tmp) + ALI_F( SIZE(struct node_t) );
		tmp->used = 1;
		if(  ((tmp->next==NULL)?pm_end:(uintptr_t)(tmp->next)) - (uintptr_t)ret - size > 2* ALI_F(SIZE(struct node_t))   ) {
			struct node_t * tmp2 = ALI_F(ret+size);
			tmp2->next = tmp->next;
			if(tmp2->next != NULL)
				tmp2->next->prev = tmp2;
			tmp2->used = 0;
			tmp->next = tmp2;
			tmp2->size = ((tmp2->next==NULL)?pm_end:(void*)(tmp2->next))  - ALI_F( ( (void *)(tmp2) ));
		}
		
	} 

	pthread_mutex_unlock(&mylock);
	return ret;
}

static void kfree(void *ptr) {
	pthread_mutex_lock(&mylock);
	struct node_t * midd = (struct node_t *)(ptr - ALI_F(SIZE(struct node_t)) );
	midd->used = 0;
	struct node_t * tmp = midd;
	while( tmp->next!=NULL && tmp->next->used==0  ) {
		tmp->next = tmp->next->next;
		if(tmp->next->next!=NULL) 
			tmp->next->next->prev = tmp;
		tmp->size =  ((tmp->next->next==NULL)?pm_end:(void*)(tmp->next->next)) - ALI_F( ( (void *)(tmp) ) );
		
	}
	tmp = midd;
	while( tmp->prev!=NULL && tmp->prev->used==0  ) {
		struct node_t * tmp2 = tmp->prev;
		tmp2->next = tmp->next;
		if(tmp->next!=NULL)
			tmp->next->prev = tmp2;	
		tmp2->size =  ((tmp->next==NULL)?pm_end:(void*)(tmp->next)) - ALI_F( ( (void *)(tmp2) ) );
		tmp = tmp2;
	}
	pthread_mutex_unlock(&mylock);
	return;
}

MODULE_DEF(pmm) {
  .init = pmm_init,
  .alloc = kalloc,
  .free = kfree,
};
