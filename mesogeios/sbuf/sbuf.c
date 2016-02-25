#include "sbuf.h"

void sbuf_init(sbuf_t *sp, int n)
{
	sp->buf = (void **) calloc(n, sizeof(void *));
	sp->n = n;                          /* buffer holds max of n elements */
	sp->front = sp->rear = 0;           /* empty buffer iff front == rear */
	pthread_mutex_init(&sp->mtx, NULL); /* mutex for locking */
	sem_init(&sp->slots, 0, n);         /* initially, buf has n empty slots */
	sem_init(&sp->elts, 0, 0);          /* initially, buf has zero elements */
}

void sbuf_deinit(sbuf_t *sp)
{
	free(sp->buf);
	pthread_mutex_destroy(&sp->mtx);
	sem_destroy(&sp->slots);
	sem_destroy(&sp->elts);
}

void sbuf_insert(sbuf_t *sp, void *elt)
{
	sem_wait(&sp->slots);                /* wait for available slot */
	pthread_mutex_lock(&sp->mtx);        /* lock the buffer */
	sp->buf[(++sp->rear)%(sp->n)] = elt; /* insert element in buf */
	pthread_mutex_unlock(&sp->mtx);      /* unlock the buffer */
	sem_post(&sp->elts);                 /* announce available element */
}

void *sbuf_remove(sbuf_t *sp)
{
	void *elt;

	sem_wait(&sp->elts);                  /* wait for available element */
	pthread_mutex_lock(&sp->mtx);         /* lock the buffer */
	elt = sp->buf[(++sp->front)%(sp->n)]; /* insert element in buf */
	pthread_mutex_unlock(&sp->mtx);       /* unlock the buffer */
	sem_post(&sp->slots);                 /* announce available element */

	return elt;
}
