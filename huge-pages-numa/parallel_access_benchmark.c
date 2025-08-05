#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <string.h>

#define BUFFER_SIZE (512 * 1024 * 1024)
// Total number of memory reads to perform
#define ACCESS_COUNT 100000000L
// Perform 16 independent reads per loop to create parallelism
#define UNROLL_FACTOR 16

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <standard|huge>\n", argv[0]);
        return 1;
    }

    int use_huge_pages = (argv[1][0] == 'h');
    size_t *buffer;

    if (use_huge_pages) {
        printf("Testing with Huge Pages (Parallel Access)...\n");
        buffer = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
        if (buffer == MAP_FAILED) {
            perror("mmap failed"); return 1;
        }
    } else {
        printf("Testing with standard 4KB pages (Parallel Access)...\n");
        buffer = malloc(BUFFER_SIZE);
        if (buffer == NULL) {
            perror("malloc failed"); return 1;
        }
    }

    // --- Setup random indices to access ---
    printf("Setting up random access indices...\n");
    // ***** FIX IS HERE *****
    // The instructions array needs to hold ACCESS_COUNT elements.
    size_t* instructions = malloc(ACCESS_COUNT * sizeof(size_t));
    if(!instructions) { perror("malloc for instructions failed"); return 1; }

    // ***** AND FIX IS HERE *****
    // Populate the entire instructions array.
    for (size_t i = 0; i < ACCESS_COUNT; ++i) {
        instructions[i] = rand() % (BUFFER_SIZE / sizeof(size_t));
    }

    // --- The Workload: Parallel Random Access ---
    printf("Starting benchmark...\n");
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    volatile size_t sum = 0;
    for (long i = 0; i < ACCESS_COUNT / UNROLL_FACTOR; ++i) {
        // The core of the benchmark: 16 independent reads
        // to saturate the memory controller and expose TLB overhead.
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
    // --- End Workload ---

    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Time taken: %.4f seconds\n", time_taken);

    free(instructions);
    if (use_huge_pages) {
        munmap(buffer, BUFFER_SIZE);
    } else {
        free(buffer);
    }

    return 0;
}

