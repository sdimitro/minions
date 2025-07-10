#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// CUDA kernel for vector addition
__global__ void vectorAdd(float *a, float *b, float *c, int n) {
    // Calculate global thread index
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    
    // Ensure we don't go out of bounds
    if (idx < n) {
        c[idx] = a[idx] + b[idx];
    }
}

// CPU version for comparison
void vectorAddCPU(float *a, float *b, float *c, int n) {
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

// Utility function to check CUDA errors
void checkCudaError(cudaError_t error, const char* message) {
    if (error != cudaSuccess) {
        printf("CUDA Error: %s - %s\n", message, cudaGetErrorString(error));
        exit(1);
    }
}

// Function to initialize vectors with random values
void initializeVector(float *vec, int n) {
    for (int i = 0; i < n; i++) {
        vec[i] = (float)rand() / RAND_MAX;
    }
}

// Function to verify results
bool verifyResults(float *cpu_result, float *gpu_result, int n) {
    const float epsilon = 1e-5;
    for (int i = 0; i < n; i++) {
        if (fabs(cpu_result[i] - gpu_result[i]) > epsilon) {
            printf("Verification failed at index %d: CPU=%.6f, GPU=%.6f\n", 
                   i, cpu_result[i], gpu_result[i]);
            return false;
        }
    }
    return true;
}

// Benchmark function (kernel only)
double benchmarkGPU(float *d_a, float *d_b, float *d_c, int n, int blockSize) {
    int gridSize = (n + blockSize - 1) / blockSize;
    
    // Create CUDA events for timing
    cudaEvent_t start, stop;
    checkCudaError(cudaEventCreate(&start), "Creating start event");
    checkCudaError(cudaEventCreate(&stop), "Creating stop event");
    
    // Warm-up run
    vectorAdd<<<gridSize, blockSize>>>(d_a, d_b, d_c, n);
    checkCudaError(cudaDeviceSynchronize(), "Warmup synchronization");
    
    // Benchmark run
    checkCudaError(cudaEventRecord(start), "Recording start event");
    vectorAdd<<<gridSize, blockSize>>>(d_a, d_b, d_c, n);
    checkCudaError(cudaEventRecord(stop), "Recording stop event");
    checkCudaError(cudaEventSynchronize(stop), "Synchronizing stop event");
    
    float milliseconds = 0;
    checkCudaError(cudaEventElapsedTime(&milliseconds, start, stop), "Getting elapsed time");
    
    // Cleanup events
    checkCudaError(cudaEventDestroy(start), "Destroying start event");
    checkCudaError(cudaEventDestroy(stop), "Destroying stop event");
    
    return milliseconds / 1000.0; // Convert to seconds
}

// Benchmark function including memory transfers
double benchmarkGPUWithMemory(float *h_a, float *h_b, float *h_c, int n, int blockSize) {
    const size_t bytes = n * sizeof(float);
    int gridSize = (n + blockSize - 1) / blockSize;
    
    // Create CUDA events for timing
    cudaEvent_t start, stop;
    checkCudaError(cudaEventCreate(&start), "Creating start event");
    checkCudaError(cudaEventCreate(&stop), "Creating stop event");
    
    // Allocate device memory
    float *d_a, *d_b, *d_c;
    checkCudaError(cudaMalloc(&d_a, bytes), "Allocating device memory for a");
    checkCudaError(cudaMalloc(&d_b, bytes), "Allocating device memory for b");
    checkCudaError(cudaMalloc(&d_c, bytes), "Allocating device memory for c");
    
    // Time the entire GPU pipeline
    checkCudaError(cudaEventRecord(start), "Recording start event");
    
    // Copy data to device
    checkCudaError(cudaMemcpy(d_a, h_a, bytes, cudaMemcpyHostToDevice), "Copying a to device");
    checkCudaError(cudaMemcpy(d_b, h_b, bytes, cudaMemcpyHostToDevice), "Copying b to device");
    
    // Launch kernel
    vectorAdd<<<gridSize, blockSize>>>(d_a, d_b, d_c, n);
    
    // Copy result back
    checkCudaError(cudaMemcpy(h_c, d_c, bytes, cudaMemcpyDeviceToHost), "Copying result to host");
    
    checkCudaError(cudaEventRecord(stop), "Recording stop event");
    checkCudaError(cudaEventSynchronize(stop), "Synchronizing stop event");
    
    float milliseconds = 0;
    checkCudaError(cudaEventElapsedTime(&milliseconds, start, stop), "Getting elapsed time");
    
    // Cleanup
    checkCudaError(cudaFree(d_a), "Freeing device memory a");
    checkCudaError(cudaFree(d_b), "Freeing device memory b");
    checkCudaError(cudaFree(d_c), "Freeing device memory c");
    checkCudaError(cudaEventDestroy(start), "Destroying start event");
    checkCudaError(cudaEventDestroy(stop), "Destroying stop event");
    
    return milliseconds / 1000.0; // Convert to seconds
}

int main() {
    // Vector size
    const int n = 1000000000; 
    const size_t bytes = n * sizeof(float);
    
    printf("CUDA Vector Addition Tutorial\n");
    printf("Vector size: %d elements (%.2f MB per vector)\n", n, bytes / (1024.0 * 1024.0));
    printf("===============================================\n\n");
    
    // Allocate host memory
    float *h_a = (float*)malloc(bytes);
    float *h_b = (float*)malloc(bytes);
    float *h_c_cpu = (float*)malloc(bytes);
    float *h_c_gpu = (float*)malloc(bytes);
    
    if (!h_a || !h_b || !h_c_cpu || !h_c_gpu) {
        printf("Failed to allocate host memory\n");
        return 1;
    }
    
    // Initialize vectors
    srand(time(NULL));
    initializeVector(h_a, n);
    initializeVector(h_b, n);
    
    // CPU benchmark
    printf("1. CPU Benchmark\n");
    clock_t cpu_start = clock();
    vectorAddCPU(h_a, h_b, h_c_cpu, n);
    clock_t cpu_end = clock();
    double cpu_time = ((double)(cpu_end - cpu_start)) / CLOCKS_PER_SEC;
    printf("   CPU time: %.6f seconds\n\n", cpu_time);
    
    // GPU setup
    printf("2. GPU Setup and Memory Management\n");
    
    // Allocate device memory
    float *d_a, *d_b, *d_c;
    checkCudaError(cudaMalloc(&d_a, bytes), "Allocating device memory for a");
    checkCudaError(cudaMalloc(&d_b, bytes), "Allocating device memory for b");
    checkCudaError(cudaMalloc(&d_c, bytes), "Allocating device memory for c");
    printf("   ✓ Allocated %.2f MB on GPU\n", 3 * bytes / (1024.0 * 1024.0));
    
    // Copy data to device
    checkCudaError(cudaMemcpy(d_a, h_a, bytes, cudaMemcpyHostToDevice), "Copying a to device");
    checkCudaError(cudaMemcpy(d_b, h_b, bytes, cudaMemcpyHostToDevice), "Copying b to device");
    printf("   ✓ Copied input data to GPU\n\n");
    
    // Test different block sizes
    printf("3. GPU Benchmarks - Kernel Only\n");
    int blockSizes[] = {32, 64, 128, 256, 512, 1024};
    int numBlockSizes = sizeof(blockSizes) / sizeof(blockSizes[0]);
    
    double bestTime = 1e9;
    int bestBlockSize = 0;
    
    for (int i = 0; i < numBlockSizes; i++) {
        int blockSize = blockSizes[i];
        int gridSize = (n + blockSize - 1) / blockSize;
        
        double gpu_time = benchmarkGPU(d_a, d_b, d_c, n, blockSize);
        
        printf("   Block size: %4d, Grid size: %6d, Time: %.6f s, Speedup: %.2fx\n", 
               blockSize, gridSize, gpu_time, cpu_time / gpu_time);
        
        if (gpu_time < bestTime) {
            bestTime = gpu_time;
            bestBlockSize = blockSize;
        }
    }
    
    printf("\n   Best kernel configuration: Block size %d (%.6f seconds, %.2fx speedup)\n\n", 
           bestBlockSize, bestTime, cpu_time / bestTime);
    
    // Test with memory transfers included
    printf("4. GPU Benchmarks - Including Memory Transfers\n");
    for (int i = 0; i < numBlockSizes; i++) {
        int blockSize = blockSizes[i];
        
        double gpu_time_with_mem = benchmarkGPUWithMemory(h_a, h_b, h_c_gpu, n, blockSize);
        
        printf("   Block size: %4d, Total time: %.6f s, Speedup: %.2fx\n", 
               blockSize, gpu_time_with_mem, cpu_time / gpu_time_with_mem);
    }
    printf("\n");
    
    // Run with best configuration and verify results
    printf("5. Result Verification\n");
    int bestGridSize = (n + bestBlockSize - 1) / bestBlockSize;
    vectorAdd<<<bestGridSize, bestBlockSize>>>(d_a, d_b, d_c, n);
    checkCudaError(cudaDeviceSynchronize(), "Final kernel execution");
    
    // Copy result back to host
    checkCudaError(cudaMemcpy(h_c_gpu, d_c, bytes, cudaMemcpyDeviceToHost), "Copying result to host");
    
    // Verify results
    if (verifyResults(h_c_cpu, h_c_gpu, n)) {
        printf("   ✓ Results verified successfully!\n\n");
    } else {
        printf("   ✗ Result verification failed!\n\n");
    }
    
    // Performance analysis
    printf("6. Performance Analysis\n");
    double bandwidth_cpu = (3.0 * bytes) / (cpu_time * 1e9); // GB/s
    double bandwidth_gpu = (3.0 * bytes) / (bestTime * 1e9); // GB/s
    printf("   CPU effective bandwidth: %.2f GB/s\n", bandwidth_cpu);
    printf("   GPU effective bandwidth: %.2f GB/s\n", bandwidth_gpu);
    printf("   Bandwidth improvement: %.2fx\n\n", bandwidth_gpu / bandwidth_cpu);
    
    // GPU device information
    printf("7. GPU Device Information\n");
    cudaDeviceProp prop;
    checkCudaError(cudaGetDeviceProperties(&prop, 0), "Getting device properties");
    printf("   Device: %s\n", prop.name);
    printf("   Compute capability: %d.%d\n", prop.major, prop.minor);
    printf("   Max threads per block: %d\n", prop.maxThreadsPerBlock);
    printf("   Max grid size: %d x %d x %d\n", prop.maxGridSize[0], prop.maxGridSize[1], prop.maxGridSize[2]);
    printf("   Memory bandwidth: %.2f GB/s\n", 2.0 * prop.memoryClockRate * (prop.memoryBusWidth / 8) / 1.0e6);
    
    // Cleanup
    free(h_a);
    free(h_b);
    free(h_c_cpu);
    free(h_c_gpu);
    
    checkCudaError(cudaFree(d_a), "Freeing device memory a");
    checkCudaError(cudaFree(d_b), "Freeing device memory b");
    checkCudaError(cudaFree(d_c), "Freeing device memory c");
    
    printf("\nTutorial completed successfully!\n");
    return 0;
}
