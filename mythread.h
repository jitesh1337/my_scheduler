/*
 * mythread.h -- interface of user threads library
 */

#ifndef MYTHREAD_H
#define MYTHREAD_H

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#include <pthread.h>
#include <sys/times.h>
#include "myqueue.h"
#include "futex.h"

typedef struct mythread_attr { /* thread attributes */
  int attr;                    /* not yet used */
} mythread_attr_t;

typedef struct mythread {      /* thread control block */
  pid_t tid;
  struct futex block;
  struct mythread *locknext;   /* next pointer for MCS lock */
  int locked;                  /* MCS status for spin lock/backoff */
  void *stack;
  int state;                   /* state of execution */
  void * (*start_func)(void*); /* thread_func to be called */
  void *arg;                   /* thread_arg arguments of thread_func */
  mythread_queue_t joinq;      /* Q of threads waiting for my return */
  void *returnValue;           /* Pointer to detached function's return value */
  int reschedule;              /* if set to signo then reschedule ASAP */
  struct tms ts;               /* last timestamp recorded at timeslice */
  int preemptions;             /* number of preemptions so far */
} *mythread_t;

/*
 * mythread_readyq - pointer to ready q
 */
mythread_queue_t *mythread_readyq(void);

/*
 * mythread_runq - pointer to run q
 */
mythread_queue_t *mythread_runq(void);

/*
 * mythread_setconcurrency - set the number of LWPs
 * (max. number of parallel threads)
 */
void mythread_setconcurrency(int new_level);

/*
 * mythread_getconcurrency - return the number of LWPs
 * (max. number of parallel threads)
 */
int mythread_getconcurrency(void);

/*
 * mythread_enter_kernel - enter the monolithic threading kernel
 */
void mythread_enter_kernel(void);

/*
 * mythread_tryenter_kernel - enter the monolithic threading kernel IF not busy
 * return TRUE on success, FALSE o/w
 */
int mythread_tryenter_kernel(void);

/*
 * mythread_leave_kernel_nonpreemptive - leave the monolithic threading kernel
 * without allowing preemptions
 */
void mythread_leave_kernel_nonpreemptive(void);

/*
 * mythread_self - thread id of running thread
 */
mythread_t mythread_self(void);

/*
 * mythread_self - thread id of running thread
 */
mythread_t mythread_self(void);

/*
 * mythread_block - remove currently running thread off run q,
 * if q is non-NULL, enq this thread on q, suspend the thread,
 * add the state flags to the thread's state (via bit-wise OR | state)
 * and activate ready threads if an LWP becomes available
 * pre: must be in monolithic kernel
 * post: return outside the monolithic kernel
 */
void mythread_block(mythread_queue_t *q, int state);

/*
 * mythread_unblock - resumes the thread at the head of q,
 * remove the state flags to the thread's state (via bit-wise AND & ~state)
 * and activate the thread if an LWP becomes available (o/w mark as ready)
 * pre: must be in monolithic kernel
 * post: return outside the monolithic kernel
 */
void mythread_unblock(mythread_queue_t *q, int state);

/*
 * mythread_unblock_thread - resumes the target thread from q,
 * remove the state flags to the thread's state (via bit-wise AND & ~state)
 * and activate the thread if an LWP becomes available (o/w mark as ready)
 * pre: must be in monolithic kernel
 * post: return outside the monolithic kernel
 */
void mythread_unblock_thread(mythread_queue_t *q, mythread_t target, int state);

/*
 * mythread_create - prepares context of new_thread_ID as start_func(arg),
 * attr is ignored right now.
 * Threads are activated (run) according to the number of available LWPs
 * or are marked as ready.
 */
int mythread_create(mythread_t *new_thread_ID,
		    const mythread_attr_t *attr,
		    void * (*start_func)(void *),
		    void *arg);

/*
 * mythread_yield - switch from running thread to the next ready one
 */
int mythread_yield(void);

/*
 * mythread_join - suspend calling thread if target_thread has not finished,
 * enqueue on the join Q of the target thread, then dispatch ready thread;
 * once target_thread finishes, it activates the calling thread / marks it
 * as ready.
 */
int mythread_join(mythread_t target_thread, void **status);

/*
 * mythread_exit - exit thread, awakes joiners on return
 * from thread_func and dequeue itself from run Q before dispatching run->next
 */
void mythread_exit(void *retval);

/*
 * mythread_init_sched - called once at the start of each thread
 * enable virtual timer signals, install signal handler for it and
 * set time slice to 20ms for virtual timer
 */
void mythread_init_sched(void);

/*
 * mythread_exit_sched - called on thread terminiation (exit) by each thread
 * disable all the features activated in mythread_init_sched()
 */
void mythread_exit_sched(void);

#endif /* MYTHREAD_H */
