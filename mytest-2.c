#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/syscall.h>

#include <mythread.h>
#include <mymutex.h>
#include <mythread_priv.h>
#include <limits.h>

/* Number of threads to start */
#define NTHREADS	10

#define MYLIMIT	50000

struct futex printf_fut;
mythread_mutex_t mymutex;
int gcount = 0;

/* This function will first increment count by 50, yield. When it gets the 
 * control back, it will increment count again and then exit
 */
void *thread_func(void *arg)
{
	int count = *(int *)arg;
	int i;
	//mythread_t me = mythread_self();

	while(gcount < MYLIMIT) {
		mythread_mutex_lock(&mymutex);
			for(i = 0; i < INT_MAX/1000; i++);
			gcount += 50;
		mythread_mutex_unlock(&mymutex);
	}

	if (count % 2)
		mythread_exit(0);

	return NULL;
}

/* This is a simple demonstration of how to use the mythread library.
 * Start NTRHEADS number of threads, collect count value and exit
 */
int main()
{
	mythread_t threads[NTHREADS];
	int count[NTHREADS];
	int i;
	char *status;
	
	futex_init(&printf_fut, 1);
	mythread_mutex_init(&mymutex, NULL);
	mythread_setconcurrency(2);

	for (i = 0; i < NTHREADS; i++) {
		count[i] = i;
		mythread_create(&threads[i], NULL, thread_func, &count[i]);
	}

	mythread_t me = mythread_self();
	DEBUG_PRINTF("In main_func, I am: %ld\n", (long int)me->tid);

	for (i = 0; i < NTHREADS; i++) {
		DEBUG_PRINTF("Main: Will now wait for thread %ld. Yielding..\n", (long int)threads[i]->tid);
		mythread_join(threads[i], (void **)&status);
		DEBUG_PRINTF("Main: Thread %ld exited and increment count to %d\n", (long int)threads[i]->tid, count[i]);
	}
	DEBUG_PRINTF("Main: All threads completed execution:%d. Will now exit..\n", gcount);
	mythread_exit(NULL);

	return 0;
}
