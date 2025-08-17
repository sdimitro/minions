#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

// Represents a single line within a cache set
typedef struct {
    bool valid;         // Is this line holding valid data?
    uint32_t tag;       // The tag portion of the address
    int lru_counter;    // Counter for LRU replacement policy
} CacheLine;

// Represents the entire cache structure
typedef struct {
    // Configuration
    int cache_size;     // Total size in bytes
    int block_size;     // Size of each block/line in bytes
    int associativity;  // Number of lines per set (N-way)

    // Derived properties
    int num_lines;      // Total number of lines in the cache
    int num_sets;       // Total number of sets in the cache
    int tag_bits;       // Number of bits for the tag
    int index_bits;     // Number of bits for the index
    int offset_bits;    // Number of bits for the offset

    // Data storage
    CacheLine** sets;   // A 2D array: sets[index][way]

    // Statistics
    int hits;
    int misses;
} Cache;

// Function to calculate log base 2
int log2_int(int n) {
    if (n <= 0) return 0;
    // Using the property log2(n) = log(n) / log(2)
    return (int)(log(n) / log(2));
}

// Initialize the cache simulator
Cache* create_cache(int cache_size, int block_size, int associativity) {
    Cache* cache = (Cache*)malloc(sizeof(Cache));
    if (!cache) return NULL;

    cache->cache_size = cache_size;
    cache->block_size = block_size;
    cache->associativity = associativity;

    cache->num_lines = cache_size / block_size;
    if (associativity == 0) { // Special case for fully associative
        cache->associativity = cache->num_lines;
        cache->num_sets = 1;
    } else {
        cache->num_sets = cache->num_lines / associativity;
    }

    cache->offset_bits = log2_int(block_size);
    cache->index_bits = log2_int(cache->num_sets);
    cache->tag_bits = 32 - cache->index_bits - cache->offset_bits; // Assuming 32-bit addresses

    cache->hits = 0;
    cache->misses = 0;

    // Allocate memory for the sets (the rows of our 2D cache array)
    cache->sets = (CacheLine**)malloc(cache->num_sets * sizeof(CacheLine*));
    for (int i = 0; i < cache->num_sets; i++) {
        // Allocate memory for the lines/ways within each set (the columns)
        cache->sets[i] = (CacheLine*)malloc(cache->associativity * sizeof(CacheLine));
        // Initialize all lines to be invalid
        for (int j = 0; j < cache->associativity; j++) {
            cache->sets[i][j].valid = false;
            cache->sets[i][j].tag = 0;
            cache->sets[i][j].lru_counter = 0;
        }
    }

    printf("--- Cache Configuration ---\n");
    printf("Cache Size: %d bytes\n", cache->cache_size);
    printf("Block Size: %d bytes\n", cache->block_size);
    if (cache->num_sets == 1 && cache->associativity > 1) {
        printf("Associativity: Fully Associative (%d-way)\n", cache->associativity);
    } else if (cache->associativity == 1) {
        printf("Associativity: Direct-Mapped\n");
    }
    else {
        printf("Associativity: %d-way Set-Associative\n", cache->associativity);
    }
    printf("Sets: %d, Lines per Set: %d\n", cache->num_sets, cache->associativity);
    printf("Total Lines: %d\n", cache->num_lines);
    printf("Tag bits: %d, Index bits: %d, Offset bits: %d\n\n", cache->tag_bits, cache->index_bits, cache->offset_bits);

    return cache;
}

// Free all allocated memory to prevent memory leaks
void destroy_cache(Cache* cache) {
    for (int i = 0; i < cache->num_sets; i++) {
        free(cache->sets[i]);
    }
    free(cache->sets);
    free(cache);
}

// Simulate a memory access and update the cache state
void cache_access(Cache* cache, uint32_t address) {
    // Calculate masks to extract parts of the address
    uint32_t offset_mask = (1 << cache->offset_bits) - 1;
    uint32_t index_mask = (1 << cache->index_bits) - 1;

    // Extract tag, index, and offset from the address
    uint32_t offset = address & offset_mask;
    uint32_t index = (address >> cache->offset_bits) & index_mask;
    uint32_t tag = address >> (cache->offset_bits + cache->index_bits);

    printf("--- Accessing Address: %u (0x%x) ---\n", address, address);
    printf("  Tag: 0x%x, Index: %u, Offset: %u\n", tag, index, offset);

    CacheLine* target_set = cache->sets[index];
    int hit_way = -1;

    // 1. Check for a hit by comparing tags in the target set
    for (int i = 0; i < cache->associativity; i++) {
        if (target_set[i].valid && target_set[i].tag == tag) {
            hit_way = i;
            cache->hits++;
            printf("  -> HIT in Set %u, Way %d\n", index, i);
            break;
        }
    }

    // 2. Handle a miss
    if (hit_way == -1) {
        cache->misses++;
        printf("  -> MISS in Set %u\n", index);

        // Find an empty way or the least recently used (LRU) way to replace
        int empty_way = -1;
        int lru_way = 0;
        int max_lru = -1;

        for (int i = 0; i < cache->associativity; i++) {
            // Prefer an empty slot if one exists
            if (!target_set[i].valid) {
                empty_way = i;
                break;
            }
            // Track the line with the largest LRU counter (oldest)
            if (target_set[i].lru_counter > max_lru) {
                max_lru = target_set[i].lru_counter;
                lru_way = i;
            }
        }

        if (empty_way != -1) {
            // Place the new block in the empty way
            hit_way = empty_way;
            printf("  -> Placing in empty Way %d\n", hit_way);
        } else {
            // Evict the LRU way
            hit_way = lru_way;
            printf("  -> Evicting Way %d (Tag 0x%x), Placing new block\n", hit_way, target_set[hit_way].tag);
        }
        target_set[hit_way].valid = true;
        target_set[hit_way].tag = tag;
    }

    // 3. Update LRU counters for the accessed set
    // Increment all valid counters, then reset the most recently used one to 0
    for (int i = 0; i < cache->associativity; i++) {
        if (target_set[i].valid) {
            target_set[i].lru_counter++;
        }
    }
    if (hit_way != -1) {
        target_set[hit_way].lru_counter = 0;
    }

    // Print the current state of the set for visualization
    printf("  Set %u State: ", index);
    for (int i = 0; i < cache->associativity; i++) {
        if (target_set[i].valid) {
            printf("[Way %d: Tag=0x%x, LRU=%d] ", i, target_set[i].tag, target_set[i].lru_counter);
        } else {
            printf("[Way %d: Empty] ", i);
        }
    }
    printf("\n\n");
}

int main(int argc, char* argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <cache_size_bytes> <block_size_bytes> <associativity> <addr1> <addr2> ...\n", argv[0]);
        fprintf(stderr, "  - associativity: 1 for direct-mapped, 0 for fully associative, N for N-way\n");
        fprintf(stderr, "Example: %s 1024 64 2 0 4096 64 4160\n", argv[0]);
        return 1;
    }

    int cache_size = atoi(argv[1]);
    int block_size = atoi(argv[2]);
    int associativity = atoi(argv[3]);

    Cache* my_cache = create_cache(cache_size, block_size, associativity);
    if (!my_cache) {
        fprintf(stderr, "Failed to create cache.\n");
        return 1;
    }

    // Process each memory address provided on the command line
    for (int i = 4; i < argc; i++) {
        // strtoul allows parsing hex (0x...) or decimal addresses
        uint32_t addr = (uint32_t)strtoul(argv[i], NULL, 0);
        cache_access(my_cache, addr);
    }

    int total = my_cache->hits + my_cache->misses;
    printf("--- Final Statistics ---\n");
    printf("Total Accesses: %d\n", total);
    printf("Hits: %d\n", my_cache->hits);
    printf("Misses: %d\n", my_cache->misses);
    if (total > 0) {
        printf("Hit Rate: %.2f%%\n", (double)my_cache->hits / total * 100.0);
    }
    printf("------------------------\n");

    destroy_cache(my_cache);
    return 0;
}

