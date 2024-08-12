#include <stdatomic.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

typedef struct queue_meta {
    _Atomic(size_t) q_p;
    size_t q_n;
} queue_meta_t;

void
queue_meta_init(queue_meta_t *q, size_t size)
{
    q->q_n = size;
    atomic_init(&q->q_p, 0);
}

void
queue_meta_deinit(queue_meta_t *q)
{
    q->q_n = 0;
}

void
queue_meta_faa(queue_meta_t *q)
{
   atomic_fetch_add(&q->q_p, 1);
}

void
queue_meta_debug_print(queue_meta_t *q)
{
    size_t p = atomic_load(&q->q_p);

    printf("p %8lu | / %8lu | %% %8lu | cycle %8lu | idx %8lu | map %8lu\n",
           p, p / q->q_n, p % q->q_n,
           (p << 1) | (2 * q->q_n - 1), /* cycle */
           (p & (q->q_n - 1)), /* idx */
           (p & (q->q_n - 1)) & (q->q_n - 1)); /* map (no order) - (idx & (n - 1) */
}

/* See https://docs.rs/crossbeam-utils/latest/crossbeam_utils/struct.CachePadded.html for more info*/
void
dump_machine_info(void)
{
#if defined(__x86_64__)
    char *arch = "x86_64";
# define LFATOMIC_LOG2			3
# define LFATOMIC_WIDTH			64
# define LFATOMIC_BIG_WIDTH		128
# define __LFLOAD_SPLIT(dtype_width)	(dtype_width > LFATOMIC_WIDTH)
# define __LFCMPXCHG_SPLIT(dtype_width)	0
#define LF_CACHE_SHIFT		7U

#elif defined(__aarch64__)
    char *arch = "AArch64";
# define LFATOMIC_LOG2			3
# define LFATOMIC_WIDTH			64
# define LFATOMIC_BIG_WIDTH		128
# define __LFLOAD_SPLIT(dtype_width)	(dtype_width > LFATOMIC_WIDTH)
# define __LFCMPXCHG_SPLIT(dtype_width)	0
#define LF_CACHE_SHIFT		7U

#else
    char *arch = "Unknown";
# define __LFLOAD_SPLIT(dtype_width)	0
# define __LFCMPXCHG_SPLIT(dtype_width)	0
# if UINTPTR_MAX == UINT32_C(0xFFFFFFFF)
#  define LFATOMIC_LOG2			2
#  define LFATOMIC_WIDTH		32
#  define LFATOMIC_BIG_WIDTH	32
# elif UINTPTR_MAX == UINT64_C(0xFFFFFFFFFFFFFFFF)
#  define LFATOMIC_LOG2			3
#  define LFATOMIC_WIDTH		64
#  define LFATOMIC_BIG_WIDTH	64
# endif
#define LF_CACHE_SHIFT		6U
#endif

#define LF_CACHE_BYTES		(1U << LF_CACHE_SHIFT)

/* XXX: True for x86/x86-64 but needs to be properly defined for other CPUs. */
// 
// /* Allow to use LEA for x86/x86-64. */
// #if defined(__i386__) || defined(__x86_64__)
// # define __LFMERGE(x,y)	((x) + (y))
// #else
// # define __LFMERGE(x,y)	((x) | (y))
// #endif
    printf("ARCH: %s\n", arch);
    printf("Cache line shift: %d\n", LF_CACHE_SHIFT);
    printf("Cache line length (bytes): %d\n", LF_CACHE_BYTES);
}

int
main(int c, char *v[])
{
    dump_machine_info();

    queue_meta_t q;
    queue_meta_init(&q, 8);
    queue_meta_debug_print(&q);
    for (int i = 0; i < 18; i++) {
        queue_meta_faa(&q);
        queue_meta_debug_print(&q);
    }
    queue_meta_deinit(&q);
    return 0;
}
