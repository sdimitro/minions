// File: tlb_benchmark.c
//
// Description: A C program to demonstrate the performance benefit of huge pages
// by measuring the time taken to access a large memory buffer backed by
// standard 4KB pages versus 2MB huge pages. The benchmark intentionally
// causes TLB misses to highlight the performance difference.
//
// Inspired by examples from Linux documentation and online benchmarks.[18, 30]

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <string.h>

// Define a large buffer size: 8 GB
#define BUFFER_SIZE ((long)8 * 1024 * 1024 * 1024)
// Standard page size is 4KB
#define PAGE_SIZE (4 * 1024)
// Number of loops for the benchmark to run
#define BENCHMARK_LOOPS 100

// Function to perform timed access to the buffer
// It strides through the buffer, touching one element per page to
// maximize TLB misses.
void access_buffer(char *buffer) {
    for (int i = 0; i < BENCHMARK_LOOPS; i++) {
        for (long j = 0; j < BUFFER_SIZE; j += PAGE_SIZE) {
            buffer[j] = (char)i; // Write to the page
        }
    }
}

int main() {
    struct timespec start, end;
    double time_std_pages, time_huge_pages;
    char *buffer;

    printf("Benchmark starting...\n");
    printf("Buffer Size: %ld MB\n", BUFFER_SIZE / (1024 * 1024));
    printf("Benchmark Loops: %d\n\n", BENCHMARK_LOOPS);

    // --- Case 1: Standard 4KB Pages ---
    printf("Testing with standard 4KB pages...\n");

    // Allocate buffer using standard mmap
    buffer = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   if (buffer == MAP_FAILED) {
        perror("mmap failed for standard pages");
        return 1;
    }

    // Prefault pages to ensure they are resident in memory
    memset(buffer, 0, BUFFER_SIZE);

    // Run benchmark
    clock_gettime(CLOCK_MONOTONIC, &start);
    access_buffer(buffer);
    clock_gettime(CLOCK_MONOTONIC, &end);

    time_std_pages = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Time taken (standard pages): %.4f seconds\n\n", time_std_pages);

    // Clean up
    munmap(buffer, BUFFER_SIZE);


    // --- Case 2: 2MB Huge Pages ---
    printf("Testing with 2MB huge pages...\n");

    // Allocate buffer using mmap with MAP_HUGETLB flag
    buffer = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);

    if (buffer == MAP_FAILED) {
        perror("mmap failed for huge pages. Ensure huge pages are allocated and available");
        printf("Check with: cat /proc/meminfo | grep Huge\n");
        printf("Allocate with: sudo sysctl -w vm.nr_hugepages=<num>\n");
        return 1;
    }

    // Prefault pages
    memset(buffer, 0, BUFFER_SIZE);

    // Run benchmark
    clock_gettime(CLOCK_MONOTONIC, &start);
    access_buffer(buffer);
    clock_gettime(CLOCK_MONOTONIC, &end);

        time_huge_pages = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Time taken (huge pages): %.4f seconds\n\n", time_huge_pages);

    // Clean up
    munmap(buffer, BUFFER_SIZE);

    printf("--- Analysis ---\n");
    if (time_huge_pages < time_std_pages) {
        double speedup = ((time_std_pages - time_huge_pages) / time_std_pages) * 100.0;
        printf("Huge pages were %.2f%% faster.\n", speedup);
    } else {
        printf("Huge pages did not show a speedup in this run.\n");
    }

    return 0;
}
