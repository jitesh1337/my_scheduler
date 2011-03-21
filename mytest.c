#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <mythread.h>
#include <mythread_priv.h>

/* Number of threads to start */
#define NTHREADS	2


/* This function will first increment count by 50, yield. When it gets the 
 * control back, it will increment count again and then exit
 */
void *thread_func(void *arg)
{
	int *count = (int *)arg;

	mythread_t me = mythread_self();
	DEBUG_PRINTF("In thread_func, I am: %ld\n", (long int)me->tid);

	*count = *count + 50;
	DEBUG_PRINTF("Thread %ld: Incremented count by 50 and will now yield\n", (long int)me->tid);
	fflush(stdout);
	//mythread_yield();
	*count = *count + 50;
	DEBUG_PRINTF("Thread %ld: Incremented count by 50 and will now exit\n", (long int)me->tid);
	fflush(stdout);

	while(1);

	mythread_exit(NULL);
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
	sigset_t mask;

	mythread_setconcurrency(3);

	for (i = 0; i < NTHREADS; i++) {
		count[i] = i;
		mythread_create(&threads[i], NULL, thread_func, &count[i]);
	}
	
	sigemptyset(&mask);
	sigaddset(&mask, SIGALRM);
	sigprocmask(SIG_UNBLOCK, &mask, NULL);

	mythread_t me = mythread_self();
	DEBUG_PRINTF("In main_func, I am: %ld\n", (long int)me->tid);

	for (i = 0; i < NTHREADS; i++) {
		DEBUG_PRINTF("Main: Will now wait for thread %ld. Yielding..\n", (long int)threads[i]->tid);
		mythread_join(threads[i], (void **)&status);
		DEBUG_PRINTF("Main: Thread %ld exited and increment count to %d\n", (long int)threads[i]->tid, count[i]);
	}
	DEBUG_PRINTF("Main: All threads completed execution. Will now exit..\n");
	mythread_exit(NULL);

	return 0;
}
