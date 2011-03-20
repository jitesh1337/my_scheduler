#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <signal.h>

#include "mythread.h"

#include <sys/syscall.h>
#include <sys/types.h>

static void signal_handler(int sig)
{
	printf("I am: %ld\n", (long int)syscall(SYS_gettid));

	if ( sig == SIGALRM ) {
		printf("Received Alarm Signal! \n");
		fflush(stdout);
	}
	else if ( sig == SIGUSR1) {
		printf("Received User Signal\n");
		fflush(stdout);
	}

	mythread_unblock(mythread_readyq(), 0);
}

struct sigaction sig_act;
struct sigaction old_sig_act;

void mythread_init_sched()
{
	memset(&sig_act, '\0', sizeof(sig_act));

	sig_act.sa_handler = signal_handler;
	sigemptyset(&sig_act.sa_mask);

	if ( sigaction(SIGALRM, &sig_act, &old_sig_act) == -1 ) {
		printf("Error in registering the Signal Handler for SIGALRM!\n");
		printf("Exiting....");
		exit(-1);
	}

	if ( sigaction(SIGUSR1, &sig_act, &old_sig_act) == -1 ) {
		printf("Error in registering the Signal Handler for SIGUSR1!\n");
		printf("Exiting....");
		exit(-1);
	}
}

void mythread_exit_sched()
{

	if ( sigaction(SIGALRM, &old_sig_act, &sig_act) == -1 ) {
		printf("Error in removing the Signal Handler for SIGALRM!\n");
		printf("Exiting....\n");
		exit(-1);
	}

	if ( sigaction(SIGUSR1, &old_sig_act, &sig_act) == -1 ) {
		printf("Error in removing the signal handler for SIGUSR1!\n");
		printf("Exiting....\n");
		exit(-1);
	}

}

void mythread_leave_kernel()
{
        ;
}
