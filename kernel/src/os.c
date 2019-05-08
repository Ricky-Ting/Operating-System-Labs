#include <common.h>
#include <klib.h>
#include <devices.h>
void mytest(void);

void fa() {
	while(1) {
		_putc('a');
		_yield();
	}
}
void fb() {
	while(1) {
		_putc('b');
		_yield();
	}
}

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


static void os_init() {
	handler_head = NULL;
	pmm->init();
	kmt->init();
	dev->init();
	//vfs->init();
	//kmt->create(pmm->alloc(sizeof(task_t)), "a", fa, NULL);
	//kmt->create(pmm->alloc(sizeof(task_t)), "b", fb, NULL);
	kmt->create(pmm->alloc(sizeof(task_t)), "print1", echo_task, "tty1");
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

