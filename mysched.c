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

#define SLEEPING	0x2
#define IN_SIGALRM	0x4
#define	IN_SIGUSR1	0x8

static int mythread_scheduler();

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
			DEBUG_PRINTF("(%ld,%d) ", (long int)tcb->tid, tcb->preemptions);
			qptr = qptr->next;
		} while(qptr != head);
	}

	DEBUG_PRINTF("  Readyq: ");
	head = *mythread_readyq();
	qptr = head;
	if (qptr != NULL) {
		do {
			tcb = (mythread_t)qptr->item;
			DEBUG_PRINTF("(%ld,%d) ", (long int)tcb->tid, tcb->preemptions);
			qptr = qptr->next;
		} while(qptr != head);
	}
}

static void signal_handler(int sig)
{
	mythread_t self = mythread_self();
	mythread_queue_t ptr;
	ptr = *mythread_runq();

	if (ptr != NULL) {
		if (sig == SIGALRM)
			self->state |= IN_SIGALRM;
		else
			self->state |= IN_SIGUSR1;
		self->reschedule = 1;

		if (sig == SIGALRM) {
			DEBUG_PRINTF("Now competing for kernel. SIGALRM %ld\n", (long int)self->tid);
		} else	
			printf("Now competing for kernel. SIGUSR1 %ld\n", (long int)self->tid);
		if(mythread_tryenter_kernel() == TRUE)
		{
			if (self->reschedule == 0)
				mythread_leave_kernel_nonpreemptive();

			//printf("Got lock %ld\n", (long int)self->tid);
			dump_queues();
			if (sig == SIGALRM) {
				DEBUG_PRINTF("Received Alarm Signal! %ld\n", (long int)self->tid);

				mythread_leave_kernel();
			} else if (sig == SIGUSR1) {
				DEBUG_PRINTF("Received User Signal! %ld\n", (long int)self->tid);
				mythread_leave_kernel();
			}
		} else {
			DEBUG_PRINTF("enter_kernel() failed!! %ld\n", (long int)self->tid);
		} 
	}
}

void mythread_leave_kernel()
{
	mythread_t self = mythread_self();

retry:
	if (self->reschedule == 1) {
		self->reschedule = 0;
		printf("State: %d %ld\n", self->state & SLEEPING, (long int)self->tid);
		if (self->state & SLEEPING || mythread_inq(mythread_runq(), self) == FALSE) {
			//printf("non-Preemptive leave %ld\n", (long int)self->tid);
			mythread_leave_kernel_nonpreemptive();
		} else if (mythread_scheduler() != 0) {
			//printf("non-Preemptive leave %ld\n", (long int)self->tid);
			mythread_leave_kernel_nonpreemptive();
		} else {
			//printf("BLock %ld\n", (long int)self->tid);
			self->preemptions = self->preemptions + 1;
			mythread_block(mythread_readyq(), SLEEPING);
			self->state &= (~SLEEPING);
		}
	}
	else {
		//printf("Leaving without reschedule %ld %ld\n", (long int)self->tid, (long int)self->tid);
        	mythread_leave_kernel_nonpreemptive();
	}

	if (self->reschedule == 1) {
		 if (mythread_tryenter_kernel() == TRUE)
			goto retry;
	}
}

static int mythread_scheduler()
{
	mythread_queue_t ptr, head;
	mythread_t self = mythread_self();
	/* We are in the kernel already, don't worry about going in the kernel */

	if (self->state & IN_SIGALRM) {
		head = *mythread_runq();
		ptr = head;
		do {
			if (self != ptr->item) {
				syscall(SYS_tkill, ((mythread_t)ptr->item)->tid, SIGUSR1);
			}
			ptr = ptr->next;
		} while(ptr != head); 
	}
	self->state &= ~IN_SIGALRM;
	self->state &= ~IN_SIGUSR1;

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

	timerval.tv_sec = 0;
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

