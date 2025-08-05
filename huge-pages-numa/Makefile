# we link against lrt (librt) for high-resolution timers
all:
	gcc -O2 -o tlb_benchmark tlb_benchmark.c -lrt
	gcc -O2 -o tlb_benchmark_4k tlb_benchmark_4k.c -lrt
	gcc -O2 -o tlb_benchmark_2mb tlb_benchmark_2mb.c -lrt
	gcc -o pointer_chase_benchmark pointer_chase_benchmark.c -lrt
	gcc -o parallel_access_benchmark parallel_access_benchmark.c -lrt
	gcc -o numa_controlled_benchmark numa_controlled_benchmark.c -lnuma -lrt
