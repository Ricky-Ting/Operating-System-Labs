

#ifdef TRACEME
	#define TRACE_ENTRY printf("[trace] %s:entry\n", __func__)
	#define TRACE_EXIT printf("[trace] %s:exit\n", __func__)
#else
	#define TRACE_ENTRY ((void)0)
	#define TRACE_EXIT ((void)0)
#endif
