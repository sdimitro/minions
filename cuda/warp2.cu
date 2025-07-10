#include <cuda_runtime.h>
#include <device_launch_parameters.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// Compute-intensive kernel: Matrix-vector multiplication with expensive operations
__global__ void computeIntensiveKernel(float *input, float *output, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    
    if (idx < n) {
        float result = input[idx];
        
        // Perform many expensive operations to make it compute-bound
        for (int i = 0; i < 1000; i++) {
            result = sinf(result) * cosf(result) + sqrtf(fabsf(result));
            result = powf(result, 0.9f) + logf(fabsf(result) + 1.0f);
            result = expf(result * 0.001f) - tanhf(result * 0.01f);
        }
        
        output[idx] = result;
    }
}

// CPU version of the same computation
void computeIntensiveCPU(float *input, float *output, int n) {
    for (int i = 0; i < n; i++) {
        float result = input[i];
        
        for (int j = 0; j < 1000; j++) {
            result = sinf(result) * cosf(result) + sqrtf(fabsf(result));
            result = powf(result, 0.9f) + logf(fabsf(result) + 1.0f);
            result = expf(result * 0.001f) - tanhf(result * 0.01f);
        }
        
        output[i] = result;
    }
}

// Simple kernel that shows warp divergence effects
__global__ void warpDivergenceKernel(float *input, float *output, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    
    if (idx < n) {
        float result = input[idx];
        
        // This creates warp divergence - different threads take different paths
        // Each path now performs the same amount of work as computeIntensiveKernel
        // A warp will execute both paths sequentially, highlighting the overhead.
        if (threadIdx.x % 2 == 0) {
            // Even threads do one type of computation
            for (int i = 0; i < 1000; i++) { // Increased iterations and operations
                result = sinf(result) * cosf(result) + sqrtf(fabsf(result));
                result = powf(result, 0.9f) + logf(fabsf(result) + 1.0f);
                result = expf(result * 0.001f) - tanhf(result * 0.01f);
            }
        } else {
            // Odd threads do different computation
            for (int i = 0; i < 1000; i++) { // Increased iterations and operations
                result = sinf(result) * cosf(result) + sqrtf(fabsf(result));
                result = powf(result, 0.9f) + logf(fabsf(result) + 1.0f);
                result = expf(result * 0.001f) - tanhf(result * 0.01f);
            }
        }
        
        output[idx] = result;
    }
}

// Dedicated kernel for simple memory-bound copy operation
__global__ void copyKernel(float *input, float *output, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n) {
        output[idx] = input[idx];
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
        vec[i] = 0.1f + (float)i * 0.001f; // Avoid problematic values for math functions
    }
}

// Modified benchmarkGPU to accept a function pointer to a __global__ kernel
// This function benchmarks ONLY the kernel execution time.
double benchmarkGPU(void (*kernel)(float*, float*, int), float *d_in, float *d_out, int n, int blockSize) {
    int gridSize = (n + blockSize - 1) / blockSize;
    
    cudaEvent_t start, stop;
    checkCudaError(cudaEventCreate(&start), "Creating start event");
    checkCudaError(cudaEventCreate(&stop), "Creating stop event");
    
    // Warm-up
    kernel<<<gridSize, blockSize>>>(d_in, d_out, n);
    checkCudaError(cudaDeviceSynchronize(), "Warmup synchronization");
    
    // Benchmark
    checkCudaError(cudaEventRecord(start), "Recording start event");
    kernel<<<gridSize, blockSize>>>(d_in, d_out, n);
    checkCudaError(cudaEventRecord(stop), "Recording stop event");
    checkCudaError(cudaEventSynchronize(stop), "Synchronizing stop event");
    
    float milliseconds = 0;
    checkCudaError(cudaEventElapsedTime(&milliseconds, start, stop), "Getting elapsed time");
    
    checkCudaError(cudaEventDestroy(start), "Destroying start event");
    checkCudaError(cudaEventDestroy(stop), "Destroying stop event");
    
    return milliseconds / 1000.0; // Return time in seconds
}

int main() {
    const int n = 100000; // Smaller size for compute-intensive operations
    const size_t bytes = n * sizeof(float);
    
    printf("CUDA Warp Efficiency Demonstration\n");
    printf("Problem size: %d elements\n", n);
    printf("===============================================\n\n");
    
    // Allocate host memory
    float *h_input = (float*)malloc(bytes);
    float *h_output_cpu = (float*)malloc(bytes);
    float *h_output_gpu = (float*)malloc(bytes);
    
    initializeVector(h_input, n);
    
    // CPU benchmark
    printf("1. CPU Compute-Intensive Benchmark\n");
    clock_t cpu_start = clock();
    computeIntensiveCPU(h_input, h_output_cpu, n);
    clock_t cpu_end = clock();
    double cpu_time = ((double)(cpu_end - cpu_start)) / CLOCKS_PER_SEC;
    printf("    CPU time: %.4f seconds\n\n", cpu_time);
    
    // GPU setup
    float *d_input, *d_output;
    checkCudaError(cudaMalloc(&d_input, bytes), "Allocating device input");
    checkCudaError(cudaMalloc(&d_output, bytes), "Allocating device output");
    
    // Events for timing memory transfers
    cudaEvent_t start_h2d, stop_h2d;
    cudaEvent_t start_d2h, stop_d2h;
    checkCudaError(cudaEventCreate(&start_h2d), "Creating H2D start event");
    checkCudaError(cudaEventCreate(&stop_h2d), "Creating H2D stop event");
    checkCudaError(cudaEventCreate(&start_d2h), "Creating D2H start event");
    checkCudaError(cudaEventCreate(&stop_d2h), "Creating D2H stop event");

    // Copy input to device and time it (H2D)
    checkCudaError(cudaEventRecord(start_h2d), "Recording H2D start event");
    checkCudaError(cudaMemcpy(d_input, h_input, bytes, cudaMemcpyHostToDevice), "Copying input to device");
    checkCudaError(cudaEventRecord(stop_h2d), "Recording H2D stop event");
    checkCudaError(cudaEventSynchronize(stop_h2d), "Synchronizing H2D stop event");
    float h2d_milliseconds = 0;
    checkCudaError(cudaEventElapsedTime(&h2d_milliseconds, start_h2d, stop_h2d), "Getting H2D elapsed time");
    double h2d_time = h2d_milliseconds / 1000.0; // Declare h2d_time here, accessible globally in main
    
    // Variables to store kernel and D2H times for the specific 256 block size run
    double regular_kernel_time_256 = 0;
    double d2h_time_256 = 0;

    // Test compute-intensive kernel with different block sizes
    printf("2. GPU Compute-Intensive Kernel - Block Size Effects (including memory transfers)\n");
    int blockSizes[] = {31, 32, 63, 64, 127, 128, 255, 256, 511, 512};
    int numBlockSizes = sizeof(blockSizes) / sizeof(blockSizes[0]);
    
    for (int i = 0; i < numBlockSizes; i++) {
        int blockSize = blockSizes[i];
        
        // Benchmark kernel execution time only
        double kernel_exec_time = benchmarkGPU(computeIntensiveKernel, d_input, d_output, n, blockSize);
        
        // Copy output back to host and time it (D2H)
        checkCudaError(cudaEventRecord(start_d2h), "Recording D2H start event");
        checkCudaError(cudaMemcpy(h_output_gpu, d_output, bytes, cudaMemcpyDeviceToHost), "Copying output to host");
        checkCudaError(cudaEventRecord(stop_d2h), "Recording D2H stop event");
        checkCudaError(cudaEventSynchronize(stop_d2h), "Synchronizing D2H stop event");
        float d2h_milliseconds = 0;
        checkCudaError(cudaEventElapsedTime(&d2h_milliseconds, start_d2h, stop_d2h), "Getting D2H elapsed time");
        double d2h_time = d2h_milliseconds / 1000.0; // This d2h_time is local to the loop

        // Store the times for blockSize 256 for later use
        if (blockSize == 256) {
            regular_kernel_time_256 = kernel_exec_time;
            d2h_time_256 = d2h_time;
        }

        // Total GPU time including transfers
        double total_gpu_time = h2d_time + kernel_exec_time + d2h_time;
        
        bool isMultipleOf32 = (blockSize % 32 == 0);
        int wastedThreads = isMultipleOf32 ? 0 : (32 - (blockSize % 32));
        
        printf("    Block size: %3d, Kernel Time: %.4f s, H2D Time: %.4f s, D2H Time: %.4f s, Total GPU Time: %.4f s, Speedup: %5.1fx %s\n", 
               blockSize, kernel_exec_time, h2d_time, d2h_time, total_gpu_time, cpu_time / total_gpu_time,
               isMultipleOf32 ? "✓" : "✗");
        
        if (!isMultipleOf32) {
            printf("        └─ Wasted threads per partial warp: %d/32 (%.1f%% efficiency)\n", 
                   wastedThreads, ((32.0 - wastedThreads) / 32.0) * 100.0);
        }
    }
    
    printf("\n3. Warp Divergence Effects\n");
    printf("    Testing kernel with branching (warp divergence)...\n");
    
    // Compare regular vs divergent kernels (kernel execution time only)
    double regular_kernel_time = benchmarkGPU(computeIntensiveKernel, d_input, d_output, n, 256);
    double divergent_kernel_time = benchmarkGPU(warpDivergenceKernel, d_input, d_output, n, 256);
    
    printf("    Regular kernel (256 threads):    %.4f s\n", regular_kernel_time);
    printf("    Divergent kernel (256 threads):  %.4f s\n", divergent_kernel_time);
    printf("    Divergence overhead: %.1f%%\n\n", ((divergent_kernel_time - regular_kernel_time) / regular_kernel_time) * 100.0);
    
    // Memory bandwidth analysis
    printf("4. Memory Bandwidth Analysis\n");
    
    printf("    Testing memory-bound operation (simple copy)...\n");
    
    // Now calling the standalone __global__ copyKernel
    double copy_kernel_time = benchmarkGPU(copyKernel, d_input, d_output, n, 256);
    
    // Calculate effective bandwidth
    double bytes_transferred = 2.0 * bytes; // Read input, write output
    double bandwidth = bytes_transferred / (copy_kernel_time * 1e9); // GB/s
    
    printf("    Copy kernel time: %.6f s\n", copy_kernel_time);
    printf("    Effective bandwidth: %.2f GB/s\n", bandwidth);
    printf("    Compute-to-memory ratio: %.1fx\n\n", regular_kernel_time / copy_kernel_time);
    
    printf("5. Why Vector Addition Isn't Great for GPU\n");
    printf("    Vector addition characteristics:\n");
    printf("    - Memory bandwidth bound (not compute bound)\n");
    printf("    - Low arithmetic intensity (1 operation per 2 memory reads + 1 write)\n");
    printf("    - Memory transfer overhead dominates for small/medium arrays\n");
    printf("    - CPU cache hierarchy often more efficient for this pattern\n\n");
    
    printf("6. When GPU Acceleration Makes Sense\n");
    printf("    ✓ High arithmetic intensity (many operations per memory access)\n");
    printf("    ✓ Large datasets (amortize memory transfer costs)\n");
    printf("    ✓ Parallelizable algorithms (no dependencies between threads)\n");
    printf("    ✓ Regular memory access patterns (coalesced memory access)\n");
    printf("    ✓ Compute-bound operations (like our intensive kernel: %.1fx speedup, including transfers)\n\n", 
             cpu_time / (regular_kernel_time_256 + h2d_time + d2h_time_256)); // Use stored times for final speedup
    
    // Cleanup
    free(h_input);
    free(h_output_cpu);
    free(h_output_gpu);
    checkCudaError(cudaFree(d_input), "Freeing device input");
    checkCudaError(cudaFree(d_output), "Freeing device output");
    checkCudaError(cudaEventDestroy(start_h2d), "Destroying H2D start event");
    checkCudaError(cudaEventDestroy(stop_h2d), "Destroying H2D stop event");
    checkCudaError(cudaEventDestroy(start_d2h), "Destroying D2H start event");
    checkCudaError(cudaEventDestroy(stop_d2h), "Destroying D2H stop event");
    
    return 0;
}

