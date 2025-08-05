#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <numa.h> // Include the NUMA library

#define BUFFER_SIZE (512 * 1024 * 1024)
#define ACCESS_COUNT 100000000L
#define UNROLL_FACTOR 16

// The benchmark workload, now in its own function
void run_benchmark(size_t* buffer, size_t* instructions) {
    printf("Starting benchmark...\n");
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    volatile size_t sum = 0;
    for (long i = 0; i < ACCESS_COUNT / UNROLL_FACTOR; ++i) {
        sum += buffer[instructions[i*UNROLL_FACTOR+0]];
        sum += buffer[instructions[i*UNROLL_FACTOR+1]];
        sum += buffer[instructions[i*UNROLL_FACTOR+2]];
        sum += buffer[instructions[i*UNROLL_FACTOR+3]];
        sum += buffer[instructions[i*UNROLL_FACTOR+4]];
        sum += buffer[instructions[i*UNROLL_FACTOR+5]];
        sum += buffer[instructions[i*UNROLL_FACTOR+6]];
        sum += buffer[instructions[i*UNROLL_FACTOR+7]];
        sum += buffer[instructions[i*UNROLL_FACTOR+8]];
        sum += buffer[instructions[i*UNROLL_FACTOR+9]];
        sum += buffer[instructions[i*UNROLL_FACTOR+10]];
        sum += buffer[instructions[i*UNROLL_FACTOR+11]];
        sum += buffer[instructions[i*UNROLL_FACTOR+12]];
        sum += buffer[instructions[i*UNROLL_FACTOR+13]];
        sum += buffer[instructions[i*UNROLL_FACTOR+14]];
        sum += buffer[instructions[i*UNROLL_FACTOR+15]];
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Time taken: %.4f seconds\n", time_taken);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <4kb_local | huge_local>\n", argv[0]);
        return 1;
    }

    if (numa_available() < 0) {
        fprintf(stderr, "NUMA not supported on this system.\n");
        return 1;
    }
    
    printf("Pinning process to NUMA Node 0.\n");
    numa_run_on_node(0);
    // Also set a strict memory policy to only allocate on Node 0
    numa_set_membind(numa_nodes_ptr); // This is a bitmask for nodes, we will set it to only allow node 0
    struct bitmask* nodemask = numa_allocate_nodemask();
    numa_bitmask_setbit(nodemask, 0);
    numa_set_membind(nodemask);


    // --- Setup is the same for both tests ---
    printf("Setting up random access indices...\n");
    size_t* instructions = malloc(ACCESS_COUNT * sizeof(size_t));
    if(!instructions) { perror("malloc for instructions failed"); return 1; }
    for (size_t i = 0; i < ACCESS_COUNT; ++i) {
        instructions[i] = rand() % (BUFFER_SIZE / sizeof(size_t));
    }
    
    size_t *buffer = NULL;

    // --- Test Selection ---
    if (strcmp(argv[1], "4kb_local") == 0) {
        printf("Testing 4KB pages allocated on local NUMA node 0...\n");
        // numa_alloc_onnode guarantees the memory is on Node 0.
        buffer = numa_alloc_onnode(BUFFER_SIZE, 0);
        if(!buffer) { perror("numa_alloc_onnode failed"); return 1; }
        run_benchmark(buffer, instructions);
        numa_free(buffer, BUFFER_SIZE);

    } else if (strcmp(argv[1], "huge_local") == 0) {
        printf("Testing Huge Pages allocated on local NUMA node 0...\n");
        // Since we are pinned to Node 0 and set membind, the kernel
        // will satisfy this mmap from Node 0's huge page pool.
        buffer = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
        if (buffer == MAP_FAILED) {
            perror("mmap for huge pages failed"); return 1;
        }
        run_benchmark(buffer, instructions);
        munmap(buffer, BUFFER_SIZE);
    } else {
        fprintf(stderr, "Invalid test type. Use '4kb_local' or 'huge_local'.\n");
    }

    free(instructions);
    numa_free_nodemask(nodemask);
    return 0;
}
