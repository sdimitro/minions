#include <immintrin.h> // For SIMD intrinsics (__m256, _mm256_loadu_ps, etc.)
#include <stdio.h>     // For printf, fprintf, stderr
#include <stdlib.h>    // For _mm_malloc, _mm_free (aligned memory allocation)
#include <time.h>      // For struct timespec, clock_gettime, CLOCK_MONOTONIC
#include <omp.h>       // For omp_get_max_threads

// Original function (for comparison) - Declare it before main or where it's used
void vectorAddCPU(float *a, float *b, float *c, int n) {
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

// Optimized SIMD function
void vectorAddCPU_SIMD_unaligned(float *a, float *b, float *c, int n) {
    int i;
    // Process 8 floats at a time using AVX (256-bit registers)
    for (i = 0; i + 7 < n; i += 8) {
        __m256 va = _mm256_loadu_ps(&a[i]);
        __m256 vb = _mm256_loadu_ps(&b[i]);
        __m256 vc = _mm256_add_ps(va, vb);
        _mm256_storeu_ps(&c[i], vc);
    }

    // Handle the remaining elements (if n is not a multiple of 8)
    for (; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

// Optimized SIMD function
void vectorAddCPU_SIMD(float *a, float *b, float *c, int n) {
    int i;
    // Process 8 floats at a time using AVX (256-bit registers)
    for (i = 0; i + 7 < n; i += 8) {
        __m256 va = _mm256_load_ps(&a[i]);
        __m256 vb = _mm256_load_ps(&b[i]);
        __m256 vc = _mm256_add_ps(va, vb);
        _mm256_store_ps(&c[i], vc);
    }

    // Handle the remaining elements (if n is not a multiple of 8)
    for (; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

// OMP + SIMD | -fopenmp -march=native -mavx2
void vectorAddCPU_SIMD_OMP(float *a, float *b, float *c, int n) {
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

// For timing
double get_time() {
    struct timespec ts;
    // clock_gettime is part of <time.h>
    // CLOCK_MONOTONIC is also defined in <time.h>
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

int main() {
    int n = 1024 * 1024; // Large enough for a good test
    float *a, *b, *c_ref, *c_simdu, *c_simd, *c_ompsimd;

    // Allocate memory using _mm_malloc for alignment
    // _mm_malloc and _mm_free are defined in <stdlib.h> and sometimes <malloc.h>
    // but <stdlib.h> is usually sufficient with intrinsics
    a = (float*)_mm_malloc(n * sizeof(float), 32); // Use _mm_malloc for aligned memory (32-byte for AVX)
    b = (float*)_mm_malloc(n * sizeof(float), 32);
    c_ref = (float*)_mm_malloc(n * sizeof(float), 32);
    c_simdu = (float*)_mm_malloc(n * sizeof(float), 32);
    c_simd = (float*)_mm_malloc(n * sizeof(float), 32);
    c_ompsimd = (float*)_mm_malloc(n * sizeof(float), 32);

    if (!a || !b || !c_ref || !c_simd) {
        // fprintf and stderr are defined in <stdio.h>
        fprintf(stderr, "Memory allocation failed!\n");
        return 1;
    }

    // Initialize data
    for (int i = 0; i < n; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
    }

    // --- Original function timing ---
    double start_time_ref = get_time();
    for (int iter = 0; iter < 100; ++iter) { // Run multiple times for better average
        vectorAddCPU(a, b, c_ref, n);
    }
    double end_time_ref = get_time();
    // printf is defined in <stdio.h>
    printf("Original (scalar) time: %f seconds\n", (end_time_ref - start_time_ref) / 100.0);

    // --- SIMD unaligned function timing ---
    double start_time_simd = get_time();
    for (int iter = 0; iter < 100; ++iter) { // Run multiple times for better average
        vectorAddCPU_SIMD_unaligned(a, b, c_simdu, n);
    }
    double end_time_simd = get_time();
    printf("SIMDu (AVX) time:      %f seconds\n", (end_time_simd - start_time_simd) / 100.0);

    // --- SIMD function timing ---
    start_time_simd = get_time();
    for (int iter = 0; iter < 100; ++iter) { // Run multiple times for better average
        vectorAddCPU_SIMD(a, b, c_simd, n);
    }
    end_time_simd = get_time();
    printf("SIMD (AVX) time:      %f seconds\n", (end_time_simd - start_time_simd) / 100.0);

    // --- OMP+SIMD (max threads) function timing ---
    double start_time_ompsimd = get_time();
    for (int iter = 0; iter < 100; ++iter) { // Run multiple times for better average
        vectorAddCPU_SIMD_OMP(a, b, c_ompsimd, n);
    }
    double end_time_ompsimd = get_time();
    printf("OMP+SIMD (max threads=%d) time:      %f seconds\n", omp_get_max_threads(), (end_time_ompsimd - start_time_ompsimd) / 100.0);

    // --- OMP+SIMD (2 threads) function timing ---
    omp_set_num_threads(2);
    start_time_ompsimd = get_time();
    for (int iter = 0; iter < 100; ++iter) { // Run multiple times for better average
        vectorAddCPU_SIMD_OMP(a, b, c_ompsimd, n);
    }
    end_time_ompsimd = get_time();
    printf("OMP+SIMD (threads=%d) time:      %f seconds\n", 2, (end_time_ompsimd - start_time_ompsimd) / 100.0);

    // --- OMP+SIMD (4 threads) function timing ---
    omp_set_num_threads(4);
    start_time_ompsimd = get_time();
    for (int iter = 0; iter < 100; ++iter) { // Run multiple times for better average
        vectorAddCPU_SIMD_OMP(a, b, c_ompsimd, n);
    }
    end_time_ompsimd = get_time();
    printf("OMP+SIMD (threads=%d) time:      %f seconds\n", 4, (end_time_ompsimd - start_time_ompsimd) / 100.0);

    // Verify correctness (optional)
    for (int i = 0; i < n; i++) {
        if (c_ref[i] != c_simdu[i]) {
            fprintf(stderr, "Error: Mismatch at index %d! c_ref=%f, c_simd=%f\n", i, c_ref[i], c_simd[i]);
            break;
        }
    }
    for (int i = 0; i < n; i++) {
        if (c_ref[i] != c_simd[i]) {
            fprintf(stderr, "Error: Mismatch at index %d! c_ref=%f, c_simd=%f\n", i, c_ref[i], c_simd[i]);
            break;
        }
    }
    for (int i = 0; i < n; i++) {
        if (c_ref[i] != c_ompsimd[i]) {
            fprintf(stderr, "Error: Mismatch at index %d! c_ref=%f, c_simd=%f\n", i, c_ref[i], c_simd[i]);
            break;
        }
    }
    printf("Verification complete.\n");

    // Free memory using _mm_free
    _mm_free(a);
    _mm_free(b);
    _mm_free(c_ref);
    _mm_free(c_simd);

    return 0;
}
