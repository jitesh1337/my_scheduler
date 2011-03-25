/* Single Author info:
 * 	(All of us contributed equal share)
 *	jhshah  Jitesh H  Shah
 *	sskanitk Salil S Kanitkar
 *	msinha	Mukul Sinha
 * Group info:
 *      jhshah     Jitesh H Shah
 *      sskanitk  Salil S Kanitkar
 *	msinha	Mukul Sinha
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/syscall.h>

#include <mythread.h>
#include <mymutex.h>
#include <limits.h>

/* Number of threads to start */
#define NTHREADS	10
#define SETCONCURRENCY	2

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
	mythread_t self = mythread_self();
	int i;

	while(1) {
		mythread_mutex_lock(&mymutex);
		if (gcount < MYLIMIT) {
			/* Delay to encourage preemptions */
			for(i = 0; i < INT_MAX/1000; i++);
			gcount += 50;
			mythread_mutex_unlock(&mymutex);
		} else {
			mythread_mutex_unlock(&mymutex);
			break;
		}
	}

	mythread_mutex_lock(&mymutex);
	printf("My preemption counter [%ld]: %d\n", (long int)self->tid, self->preemptions);
	mythread_mutex_unlock(&mymutex);

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
	
	mythread_mutex_init(&mymutex, NULL);
	mythread_setconcurrency(SETCONCURRENCY);

	printf("NOTE: some prints might be mangled due to parallelism.\nWait for some time for the final output. It will appear :-)\n\n\n");

	for (i = 0; i < NTHREADS; i++) {
		count[i] = i;
		mythread_create(&threads[i], NULL, thread_func, &count[i]);
	}

	mythread_t me = mythread_self();

	mythread_mutex_lock(&mymutex);
	printf("In main_func, I am: %ld\n", (long int)me->tid);
	mythread_mutex_unlock(&mymutex);

	for (i = 0; i < NTHREADS; i++) {
		mythread_mutex_lock(&mymutex);
		printf("Main: Will now wait for thread %ld. Yielding..\n", (long int)threads[i]->tid);
		mythread_mutex_unlock(&mymutex);

		mythread_join(threads[i], (void **)&status);
	}
	printf("Main: All threads completed execution.\nFinal count value:%d. \nWill now exit.\n", gcount);
	mythread_exit(NULL);

	return 0;
}
