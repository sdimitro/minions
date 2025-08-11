// File: numa_benchmark.c
//
// Description: A C program to demonstrate the performance impact of NUMA locality
// when using huge pages. It benchmarks memory access speed when a process
// pinned to one NUMA node accesses huge pages allocated on the same node (local)
// versus a different node (remote).
//
// Requires a multi-socket NUMA system with huge pages allocated on at least
// two nodes.

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <string.h>
#include <sched.h>
#include <numa.h>
#include <numaif.h>

#define BUFFER_SIZE (1 * 1024 * 1024 * 1024)
#define PAGE_SIZE (4 * 1024)
#define BENCHMARK_LOOPS 100

// Benchmark function (same as before)
void access_buffer(char *buffer) {
    for (int i = 0; i < BENCHMARK_LOOPS; i++) {
        for (long j = 0; j < BUFFER_SIZE; j += PAGE_SIZE) {
            buffer[j] = (char)i;
        }
    }
}

int main() {
    struct timespec start, end;
    double time_local, time_remote;
    char *buffer;
    cpu_set_t cpu_mask;

    // --- System Validation ---
    if (numa_available() < 0) {
        printf("Error: NUMA is not available on this system.\n");
        return 1;
    }

    int num_nodes = numa_num_configured_nodes();
    if (num_nodes < 2) {
        printf("Error: This benchmark requires at least 2 NUMA nodes. Found %d.\n", num_nodes);
        return 1;
    }
    printf("System has %d NUMA nodes.\n", num_nodes);

    // --- Pin Process to a CPU on Node 0 ---
    int target_cpu = 0; // Assuming CPU 0 is on Node 0
    CPU_ZERO(&cpu_mask);
    CPU_SET(target_cpu, &cpu_mask);
    if (sched_setaffinity(0, sizeof(cpu_set_t), &cpu_mask)!= 0) {
        perror("sched_setaffinity failed");
        return 1;
    }
    printf("Process pinned to CPU %d on NUMA Node %d.\n\n", target_cpu, numa_node_of_cpu(target_cpu));

    // --- Benchmark 1: Local Huge Page Access ---
    printf("--- Testing Local Access (CPU on Node 0, Memory on Node 0) ---\n");
    struct bitmask *nodemask_local = numa_allocate_nodemask();
    numa_bitmask_setbit(nodemask_local, 0); // Set policy to bind to Node 0

    if (set_mempolicy(MPOL_BIND, nodemask_local->maskp, nodemask_local->size + 1)!= 0) {
        perror("set_mempolicy for local failed");
        numa_free_nodemask(nodemask_local);
        return 1;
    }

    buffer = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE,
                  // MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (buffer == MAP_FAILED) {
        perror("mmap failed for local huge pages. Ensure enough huge pages are free on Node 0");
        numa_free_nodemask(nodemask_local);
        return 1;
    }
    memset(buffer, 0, BUFFER_SIZE);

    clock_gettime(CLOCK_MONOTONIC, &start);
    access_buffer(buffer);
    clock_gettime(CLOCK_MONOTONIC, &end);

    time_local = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Time taken (local access): %.4f seconds\n\n", time_local);

    munmap(buffer, BUFFER_SIZE);
    numa_free_nodemask(nodemask_local);

    return 0;
}
