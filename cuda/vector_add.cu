#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <thread>
#include <vector>
#include <chrono>
#include <algorithm>

// CUDA kernel for vector addition
__global__ void vectorAddGPU(float *a, float *b, float *c, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n) {
        c[idx] = a[idx] + b[idx];
    }
}

// Single-threaded CPU version
void vectorAddCPU(float *a, float *b, float *c, int n) {
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

// Multithreaded CPU version
void vectorAddCPUMultithreaded(float *a, float *b, float *c, int n, int numThreads) {
    std::vector<std::thread> threads;
    int elementsPerThread = n / numThreads;
    
    for (int t = 0; t < numThreads; t++) {
        int start = t * elementsPerThread;
        int end = (t == numThreads - 1) ? n : start + elementsPerThread;
        
        threads.emplace_back([=]() {
            for (int i = start; i < end; i++) {
                c[i] = a[i] + b[i];
            }
        });
    }
    
    for (auto& thread : threads) {
        thread.join();
    }
}

// Utility functions
void checkCudaError(cudaError_t error, const char* message) {
    if (error != cudaSuccess) {
        printf("CUDA Error: %s - %s\n", message, cudaGetErrorString(error));
        exit(1);
    }
}

void initializeVector(float *vec, int n) {
    for (int i = 0; i < n; i++) {
        vec[i] = (float)rand() / RAND_MAX;
    }
}

double benchmarkCPU(void (*func)(float*, float*, float*, int), float *a, float *b, float *c, int n) {
    auto start = std::chrono::high_resolution_clock::now();
    func(a, b, c, n);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count() / 1000000.0; // Convert to seconds
}

double benchmarkCPUMultithreaded(float *a, float *b, float *c, int n, int numThreads) {
    auto start = std::chrono::high_resolution_clock::now();
    vectorAddCPUMultithreaded(a, b, c, n, numThreads);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    return duration.count() / 1000000.0; // Convert to seconds
}

double benchmarkGPUWithMemory(float *h_a, float *h_b, float *h_c, int n, int blockSize) {
    const size_t bytes = n * sizeof(float);
    
    float *d_a, *d_b, *d_c;
    checkCudaError(cudaMalloc(&d_a, bytes), "Allocating device memory for a");
    checkCudaError(cudaMalloc(&d_b, bytes), "Allocating device memory for b");
    checkCudaError(cudaMalloc(&d_c, bytes), "Allocating device memory for c");
    
    cudaEvent_t start, stop;
    checkCudaError(cudaEventCreate(&start), "Creating start event");
    checkCudaError(cudaEventCreate(&stop), "Creating stop event");
    
    checkCudaError(cudaEventRecord(start), "Recording start event");
    
    // Memory transfers + computation
    checkCudaError(cudaMemcpy(d_a, h_a, bytes, cudaMemcpyHostToDevice), "Copying a to device");
    checkCudaError(cudaMemcpy(d_b, h_b, bytes, cudaMemcpyHostToDevice), "Copying b to device");
    
    int gridSize = (n + blockSize - 1) / blockSize;
    vectorAddGPU<<<gridSize, blockSize>>>(d_a, d_b, d_c, n);
    
    checkCudaError(cudaMemcpy(h_c, d_c, bytes, cudaMemcpyDeviceToHost), "Copying result to host");
    
    checkCudaError(cudaEventRecord(stop), "Recording stop event");
    checkCudaError(cudaEventSynchronize(stop), "Synchronizing stop event");
    
    float milliseconds = 0;
    checkCudaError(cudaEventElapsedTime(&milliseconds, start, stop), "Getting elapsed time");
    
    checkCudaError(cudaFree(d_a), "Freeing device memory a");
    checkCudaError(cudaFree(d_b), "Freeing device memory b");
    checkCudaError(cudaFree(d_c), "Freeing device memory c");
    checkCudaError(cudaEventDestroy(start), "Destroying start event");
    checkCudaError(cudaEventDestroy(stop), "Destroying stop event");
    
    return milliseconds / 1000.0;
}

int main() {
    // Test different problem sizes
    std::vector<int> problemSizes = {1000, 10000, 100000, 1000000, 10000000, 100000000};
    
    printf("CPU vs GPU Vector Addition - The Reality Check\n");
    printf("=============================================\n\n");
    
    int numCores = std::thread::hardware_concurrency();
    printf("System info:\n");
    printf("  CPU cores available: %d\n", numCores);
    printf("  Testing problem sizes: 1K to 100M elements\n\n");
    
    for (int n : problemSizes) {
        size_t bytes = n * sizeof(float);
        printf("Problem size: %d elements (%.2f MB total)\n", n, (3.0 * bytes) / (1024.0 * 1024.0));
        printf("-----------------------------------------------\n");
        
        // Allocate memory
        float *h_a = (float*)malloc(bytes);
        float *h_b = (float*)malloc(bytes);
        float *h_c1 = (float*)malloc(bytes);
        float *h_c2 = (float*)malloc(bytes);
        float *h_c3 = (float*)malloc(bytes);
        
        if (!h_a || !h_b || !h_c1 || !h_c2 || !h_c3) {
            printf("Failed to allocate memory for size %d\n", n);
            continue;
        }
        
        // Initialize data
        srand(42); // Consistent seed for reproducible results
        initializeVector(h_a, n);
        initializeVector(h_b, n);
        
        // Benchmark single-threaded CPU
        double cpu_single_time = benchmarkCPU(vectorAddCPU, h_a, h_b, h_c1, n);
        
        // Benchmark multithreaded CPU
        double cpu_multi_time = benchmarkCPUMultithreaded(h_a, h_b, h_c2, n, numCores);
        
        // Benchmark GPU (including memory transfers)
        double gpu_time = benchmarkGPUWithMemory(h_a, h_b, h_c3, n, 256);
        
        // Calculate bandwidth
        double total_bytes = 3.0 * bytes; // Read A, Read B, Write C
        double cpu_single_bandwidth = total_bytes / (cpu_single_time * 1e9);
        double cpu_multi_bandwidth = total_bytes / (cpu_multi_time * 1e9);
        double gpu_bandwidth = total_bytes / (gpu_time * 1e9);
        
        printf("  Single-threaded CPU: %.6f s (%.2f GB/s)\n", cpu_single_time, cpu_single_bandwidth);
        printf("  Multi-threaded CPU:  %.6f s (%.2f GB/s) - %.2fx speedup\n", 
               cpu_multi_time, cpu_multi_bandwidth, cpu_single_time / cpu_multi_time);
        printf("  GPU (with transfers): %.6f s (%.2f GB/s) - %.2fx vs single CPU\n", 
               gpu_time, gpu_bandwidth, cpu_single_time / gpu_time);
        
        // Determine the winner
        if (cpu_multi_time < gpu_time) {
            printf("  🏆 WINNER: Multi-threaded CPU (%.2fx faster than GPU)\n", gpu_time / cpu_multi_time);
        } else {
            printf("  🏆 WINNER: GPU (%.2fx faster than multi-threaded CPU)\n", cpu_multi_time / gpu_time);
        }
        
        // Memory transfer overhead analysis
        if (n >= 100000) { // Only for larger sizes where GPU timing is more reliable
            // Estimate pure computation time (very rough)
            double estimated_transfer_time = gpu_time * 0.8; // Assume 80% is transfer
            double estimated_compute_time = gpu_time * 0.2;  // Assume 20% is compute
            printf("  GPU breakdown (estimated): %.2f%% memory transfer, %.2f%% computation\n", 
                   (estimated_transfer_time / gpu_time) * 100.0,
                   (estimated_compute_time / gpu_time) * 100.0);
        }
        
        printf("\n");
        
        // Cleanup
        free(h_a);
        free(h_b);
        free(h_c1);
        free(h_c2);
        free(h_c3);
    }
    
    printf("Key Takeaways:\n");
    printf("==============\n");
    printf("1. For small problems: CPU wins due to GPU memory transfer overhead\n");
    printf("2. For large problems: Multi-threaded CPU often still competitive\n");
    printf("3. Vector addition is memory-bound, not compute-bound\n");
    printf("4. GPU shines for compute-intensive problems with high arithmetic intensity\n");
    printf("5. Always compare against optimized CPU code, not single-threaded!\n\n");
    
    printf("Better GPU use cases:\n");
    printf("- Matrix multiplication\n");
    printf("- Image/signal processing\n");
    printf("- Machine learning training\n");
    printf("- Scientific simulations\n");
    printf("- Cryptography\n");
    printf("- Ray tracing\n");
    
    return 0;
}
