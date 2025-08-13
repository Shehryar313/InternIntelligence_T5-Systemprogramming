#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define POOL_SIZE 2048

// This program implements a simple memory management system with a fixed-size memory pool.
// It supports dynamic memory allocation (malloc) and deallocation (free) with features like best-fit allocation,
// merging adjacent free blocks, and memory leak detection. The memory pool is initialized at the start,
// and the program provides functions to allocate and free memory, print the current memory layout, and check for memory leaks.

typedef struct Block {
    size_t size;
    int free;
    int id;  // unique ID for debugging
    struct Block *next;
} Block;

// Memory pool
char memory_pool[POOL_SIZE];
Block *free_list = (Block *)memory_pool;
int current_id = 1;  // For debugging and tracking

//  Initialize memory pool
void initialize_memory() {
    free_list->size = POOL_SIZE - sizeof(Block);
    free_list->free = 1;
    free_list->next = NULL;
    free_list->id = current_id++;
}
// function to merge adjacent free blocks
void merge_blocks() {
    Block *curr = free_list;
    // Traverse the free list and merge adjacent free blocks
    if (!curr) return;  // No blocks to merge
    // Start with the first block
    while (curr && curr->next) {
        if (curr->free && curr->next->free) {
            curr->size += sizeof(Block) + curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}

// function to find the best-fit block for allocation
Block *find_best_fit(size_t size) {
    // Find the best-fit block for the requested size
    Block *best = NULL;
    // Traverse the free list to find the best fit
    Block *curr = free_list;
    while (curr) {
        if (curr->free && curr->size >= size) {
            // If this block is free and large enough, check if it's the best fit
            if (!best || curr->size < best->size) {
                best = curr;
            }
        }
        curr = curr->next;
    }
    return best;
}

// function to split a block into two parts
void split_block(Block *block, size_t size) {
    // Split the block into two: one for the requested size and another for the remaining free space
    Block *new_block = (Block *)((char *)block + sizeof(Block) + size);
    new_block->size = block->size - size - sizeof(Block);
    new_block->free = 1;
    new_block->next = block->next;
    new_block->id = current_id++;
    // Update the original block
    block->size = size;
    block->free = 0;
    block->next = new_block;
}

// function to allocate memory
void *my_malloc(size_t size) {
    Block *fit = find_best_fit(size);
    // Check if a suitable block was found
    if (!fit) {
        printf("ERROR: Out of memory.\n");
        return NULL;
    }
    // Check if the requested size is valid
    if (fit->size > size + sizeof(Block)) {
        split_block(fit, size);
    } else {
        fit->free = 0;
    }
    // Return a pointer to the allocated memory
    return (char *)fit + sizeof(Block);
}
// function to free allocated memory
void my_free(void *ptr) {
    if (!ptr) return;
    // Check if the pointer is valid
    Block *block = (Block *)((char *)ptr - sizeof(Block));
    // Validate the pointer
    if (block < (Block *)memory_pool || (char *)block >= memory_pool + POOL_SIZE) {
        printf("WARNING: Invalid free detected!\n");
        return;
    }
    // Mark the block as free
    block->free = 1;
    merge_blocks();
}

// function to print the current memory layout
void print_memory() {
    Block *curr = free_list;
    printf("\n--- Memory Layout ---\n");
    while (curr) {
        printf("[ID %2d] %4zu bytes | %s\n", curr->id, curr->size, curr->free ? "Free" : "Used");
        curr = curr->next;
    }
    printf("----------------------\n");
}
// function to check for memory leaks
void check_leaks() {
    Block *curr = free_list;
    int leaks = 0;
    // Traverse the free list to check for leaks
    while (curr) {
        if (!curr->free) {
            printf("LEAK DETECTED: Block ID %d (%zu bytes) not freed.\n", curr->id, curr->size);
            leaks++;
        }
        curr = curr->next;
    }
    if (!leaks) {
        printf(" No memory leaks.\n");
    }
}
// function to test memory leak detection
void memory_leak_test() {
    initialize_memory(); // Reset pool
    printf("Memory Leak Test\n");
    printf("=================\n");
    printf("Initializing memory pool...\n");
    initialize_memory();
    print_memory();  // Initial state
    printf("Running memory leak test...\n");
    
    void *a = my_malloc(128);
    void *b = my_malloc(256);
    void *c = my_malloc(64);

    print_memory();  // Before freeing

    my_free(a);
    my_free(b);

    print_memory();  // After freeing


    check_leaks();   // Should report no leaks
    my_free(c);
    print_memory();  // Final state
    check_leaks();   // Should report no leaks
}


int main() {
   
    // Run the memory leak test
    memory_leak_test();
    return 0;
}

