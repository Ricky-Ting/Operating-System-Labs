#ifndef __TRACE_H__
#define __TRACE_H__

#define TRACEME

#ifdef TRACEME
	#define TRACE_ENTRY printf("[trace] %s:entry\n", __func__)
	#define TRACE_EXIT printf("[trace] %s:exit\n", __func__)
#else
	#define TRACE_ENTRY ((void)0)
	#define TRACE_EXIT ((void)0)
#endif

/*

#ifdef NDEBUG
	#define assert(ignore) ((void)0)
#else
	#define assert(cond) \
		do { \
			if(!(cond)) { \
				printf("Assertion fail at %s:%d\n", __FILE__, __LINE__ ); \
				_halt(1); \
			} \
		} while(0)
#endif
*/
#endif
