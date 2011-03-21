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
	dump_queues();
	mythread_t self = mythread_self();
	mythread_queue_t ptr, head;

	if (sig == SIGALRM) {
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
	}

	/* mythread_enter_kernel();
	mythread_unblock(mythread_readyq(), 0);

	mythread_enter_kernel();
	mythread_block(mythread_readyq(), 0); */
}

struct sigaction sig_act;
struct sigaction old_sig_act;

void mythread_init_sched()
{
	sigset_t mask;
	struct itimerval timer;
	struct timeval timerval;

	memset(&sig_act, '\0', sizeof(sig_act));
	sig_act.sa_handler = signal_handler;
	sigemptyset(&sig_act.sa_mask);

	sigemptyset(&mask);
	sigprocmask(SIG_SETMASK, &mask, NULL);

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
	timerval.tv_usec = 0;
	timer.it_interval = timerval;
	timer.it_value = timerval;
	setitimer(ITIMER_REAL, &timer, NULL);
}

void mythread_exit_sched()
{
	if (sigaction(SIGALRM, &old_sig_act, &sig_act) == -1) {
		DEBUG_PRINTF("Error in removing the Signal Handler for SIGALRM!\n");
		DEBUG_PRINTF("Exiting....\n");
		exit(-1);
	}

	if (sigaction(SIGUSR1, &old_sig_act, &sig_act) == -1) {
		DEBUG_PRINTF("Error in removing the signal handler for SIGUSR1!\n");
		DEBUG_PRINTF("Exiting....\n");
		exit(-1);
	}
}

void mythread_leave_kernel()
{
        mythread_leave_kernel_nonpreemptive();
}
