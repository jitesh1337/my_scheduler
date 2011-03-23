#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <signal.h>

#include "mythread.h"
#include <myqueue.h>
#include <mythread_priv.h>

#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>

int mythread_scheduler();

void dump_queues()
{
	mythread_queue_t qptr, head;
	mythread_t tcb;

	DEBUG_PRINTF("Runq: ");
	head = *mythread_runq();
	qptr = head;
	if (qptr != NULL) {
		do {
			tcb = (mythread_t)qptr->item;
			DEBUG_PRINTF("%ld ", (long int)tcb->tid);
			qptr = qptr->next;
		} while(qptr != head);
	}

	DEBUG_PRINTF("  Readyq: ");
	head = *mythread_readyq();
	qptr = head;
	if (qptr != NULL) {
		do {
			tcb = (mythread_t)qptr->item;
			DEBUG_PRINTF("%ld ", (long int)tcb->tid);
			qptr = qptr->next;
		} while(qptr != head);
	}
}

static void signal_handler(int sig)
{
	mythread_t self = mythread_self();
	mythread_queue_t ptr, head;

	head = *mythread_runq();
	ptr = head;

	if (ptr != NULL) {

		self->reschedule = 1;

		printf("Now competing for kernel %ld\n", (long int)syscall(SYS_gettid));
		//while(mythread_tryenter_kernel() == FALSE);
		if(mythread_tryenter_kernel() == TRUE)
		{
			printf("Got lock %ld\n", (long int)syscall(SYS_gettid));
			dump_queues();
			if (sig == SIGALRM) {
	
				DEBUG_PRINTF("Received Alarm Signal! %ld\n", (long int)syscall(SYS_gettid));

				ptr = head;
				do {
					if (self != ptr->item) {
						syscall(SYS_tkill, ((mythread_t)ptr->item)->tid, SIGUSR1);
					}
					ptr = ptr->next;
				} while(ptr != head);

				mythread_leave_kernel();

			}
			else if (sig == SIGUSR1) {

				DEBUG_PRINTF("Received User Signal! %ld\n", (long int)syscall(SYS_gettid));

				self->preemptions = self->preemptions + 1;
				mythread_leave_kernel();

			}
		} else {
			DEBUG_PRINTF("enter_kernel() failed!! %ld\n", (long int)syscall(SYS_gettid));
		} 
	}
	/*if (sig == SIGALRM) {
				
		mythread_enter_kernel();
		head = *mythread_runq();
		ptr = head;
		if (ptr != NULL) {
			do {
				if (self != ptr->item) {
					syscall(SYS_tkill, ((mythread_t)ptr->item)->tid, SIGUSR1);
				}
				ptr = ptr->next;
			} while(ptr != head);
		}
		mythread_leave_kernel();
		DEBUG_PRINTF("Received Alarm Signal! %ld\n", (long int)syscall(SYS_gettid));
	} else if (sig == SIGUSR1) {
		DEBUG_PRINTF("Received User Signal! %ld\n", (long int)syscall(SYS_gettid));
	}*/

	/*mythread_enter_kernel();
	mythread_unblock(mythread_readyq(), 0);

	mythread_enter_kernel();
	mythread_block(mythread_readyq(), 0);*/
}

void mythread_leave_kernel()
{
	mythread_t self = mythread_self();

retry:
	if (self->reschedule == 1) {
		self->reschedule = 0;
		if (mythread_scheduler() != 0)
			mythread_leave_kernel_nonpreemptive();
		else {
			mythread_block(mythread_readyq(), 0);
		}
	}
	else {
		printf("Leaving without reschedule %ld %ld\n", (long int)self->tid, (long int)syscall(SYS_gettid));
        	mythread_leave_kernel_nonpreemptive();
	}

	if (self->reschedule == 1) {
		 if (mythread_tryenter_kernel() == TRUE)
			goto retry;
	}
}

int mythread_scheduler()
{
	/* We are in the kernel already, don't worry about going in the kernel */
	if (*mythread_readyq() != NULL)
		return 0;
	else 
		return -1;

}

struct sigaction sig_act;
struct sigaction old_sig_act;

sigset_t newmask;
sigset_t oldmask;

void mythread_init_sched()
{
	struct itimerval timer;
	struct timeval timerval;

	memset(&sig_act, '\0', sizeof(sig_act));
	sig_act.sa_handler = signal_handler;

	sigemptyset(&newmask);
	sigaddset(&newmask, SIGALRM);
	sigaddset(&newmask, SIGUSR1);
	sigprocmask(SIG_UNBLOCK, &newmask, &oldmask);

	if (sigaction(SIGALRM, &sig_act, &old_sig_act) == -1) {
		DEBUG_PRINTF("Error in registering the Signal Handler for SIGALRM!\n");
		DEBUG_PRINTF("Exiting....");
		exit(-1);
	}

	if (sigaction(SIGUSR1, &sig_act, &old_sig_act) == -1) {
		DEBUG_PRINTF("Error in registering the Signal Handler for SIGUSR1!\n");
		DEBUG_PRINTF("Exiting....");
		exit(-1);
	}

	timerval.tv_sec = 3;
	timerval.tv_usec = 9999;
	timer.it_interval = timerval;
	timer.it_value = timerval;
	setitimer(ITIMER_REAL, &timer, NULL);
}

void mythread_exit_sched()
{

	if (sigaction(SIGUSR1, &old_sig_act, &sig_act) == -1) {
		DEBUG_PRINTF("Error in removing the signal handler for SIGUSR1!\n");
		DEBUG_PRINTF("Exiting....\n");
		exit(-1);
	}

	if (sigaction(SIGALRM, &old_sig_act, &sig_act) == -1) {
		DEBUG_PRINTF("Error in removing the Signal Handler for SIGALRM!\n");
		DEBUG_PRINTF("Exiting....\n");
		exit(-1);
	}

	sigprocmask(SIG_SETMASK, &oldmask, &newmask);

}

