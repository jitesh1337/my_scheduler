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
#include <errno.h>
#include <string.h>

#include <signal.h>

#include "mythread.h"
#include <myqueue.h>

#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>

#define SLEEPING	0x2
#define IN_SIGALRM	0x4
#define	IN_SIGUSR1	0x8

static int mythread_scheduler();

#if 0
void dump_queues()
{
	mythread_queue_t qptr, head;
	mythread_t tcb;

	printf("Runq: ");
	head = *mythread_runq();
	qptr = head;
	if (qptr != NULL) {
		do {
			tcb = (mythread_t)qptr->item;
			printf("(%ld,%d) ", (long int)tcb->tid, tcb->preemptions);
			qptr = qptr->next;
		} while(qptr != head);
	}

	printf("  Readyq: ");
	head = *mythread_readyq();
	qptr = head;
	if (qptr != NULL) {
		do {
			tcb = (mythread_t)qptr->item;
			printf("(%ld,%d) ", (long int)tcb->tid, tcb->preemptions);
			qptr = qptr->next;
		} while(qptr != head);
	}
}
#endif

static void signal_handler(int sig)
{
	mythread_t self = mythread_self();
	mythread_queue_t ptr;
	ptr = *mythread_runq();

	/* Unlikely */
	if (ptr != NULL) {

		/* Save the type of signal in our state for later use */
		if (sig == SIGALRM)
			self->state |= IN_SIGALRM;
		else
			self->state |= IN_SIGUSR1;
		self->reschedule = 1;

		if(mythread_tryenter_kernel() == TRUE) {
			//dump_queues();
			
			/* Fantastic race! Before trying to enter the kernel but
			 * AFTER setting reschedule to 1, we might get another
			 * signal and do the rescheduling. It will set, reschedule
			 * to 0. In that case, avoid re-rescheduling.
			 */
			if (self->reschedule == 0)
				/* Unlikely */
				mythread_leave_kernel_nonpreemptive();
			else 
				mythread_leave_kernel();
		}
	}
}

/* Whew, One complex piece of code this */
void mythread_leave_kernel()
{
	mythread_t self = mythread_self();

retry:
	if (self->reschedule == 1) {
		self->reschedule = 0;
		/* SLEEPING state is set when mythread_leave_kernel() is called from inside
		 * mythread_block(). Thus, do non-preemptive leave in that case.
		 *
		 * If mutices are being used, it is possible that the thread calls
		 * leave_kernel()  when NOT in the runq. So check that too.
		 */
		if (self->state & SLEEPING || mythread_inq(mythread_runq(), self) == FALSE) {
			mythread_leave_kernel_nonpreemptive();
		} else if (mythread_scheduler() != 0) {
			/* Scheduler said this thread doesn't need to be rescheduled */
			mythread_leave_kernel_nonpreemptive();
		} else {
			/* Woohoo! Finally, the actual rescheduling. Increment premption counter
			 * too
			 */
			self->preemptions = self->preemptions + 1;
			mythread_block(mythread_readyq(), SLEEPING);
			/* Manually remove sleeping state */
			self->state &= (~SLEEPING);
		}
	} else {
		/* leave_kernel() was called outside of the scheduler. Just do a
		 * non-preemptive leave.
		 */
        	mythread_leave_kernel_nonpreemptive();
	}

	/* It is possible that the thread got a signal when it was inside a signal
	 * handler. In that case, repeat the above procedure.
	 */
	if (self->reschedule == 1) {
		 if (mythread_tryenter_kernel() == TRUE)
			goto retry;
	}
}

/* Actual decision of scheduling is taken here */
static int mythread_scheduler()
{
	mythread_queue_t ptr, head;
	mythread_t self = mythread_self();
	/* We are in the kernel already, don't worry about going in the kernel */

	/* If we got here via a SIGALRM, then, send SIGUSR1 to everyone */
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

	/* Reset all signal states */
	self->state &= ~IN_SIGALRM;
	self->state &= ~IN_SIGUSR1;

	/* Scheduling decision */
	if (*mythread_readyq() != NULL)
		return 0;
	else 
		return -1;

}

/* Global structures to save old data to restore them */
struct sigaction sig_act;
struct sigaction old_sig_act;

sigset_t newmask;
sigset_t oldmask;

static int timer_initialised = 0;

/* init_sched */
void mythread_init_sched()
{
	struct itimerval timer;
	struct timeval timerval;

	memset(&sig_act, '\0', sizeof(sig_act));
	sig_act.sa_handler = signal_handler;

	/* Install SIGALRM and save old sigaction */
	if (sigaction(SIGALRM, &sig_act, &old_sig_act) == -1) {
		printf("Error in registering the Signal Handler for SIGALRM!\n");
		printf("Exiting....");
		exit(-1);
	}

	/* Install SIGUSR1 and save old sigaction */
	if (sigaction(SIGUSR1, &sig_act, &old_sig_act) == -1) {
		printf("Error in registering the Signal Handler for SIGUSR1!\n");
		printf("Exiting....");
		exit(-1);
	}

	/* Unmask signals */
	sigemptyset(&newmask);
	sigaddset(&newmask, SIGALRM);
	sigaddset(&newmask, SIGUSR1);
	sigprocmask(SIG_UNBLOCK, &newmask, &oldmask);

	/* Start the timer */
	if (timer_initialised == 0) {
		timer_initialised = 1;
		timerval.tv_sec = 0;
		timerval.tv_usec = 10000;
		timer.it_interval = timerval;
		timer.it_value = timerval;
		setitimer(ITIMER_REAL, &timer, NULL);
	}
}

void mythread_exit_sched()
{
	/* Restore old SIGUSR1 handler */
	if (sigaction(SIGUSR1, &old_sig_act, &sig_act) == -1) {
		printf("Error in removing the signal handler for SIGUSR1!\n");
		printf("Exiting....\n");
		exit(-1);
	}

	/* Restore old SIGALRM handler */
	if (sigaction(SIGALRM, &old_sig_act, &sig_act) == -1) {
		printf("Error in removing the Signal Handler for SIGALRM!\n");
		printf("Exiting....\n");
		exit(-1);
	}

	/* Restore old signal mask */
	sigprocmask(SIG_SETMASK, &oldmask, &newmask);
}

