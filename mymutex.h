/*
 * mymutex.h -- interface of mutual exclusion
 */

#ifndef MYMUTEX_H
#define MYMUTEX_H

//#define MYTHREAD_SPIN

typedef unsigned int mythread_lock_t;

typedef struct mythread_mutexattr {
  int attr;                           /* not yet used */
} mythread_mutexattr_t;

typedef struct mythread_mutex {
  mythread_lock_t spin;
  mythread_queue_t q;
  //#ifndef MYTHREAD_SPIN
  mythread_t tail;          /* end of MCS queue */
  struct mythread *locknext; /* head pointer of MCS lock queue */
  //#endif
} mythread_mutex_t;

/*
 * mythread_mutex_init - initialize mutex data structure
 */
int mythread_mutex_init(mythread_mutex_t *lock,
			const mythread_mutexattr_t *attr);

/*
 * mythread_mutex_lock - aquire a lock for mutual exclusion
 */
int mythread_mutex_lock(mythread_mutex_t *lock);

/*
 * mythread_mutex_unlock - release a lock for mutual exclusion
 */
int mythread_mutex_unlock(mythread_mutex_t *lock);

/*
 * mythread_mutex_destroy - relinguish a lock for mutual exclusion
 */
int mythread_mutex_destroy(mythread_mutex_t *lock);

#endif
