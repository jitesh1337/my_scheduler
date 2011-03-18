/*
 * mycond.h -- interface of condition variables
 */

#ifndef MYCOND_H
#define MYCOND_H

#include "mymutex.h"

typedef struct mythread_condattr {
  int attr;                           /* not yet used */
} mythread_condattr_t;

typedef struct mythread_cond {
  mythread_queue_t q;
} mythread_cond_t;

int mythread_cond_init(mythread_cond_t *cond,
		       const mythread_condattr_t *attr);

int mythread_cond_wait(mythread_cond_t *cond, mythread_mutex_t *lock);

int mythread_cond_signal(mythread_cond_t *cond);

int mythread_cond_broadcast(mythread_cond_t *cond);

#endif
