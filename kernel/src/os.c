#include <common.h>
#include <klib.h>
#include <devices.h>
void echo_task(void *name) {
	device_t *tty = dev_lookup(name);
	while(1) {
		char line[128], text[128];
		sprintf(text, "(%s) $", name); tty_write(tty,0,text,strlen(text));
		int nread = tty->ops->read(tty, 0, line, sizeof(line));
		line[nread - 1] = '\0';
		sprintf(text, "Echo: %s.\n", line); tty_write(tty,0,text,strlen(text));
	}
}
struct handler_node* handler_head;


void test() {
	
	int ret = vfs->access("/mnt",0);
	printf("Access is %d\n",ret);
	int fd = vfs->open("/mnt/a.txt",0);
	printf("The fd is %d\n",fd);
	
	ret = vfs->access("/mnt/a.txt",0);
	printf("a.xtxt is %d\n",ret);
	
	ret = vfs->write(fd,"helloworld\n",11);
	printf("Written %d bytes\n",ret);
	ret = vfs->lseek(fd,0,SEEK_SET);
	printf("The offset is set to %d\n",ret);
	char buf[200];
	ret = vfs->read(fd,buf,11);
	printf("Read %d bytes \n",ret);
	printf("Read %s\n",buf);
	

	ret = vfs->mkdir("/mnt/ricky");
	printf("mkdir return %d\n",ret);
	int fd2 = vfs->open("/mnt/ricky/cir.txt",0);
	printf("create cir, %d\n",fd2);

	ret = vfs->access("/mnt/ricky",0);
	printf("access /ricky, %d\n",ret);
	
	ret = vfs->access("/mnt/ricky/cir.txt",0);
	printf("access /mnt/ricky/cir.txt %d\n",ret);

	char buf2[200],buf3[20];	
	vfs->write(fd2,"hello,",6);
	vfs->lseek(fd2,0,SEEK_SET);
	vfs->read(fd2,buf2,6);
	printf("%s\n",buf2);
	vfs->lseek(fd2,0,SEEK_END);
	vfs->write(fd2,"world",5);
	vfs->lseek(fd2,0,SEEK_SET);
	vfs->read(fd2,buf3,11);
	printf("%s\n",buf3); 


	ret = vfs->link("/mnt/ricky/cir.txt", "/mnt/ricky/rir.txt");
	printf("link return %d\n",ret);


	char buf4[30];
	int fd3 = vfs->open("/mnt/ricky/rir.txt",0);
	vfs->read(fd3,buf4,20);
	printf("rir:%sh\n");

	ret = vfs->unlink("/mnt/ricky/cir.txt");
	vfs->unlink("/mnt/ricky/rir.txt");
	printf("unlink return %d\n",ret);
	ret = vfs->rmdir("/mnt/ricky");
	printf("rm return %d\n",ret);
}


static void os_init() {
	handler_head = NULL;
	pmm->init();
	kmt->init();
	dev->init();
	vfs->init();
vfs->mount("/mnt",create_blkfs("ramdisk0",dev_lookup("ramdisk0")));

		//kmt->create(&tmptask, "test", test, NULL);

	kmt->create(pmm->alloc(sizeof(task_t)), "test", test, NULL);
	//kmt->create(pmm->alloc(sizeof(task_t)), "print1", echo_task, "tty1");
	//kmt->create(pmm->alloc(sizeof(task_t)), "print2", echo_task, "tty2");
	//kmt->create(pmm->alloc(sizeof(task_t)), "print3", echo_task, "tty3");
	//kmt->create(pmm->alloc(sizeof(task_t)), "print4", echo_task, "tty4");

};
static void hello() {
  for (const char *ptr = "Hello from CPU #"; *ptr; ptr++) {
    _putc(*ptr);
  }
  _putc("12345678"[_cpu()]); _putc('\n');
}

static void os_run() {
  hello();
  _intr_write(1);
	//mytest();
  while (1) {
		//printf("CPU:%d\n",_cpu());
    _yield();
  }
	
}

static _Context *os_trap(_Event ev, _Context *context) {
	/*
		Can the ret be NULL?
	*/
	TRACE_ENTRY;
	//printf("Event: %d\n", ev.event);
	_Context *ret = NULL;
	struct handler_node* iter = handler_head; 		
	assert(iter!=NULL);
	while(iter!=NULL) {
		if(iter->event==_EVENT_NULL || iter->event == ev.event) {
			_Context *next = iter->handler(ev, context);
			if(next) 
				ret = next;
		}
		iter = iter->next;
	}		
	assert(ret!=NULL);
	TRACE_EXIT;	
  return ret;
}


static void os_on_irq(int seq, int event, handler_t handler) {
	/*
		May be buggy. 
		pmm has to be inited before this function is called.
	*/			
	TRACE_ENTRY;
	
	if(handler_head==NULL) {
		handler_head = pmm->alloc(sizeof(struct handler_node));
		handler_head->seq = seq;
		handler_head->event = event;
		handler_head->handler = handler;
		handler_head->next = NULL;				
	} else {
		struct handler_node* insert_node = pmm->alloc(sizeof(struct handler_node));
		insert_node->seq = seq;
		insert_node->event = event;
		insert_node->handler = handler;
		if(seq < handler_head->seq) {
			insert_node->next = handler_head;
			handler_head = insert_node;		
		} else {
			struct handler_node* iter = handler_head;
		 	while(iter->next!=NULL && seq >= iter->next->seq)
				iter = iter->next;
			insert_node->next = iter->next;
			iter->next = insert_node;		
		}
	}
	TRACE_EXIT;
	return;
}

MODULE_DEF(os) {
  .init   = os_init,
  .run    = os_run,
  .trap   = os_trap,
  .on_irq = os_on_irq,
};

