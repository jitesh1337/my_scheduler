#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mythread.h>

/* Number of threads to start */
#define NTHREADS	1

/* This function will first increment count by 50, yield. When it gets the 
 * control back, it will increment count again and then exit
 */
void *thread_func(void *arg)
{
	int *count = (int *)arg;

	mythread_t me = mythread_self();
	printf("In thread_func, I am: %ld\n", (long int)me->tid);

	*count = *count + 50;
	printf("Thread %ld: Incremented count by 50 and will now yield\n", (long int)me->tid);
	fflush(stdout);
	//mythread_yield();
	*count = *count + 50;
	printf("Thread %ld: Incremented count by 50 and will now exit\n", (long int)me->tid);
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

	for (i = 0; i < NTHREADS; i++) {
		count[i] = i;
		mythread_create(&threads[i], NULL, thread_func, &count[i]);
	}
	
	mythread_t me = mythread_self();
	printf("In main_func, I am: %ld", (long int)me->tid);

	for (i = 0; i < NTHREADS; i++) {
		printf("Main: Will now wait for thread %ld. Yielding..\n", (long int)threads[i]->tid);
		mythread_join(threads[i], (void **)&status);
		printf("Main: Thread %ld exited and increment count to %d\n", (long int)threads[i]->tid, count[i]);
	}
	printf("Main: All threads completed execution. Will now exit..\n");
	mythread_exit(NULL);

	return 0;
}
