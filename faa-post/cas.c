#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

typedef union queue_ptr {
    uint64_t word64;
    struct {
        uint32_t cycle;
        uint32_t value;
    } fields;
} queue_ptr_t;

typedef struct queue {
    _Atomic(size_t) q_head;
    _Atomic(size_t) q_tail;
    size_t q_size;
    _Atomic(uint64_t) *q_queue;
} queue_t;

void
queue_init(queue_t *q, size_t size)
{
    q->q_size = size;
    atomic_init(&q->q_head, q->q_size);
    atomic_init(&q->q_tail, q->q_size);
    q->q_queue = calloc(size, sizeof(_Atomic(queue_ptr_t)));
    for (size_t i = 0; i < q->q_size; i++) {
        atomic_init(&q->q_queue[i], 0);
    }
}

void
queue_deinit(queue_t *q)
{
    q->q_size = 0;
    free(q->q_queue);
}

void
queue_enqueue(queue_t *q, uint32_t value)
{
    queue_ptr_t old_entry, new_entry;
    size_t tail, tail_index;

start_over:
    tail = atomic_load(&q->q_tail);
    do {
        tail_index = tail % q->q_size; // XXX: remap
        size_t tail_cycle = tail / q->q_size; // XXX: tail & ~(q->q_size - 1);
        old_entry.word64 = atomic_load(&q->q_queue[tail_index]);
        size_t old_entry_cycle = old_entry.fields.cycle;

        if (old_entry_cycle == tail_cycle) {
            /*
             * We've already enqueued that slot in this cycle.
             * Move to the next slot if you can. If the the
             * following CAS fails that's ok. It means that
             * someone else advanced this pointer for us.
             */
            atomic_compare_exchange_weak(&q->q_tail, &tail, tail + 1);
            goto start_over;
        }
        if ((old_entry_cycle + 1) != tail_cycle) {
            /*
             * Our `tail` is very stale. There have been at
             * least q_size enqueues since we loaded it.
             */
            goto start_over;
        }

        /*
         * If the following CAS fails, it means that another
         * enqueuer has changed queue[tail_index] (dequeue
         * does not modify entries).
         */
        new_entry.fields.value = value;
        new_entry.fields.cycle = tail_cycle;
    } while(!atomic_compare_exchange_weak(&q->q_queue[tail_index],
                                          &old_entry.word64, new_entry.word64));
    /* 
     * Move tail foward. If this fails, that's fine because
     * it would mean that another thread did it for us in
     * their old_entry_cycle == tail_cycle case.
     */
    atomic_compare_exchange_weak(&q->q_tail, &tail, tail + 1);
}

uint32_t
queue_dequeue(queue_t *q)
{
    queue_ptr_t entry;
    size_t head;

start_over:
    head = atomic_load(&q->q_head);
    do {
        size_t head_index = head % q->q_size; // XXX: remap
        size_t head_cycle = head / q->q_size; // XXX: head & ~(q->q_size - 1);
        entry.word64 = atomic_load(&q->q_queue[head_index]);
        size_t entry_cycle = entry.fields.cycle;

        if (entry_cycle != head_cycle) {
            if ((entry_cycle + 1) == head_cycle) {
                /* the queue is empty */
                return 0; // XXX; optional
            }
            goto start_over;
        }
        /*
         * If the following CAS fails, it means that the header pointer was
         * modified by another dequeuer.
         */
    } while (!atomic_compare_exchange_weak(&q->q_head, &head, head + 1));

    return entry.fields.value;
}

void
queue_debug_print(queue_t *q)
{
    size_t head = atomic_load(&q->q_head);
    size_t tail = atomic_load(&q->q_tail);

    printf("[\n");
    for (size_t i = 0; i < q->q_size; i++) {
        queue_ptr_t p = { .word64 =  atomic_load(&q->q_queue[i]) };
        printf("%zu: %8llX {cycle: %X, value: %X}", i, p.word64, p.fields.cycle, p.fields.value);
        if (head % q->q_size == i) {
            printf(" <- H");
        }
        if (tail % q->q_size == i) {
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
    queue_init(&q, 8);
    printf("Dequeued: %X\n", queue_dequeue(&q));
    queue_enqueue(&q, 0x2);
    queue_enqueue(&q, 0x3);
    printf("Dequeued: %X\n", queue_dequeue(&q));
    printf("Dequeued: %X\n", queue_dequeue(&q));
    printf("Dequeued: %X\n", queue_dequeue(&q));
    printf("Dequeued: %X\n", queue_dequeue(&q));
    queue_enqueue(&q, 0x4);
    queue_enqueue(&q, 0x5);
    queue_enqueue(&q, 0x6);
    queue_enqueue(&q, 0x7);
    queue_enqueue(&q, 0x8);
    queue_enqueue(&q, 0x9);
    queue_enqueue(&q, 0xA);
    printf("Dequeued: %X\n", queue_dequeue(&q));
    queue_enqueue(&q, 0xB);
    queue_enqueue(&q, 0xC);
    printf("Dequeued: %X\n", queue_dequeue(&q));
    // queue_enqueue(&q, 0xD);
    // queue_enqueue(&q, 0xE);
    // printf("Dequeued: %X\n", queue_dequeue(&q));
    //queue_dequeue(&q);
    //queue_enqueue(&q, 0xFC0000);
    //queue_enqueue(&q, 0xFD0000);
    queue_debug_print(&q);
    queue_deinit(&q);
    return 0;
}

/*
 * Good:
 * * Queue is bounded
 * * No livelocks
 * * ABA safe
 *
 * Issues with this:
 * - Uses CAS instead of FAA.
 * - When Queue is full, the enqueuer overwrites the oldest entry
 *   screwing everything up.
 * 
 * Using weak vs strong for CAS:
 *   (ref) https://en.cppreference.com/w/c/atomic/atomic_compare_exchange
 *   Weak forms/calls may fail spuriously (i.e. even if `obj == expected`)
 *   but they yield better performance in some platforms. The rule-of-thumb
 *   is: if we can avoid a loop by using the strong form, then we do that -
 *   otherwise always use the weak form.
 *   (ref) https://ryonaldteofilo.medium.com/atomics-in-c-compare-and-swap-and-memory-order-part-2-64e127847e00
 *   Why would CAS fail even if its predicate is true? In some platforms
 *   exclusive access can be expensive but reading an atomic can be cheap
 *   (depending on the cache coherency protocol) - and in most coherency
 *   protocols only one CPU is allowed to write a cache line at a time but
 *   many CPUs can read from a cache line simultaneously. (see timed-attempts
 *   and https://en.wikipedia.org/wiki/Cache_coherence)
 *   (ref) https://stackoverflow.com/questions/72766332/c11-how-to-produce-spurious-failures-upon-compare-exchange-weak
 *   On how to reproduce spurious failures and why they don't happen on x86.
 *
 * Memory ordering:
 *   (ref: https://en.cppreference.com/w/c/atomic/memory_order)
 *   We didn't use any of the explicit forms above and the default
 *   is memory_order_seq_cst (sequentially consistent).
 */
