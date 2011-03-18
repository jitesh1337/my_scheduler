/*
 * myqueue.h -- interface for queue ops
 */

#ifndef MYQUEUE_H
#define MYQUEUE_H

#include <malloc.h>
#include <stdio.h>

typedef struct mythread_queue {
  void *item;
  struct mythread_queue *prev, *next;
} *mythread_queue_t;

/* Initialize the queue */
void mythread_q_init(mythread_queue_t *headp, void *item);

/* Test if item in Q, return TRUE if so, FALSE o/w */
int mythread_inq(mythread_queue_t *headp, void *item);

/* return length if Q */
int mythread_lenq(mythread_queue_t *headp);

/* Enqueue the new item at the tail of any items at the same priority level */
void mythread_enq(mythread_queue_t *headp, void *item);

/* Remove elements from the queue */
void mythread_deq(mythread_queue_t *headp, void *item);

#endif /* MYQUEUE_H */
