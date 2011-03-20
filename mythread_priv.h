#include <stdio.h>
#include <mythread.h>

#define DEBUG_PRINTF(...) mythread_enter_kernel(); \
			printf(__VA_ARGS__);  \
			fflush(stdout); \
			mythread_leave_kernel();

