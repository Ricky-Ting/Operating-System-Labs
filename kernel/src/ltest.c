#include <common.h>

//static lock_t test_lock;

void test_full(){
  //init_lock(&test_lock, 'b');
  //printf("test_full: lock %p\n", &test_lock);
  int *p = NULL;
  int *p_old = NULL;
  int term = 0;
  while((p = pmm->alloc(1000*sizeof(int)))){
    //spin_lock(&test_lock);
    printf("\33[1;35mtest_full: I'm at %#x, %d\n\33[0m", (uintptr_t)p,_cpu());
    //Assert(test_lock.slock == 1, "test_full: test_lock.slock值为0");
    //spin_unlock(&test_lock);
    for(int i=0;i < 1000;i++){
      //printf("test_full: I'm at %p, %d\n", p, i);
      p[i] = i;
    }
    if(p_old != NULL){
      for(int i=0;i < 1000;i++){
        Assert(p_old[i] == i, "test_full: 旧值被改变");
      }
    }
    if(p_old != NULL)
      pmm->free(p_old);
    p_old = p;
    term++;
    /*
    if(term >= 5)
      break;
      */
  }
}

void test_big_small(){
  int *p = NULL;
  int *p_old = NULL;
  size_t size[] = {
    KALLOC_BLOCK/sizeof(int)/10*3,
    KALLOC_BLOCK/sizeof(int)/10*1
  };
  int num = sizeof(size)/sizeof(size[0]);
  for(int i=0;;i++){
    if((p = pmm->alloc(size[i%num]*sizeof(int))) == NULL)
      break;
    //printf("test_big_small: start: %d\n", (uintptr_t)p);
    printf("\33[1;35mtest_big_small: I'm at %d, %d, %d\n\33[0m", (uintptr_t)p, i, _cpu());
    for(int j=0;j < size[i%num];j++){
      p[j] = j;
    }
    
    //printf("test_big_small: %d %d\n", (uintptr_t)p, (uintptr_t)p_old);

    if(p_old != NULL){
      for(int j=0;j < size[(i-1)%num];j++){
        Assert(p_old[j] == j, "test_big_smal: 值被改变");
      }
    }
    if(p_old != NULL)
      pmm->free(p_old);
    p_old = p;
    /*
    if(i >= 10)
      break;
    */
  }
}


void (*test_point)() = test_big_small;
