#include <common.h>
#include <klib.h>

void mytest(void);


struct handler_node* handler_head = NULL;

static void os_init() {
  pmm->init();
}

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
    _yield();
  }
	
}

static _Context *os_trap(_Event ev, _Context *context) {
	/*
		Can the ret be NULL?
	*/
	TRACE_ENTRY;
	_Context *ret = NULL:
	struct handler_node* iter = handler_head; 		
	assert(iter!=NULL);
	while(iter!=NULL) {
		if(iter->event==_EVENT_NULL || handler->event == ev.event) {
			_Context *next = iter->handler(ev, context);
			if(next) 
				ret = next;
		}
		iter = iter->next;
	}		

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

