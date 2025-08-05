#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <string.h>

// Use a smaller size as this test is more intensive
#define BUFFER_SIZE (512 * 1024 * 1024)
#define ACCESS_COUNT 500000000L

// Function to shuffle an array of indices
void shuffle(size_t *array, size_t n) {
    if (n > 1) {
        for (size_t i = 0; i < n - 1; i++) {
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
            size_t t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <standard|huge>\n", argv[0]);
        return 1;
    }

    int use_huge_pages = (argv[1][0] == 'h');
    size_t *buffer;

    // Allocate memory
    if (use_huge_pages) {
        printf("Testing with Huge Pages (Pointer Chasing)...\n");
        buffer = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
        if (buffer == MAP_FAILED) {
            perror("mmap failed"); return 1;
        }
    } else {
        printf("Testing with standard 4KB pages (Pointer Chasing)...\n");
        buffer = malloc(BUFFER_SIZE);
        if (buffer == NULL) {
            perror("malloc failed"); return 1;
        }
    }

    // --- Setup the pointer chain ---
    printf("Setting up pointer chase chain...\n");
    size_t num_elements = BUFFER_SIZE / sizeof(size_t);
    for (size_t i = 0; i < num_elements; ++i) {
        buffer[i] = i;
    }
    srand(time(NULL));
    shuffle(buffer, num_elements);

    // --- The Workload: Chase pointers randomly ---
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    size_t next_idx = buffer[0];
    for (long i = 0; i < ACCESS_COUNT; ++i) {
        next_idx = buffer[next_idx];
    }
    // The volatile keyword here is a trick to ensure the compiler
    // doesn't optimize away the entire loop.
    volatile size_t final_val = next_idx; 

    clock_gettime(CLOCK_MONOTONIC, &end);
    // --- End Workload ---

    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Time taken: %.4f seconds\n", time_taken);

    if (use_huge_pages) {
        munmap(buffer, BUFFER_SIZE);
    } else {
        free(buffer);
    }

    return 0;
}
