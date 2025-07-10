# CUDA Vector Addition Makefile
# Usage: make [target]

# Compiler and flags
NVCC = nvcc
CFLAGS = -O2 -std=c++11
CUDA_ARCH = -arch=sm_90

# Target executable
TARGET = vector_add

# Source files
SOURCES = vector_add.cu

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(SOURCES)
	$(NVCC) $(CFLAGS) $(CUDA_ARCH) -o $(TARGET) $(SOURCES)

warp: warp.cu
	$(NVCC) $(CFLAGS) $(CUDA_ARCH) -o warp warp.cu

# Run the program
run: $(TARGET)
	./$(TARGET)

# Clean build artifacts
clean:
	rm -f $(TARGET)

# Check CUDA installation
check:
	@echo "Checking CUDA installation..."
	@nvcc --version
	@echo "\nChecking GPU devices..."
	@nvidia-smi

# Help target
help:
	@echo "Available targets:"
	@echo "  all     - Build the executable (default)"
	@echo "  run     - Build and run the program"
	@echo "  clean   - Remove build artifacts"
	@echo "  check   - Check CUDA installation"
	@echo "  help    - Show this help message"

.PHONY: all run clean check help
