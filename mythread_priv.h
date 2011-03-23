#include <stdio.h>
#include <mythread.h>
#include <futex.h>

/*#define DEBUG_PRINTF(...) mythread_enter_kernel(); \
			printf(__VA_ARGS__);  \
			fflush(stdout); \
			mythread_leave_kernel();
*/

extern struct futex printf_fut;

/* #define DEBUG_PRINTF(...) futex_down(&printf_fut);  \
			printf(__VA_ARGS__);  \
			fflush(stdout);  \
			futex_up(&printf_fut); */

#define DEBUG_PRINTF(...) printf(__VA_ARGS__);
