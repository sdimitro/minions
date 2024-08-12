#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

typedef struct queue {
    _Atomic(size_t) q_head;
    _Atomic(size_t) q_tail;
    size_t q_size;
    _Atomic(uintptr_t) *q_queue;
} queue_t;

#define UNSET_VALUE 0
#define CONSUMED_VALUE UINTPTR_MAX

void
queue_init(queue_t *q, size_t size)
{
    q->q_size = size;
    atomic_init(&q->q_head, 0);
    atomic_init(&q->q_tail, 0);
    q->q_queue = calloc(size, sizeof(_Atomic(uintptr_t)));
    for (size_t i = 0; i < q->q_size; i++) {
        atomic_init(&q->q_queue[i], UNSET_VALUE);
    }
}

void
queue_deinit(queue_t *q)
{
    q->q_size = 0;
    free(q->q_queue);
}

void
queue_enqueue(queue_t *q, uintptr_t value)
{
    while (true) {
        size_t tail = atomic_fetch_add(&q->q_tail, 1);
        /*
         * Serapheim note:
         * The snipped below could use CAS instead but SWAP is faster
         * and it doesn't hurt if we set some CONSUMED values with `value`.
         */
        /* Repeat the loop if entry already invalidated by dequeue() */
        if (atomic_exchange(&q->q_queue[tail], value) == UNSET_VALUE) {
            return;
        }
    }
}

uintptr_t
queue_dequeue(queue_t *q)
{
    while (true) {
        size_t head = atomic_fetch_add(&q->q_head, 1);
        uintptr_t value = atomic_exchange(&q->q_queue[head], CONSUMED_VALUE);
        if (value != UNSET_VALUE) {
            return value;
        }

        size_t tail = atomic_load(&q->q_tail);
        if (tail <= (head + 1)) {
            return 0;
        }
    }
}

void
queue_debug_print(queue_t *q)
{
    size_t head = atomic_load(&q->q_head);
    size_t tail = atomic_load(&q->q_tail);

    printf("[\n");
    for (size_t i = 0; i < q->q_size; i++) {
        uintptr_t p =  atomic_load(&q->q_queue[i]);
        printf("%zu: ", i);
        switch (p) {
            case UNSET_VALUE: {
                printf("%8s", "UNSET");
                break;
            }
            case CONSUMED_VALUE: {
                printf("%8s", "CONSUMED");
                break;
            }
            default: {
                printf("%8lX", p);
                break;
            }
        }
        if (head == i) {
            printf(" <- H");
        }
        if (tail == i) {
            printf(" <- T");
        }
        printf("\n");
    }
    printf("]\n");
}

int
main(int c, char *v[])
{
    queue_t q;
    queue_init(&q, 10);
    queue_enqueue(&q, 0xFFFA);
    queue_enqueue(&q, 0xFFFB);
    queue_dequeue(&q);
    queue_dequeue(&q);
    queue_dequeue(&q);
    queue_enqueue(&q, 0xFFFC);
    queue_dequeue(&q);
    queue_enqueue(&q, 0xFFFD);
    queue_enqueue(&q, 0xFFFE);
    queue_debug_print(&q);
    queue_deinit(&q);
    return 0;
}

/*
 * Issues with this:
 * [1] Susceptible to livelocks:
 *  Dequeuer and enqueuer both increment head and tail, but dequeuer goes
 *  first and CONSUMEs the EMPTY value, then enqueuer SWAPs and gets
 *  CONSUMED back instead of empty, starting another cycle of this.
 * [2] Slots are wasted
 * [3] Assumes an infinite queue (not bounded)
 */
