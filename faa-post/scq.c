#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

/*
 * We use all bits set to 1 instead of all set to 0 for
 * the following reasons:
 * [1] 0 is a more legitimate array index than UINT32_MAX
 *     and overall a more common number (i.e. it is more
 *     probable that a consumer may want to insert a 0 in
 *     our queue than UINT32_MAX).
 * [2] Dequeuers can now consume entries by using an
 *     atomic OR.
 */
#define EMPTY UINT32_MAX

typedef union queue_ptr {
    uint64_t word64;
    struct {
        uint16_t safe;  // XXX: generally one-bit
        uint16_t cycle;
        uint32_t value;
    } fields;
} queue_ptr_t;

typedef struct queue {
    _Atomic(size_t) q_head;
    _Atomic(size_t) q_tail;
    _Atomic(uint64_t) *q_queue;
    _Atomic(int64_t) q_threshold;
    size_t q_size;
    size_t q_nelems;
} queue_t;

void
queue_init(queue_t *q, size_t size)
{
    q->q_nelems = size;
    q->q_size = 2 * size;
    atomic_init(&q->q_head, q->q_size);
    atomic_init(&q->q_tail, q->q_size);
    q->q_queue = calloc(q->q_size, sizeof(_Atomic(queue_ptr_t)));
    queue_ptr_t init_value = {
        .fields = {
            .safe = 1,
            .cycle = 0,
            .value = EMPTY,
        },
    };
    for (size_t i = 0; i < q->q_size; i++) {
        atomic_init(&q->q_queue[i], init_value.word64);
    }
    atomic_init(&q->q_threshold, -1);
}

void
queue_deinit(queue_t *q)
{
    q->q_size = q->q_nelems = 0;
    free(q->q_queue);
}

void
queue_enqueue(queue_t *q, uint32_t value)
{
    int enqueued_threshold = 3 * q->q_nelems - 1;

    while (true) {
        queue_ptr_t entry;
        size_t tail = atomic_fetch_add(&q->q_tail, 1);
        size_t tail_cycle = tail / q->q_size;
        size_t tail_index = tail % q->q_size;

retry:
        entry.word64 = atomic_load(&q->q_queue[tail_index]);
        if (entry.fields.cycle < tail_cycle &&
            entry.fields.value == EMPTY &&
            (entry.fields.safe || atomic_load(&q->q_head) <= tail)) {
            queue_ptr_t new_entry = {
                .fields = {
                    .safe = 1,
                    .cycle = tail_cycle,
                    .value = value,
                },
            };
            if (!atomic_compare_exchange_weak(&q->q_queue[tail_index],
                                              &entry.word64,
                                              new_entry.word64)) {
                goto retry;
            }
            if (atomic_load(&q->q_threshold) != enqueued_threshold) {
                atomic_store(&q->q_threshold, enqueued_threshold);
            }
            return;
        }
    }
}

static const queue_ptr_t dequeued_entry = {
    .fields = {
        .safe = 0,
        .cycle = 0,
        .value = EMPTY,
    },
};

static inline void
__queue_catchup(queue_t *q, size_t head, size_t tail)
{
    while (!atomic_compare_exchange_weak(&q->q_tail,
                                         &tail,
                                         head)) {
        head = atomic_load(&q->q_head);
        tail = atomic_load(&q->q_tail);
        if (tail >= head) {
            break;
        }
    }
}

uint32_t
queue_dequeue(queue_t *q)
{
    if (atomic_load(&q->q_threshold) < 0) {
        return 0;
    }

    while (true) {
        queue_ptr_t entry;
        size_t head = atomic_fetch_add(&q->q_head, 1);
        size_t head_index = head % q->q_size;
        size_t head_cycle = head / q->q_size;

retry:
        entry.word64 = atomic_load(&q->q_queue[head_index]);
        if (entry.fields.cycle == head_cycle) {
            atomic_fetch_or(&q->q_queue[head_index],
                            dequeued_entry.word64);
            return entry.fields.value;
        }

        queue_ptr_t new_entry = {
            .fields = {
                .safe = 0,
                .cycle = entry.fields.cycle,
                .value = entry.fields.value,
            },
        };
        if (entry.fields.value == EMPTY) {
            new_entry.fields.safe = entry.fields.safe;
            new_entry.fields.cycle = head_cycle;
            new_entry.fields.value = EMPTY;
        }
        if (entry.fields.cycle < head_cycle) {
            if (!atomic_compare_exchange_weak(&q->q_queue[head_index],
                                              &entry.word64,
                                              new_entry.word64)) {
                goto retry;
            }
        }

        size_t tail = atomic_load(&q->q_tail);
        if (tail <= (head + 1)) {
            __queue_catchup(q, head + 1, tail);
            atomic_fetch_sub(&q->q_threshold, 1);
            return EMPTY;
        }
        if (atomic_fetch_sub(&q->q_threshold, 1) <= 0) {
            return EMPTY;
        }
    }
}

void
queue_debug_print(queue_t *q)
{
    size_t head = atomic_load(&q->q_head);
    size_t tail = atomic_load(&q->q_tail);
    int64_t thold = atomic_load(&q->q_threshold);

    printf("[\n");
    for (size_t i = 0; i < q->q_size; i++) {
        queue_ptr_t p = { .word64 =  atomic_load(&q->q_queue[i]) };
        printf("%4zu: %16llX {cycle: %2X, safe: %1X, value: %8X}",
               i, p.word64, p.fields.cycle, p.fields.safe, p.fields.value);
        if (head % q->q_size == i) {
            printf(" <- H");
        }
        if (tail % q->q_size == i) {
            printf(" <- T");
        }
        printf("\n");
    }
    printf("]\n");
    printf("threshold = %lld\n", thold);
}

int
main(int c, char *v[])
{
    queue_t q;
    queue_init(&q, 4);
    queue_enqueue(&q, 0xA);
    queue_enqueue(&q, 0xB);
    printf("dequeued: %u\n", queue_dequeue(&q));
    printf("dequeued: %u\n", queue_dequeue(&q));
    printf("dequeued: %u\n", queue_dequeue(&q));
    printf("dequeued: %u\n", queue_dequeue(&q));
    printf("dequeued: %u\n", queue_dequeue(&q));
    // queue_enqueue(&q, 0xC);
    // printf("dequeued: %u\n", queue_dequeue(&q));
    // queue_enqueue(&q, 0xD);
    // queue_enqueue(&q, 0xE);
    // queue_enqueue(&q, 0xF);
    // queue_enqueue(&q, 0x0);
    // queue_enqueue(&q, 0x1);
    // queue_enqueue(&q, 0x2);
    // queue_enqueue(&q, 0x3);
    // queue_enqueue(&q, 0x4);
    // printf("dequeued: %u\n", queue_dequeue(&q));
    queue_debug_print(&q);
    queue_deinit(&q);
    return 0;
}
