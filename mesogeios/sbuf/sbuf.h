#ifndef __SBUF_H
#define __SBUF_H

/*
 * A bounded buffer interface -- Producer-Consumer Problem
 * (Adjusted from CS:APP2 from Bryant & O'Hallaron)
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>

typedef struct {
  void **buf;          /* buffer array */
  int n;               /* maximum number of slots */
  int front;           /* buf[(front+1)%n] is the first element */
  int rear;            /* buf[rear%n] is last element */
  pthread_mutex_t mtx; /* protects accesses to buf */
  sem_t slots;         /* counts available slots */
  sem_t elts;          /* counts available elements */
} sbuf_t;

/* create an empty, bounded, shared FIFO buffer with n slots */
extern void sbuf_init(sbuf_t *sp, int n);

/* clean up buffer sp */
extern void sbuf_deinit(sbuf_t *sp);

/* insert element onto the rear of shared buffer sp */
extern void sbuf_insert(sbuf_t *sp, void *elt);

/* remove and return the first element from buffer sp */
extern void *sbuf_remove(sbuf_t *sp);

#endif /* __SBUF_H */
