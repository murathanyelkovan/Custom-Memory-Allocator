#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "my_allocator.h"

// Structure to hold metadata for each memory block.
typedef struct block_meta {
    size_t size;             // Size of the data block.
    int free;                // 0 if allocated, 1 if free.
    struct block_meta *next; // Pointer to the next block.
} block_meta;

#define META_SIZE sizeof(block_meta)

// Global pointer to the beginning of the memory blocks list.
static block_meta *global_base = NULL;

// For the Next Fit strategy: pointer to the last examined block.
static block_meta *last_alloc = NULL;

#ifndef ALLOC_STRATEGY
#define ALLOC_STRATEGY 1
#endif

// Extend the heap using sbrk() and create a new block.
block_meta *extend_heap(block_meta *last, size_t size) {
    block_meta *block;
    block = sbrk(0);
    void *request = sbrk(size + META_SIZE);
    if (request == (void*)-1)
        return NULL; // sbrk failed.
    block->size = size;
    block->free = 0;
    block->next = NULL;
    if (last)
        last->next = block;
    return block;
}

// If the found block is significantly larger than needed, split it.
void split_block(block_meta *block, size_t size) {
    if (block->size >= size + META_SIZE + 8) { // Ensure a minimum block size for the remainder.
        block_meta *new_block = (block_meta*)((char*)block + META_SIZE + size);
        new_block->size = block->size - size - META_SIZE;
        new_block->free = 1;
        new_block->next = block->next;
        block->size = size;
        block->next = new_block;
    }
}

// Merge adjacent free blocks to reduce fragmentation.
void merge_free_blocks() {
    block_meta *current = global_base;
    while (current && current->next) {
        if (current->free && current->next->free) {
            current->size += META_SIZE + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

// First Fit: Returns the first free block that is large enough.
block_meta *find_free_block_first_fit(block_meta **last, size_t size) {
    block_meta *current = global_base;
    while (current) {
        if (current->free && current->size >= size)
            return current;
        *last = current;
        current = current->next;
    }
    return NULL;
}

// Best Fit: Returns the smallest free block that is large enough.
block_meta *find_free_block_best_fit(block_meta **last, size_t size) {
    block_meta *current = global_base;
    block_meta *best = NULL;
    while (current) {
        if (current->free && current->size >= size) {
            if (best == NULL || current->size < best->size)
                best = current;
        }
        *last = current;
        current = current->next;
    }
    return best;
}

// Next Fit: Starts searching from the last used block.
block_meta *find_free_block_next_fit(block_meta **last, size_t size) {
    if (!global_base)
        return NULL;
    block_meta *current = last_alloc ? last_alloc : global_base;
    block_meta *start = current;
    do {
        if (current->free && current->size >= size) {
            last_alloc = current->next;
            return current;
        }
        current = current->next ? current->next : global_base;
    } while (current != start);
    *last = current;
    return NULL;
}

// Custom malloc implementation.
void *my_malloc(size_t size) {
    if (size <= 0)
        return NULL;

    block_meta *block;
    block_meta *last = global_base;

    // Choose the allocation strategy based on the ALLOC_STRATEGY flag.
#if ALLOC_STRATEGY == 1
    block = find_free_block_first_fit(&last, size);
#elif ALLOC_STRATEGY == 2
    block = find_free_block_best_fit(&last, size);
#elif ALLOC_STRATEGY == 3
    block = find_free_block_next_fit(&last, size);
#else
    block = find_free_block_first_fit(&last, size);
#endif

    // If a suitable free block is found, mark it allocated and split it if necessary.
    if (block) {
        block->free = 0;
        split_block(block, size);
    } else {
        // No fitting block found; extend the heap.
        if (global_base == NULL) {
            block = extend_heap(NULL, size);
            global_base = block;
            if (!block)
                return NULL;
        } else {
            block = extend_heap(last, size);
            if (!block)
                return NULL;
        }
    }
    return (void*)(block + 1); // Return pointer to the data area.
}

// Custom free implementation.
void my_free(void *ptr) {
    if (!ptr)
        return;
    // Retrieve the block's metadata.
    block_meta *block_ptr = (block_meta*)ptr - 1;
    block_ptr->free = 1;
    merge_free_blocks();
}

// Custom calloc implementation.
void *my_calloc(size_t nmemb, size_t size) {
    size_t total_size = nmemb * size;
    void *ptr = my_malloc(total_size);
    if (ptr)
        memset(ptr, 0, total_size);
    return ptr;
}

// Custom realloc implementation.
void *my_realloc(void *ptr, size_t size) {
    if (!ptr)
        return my_malloc(size);
    block_meta *block_ptr = (block_meta*)ptr - 1;
    // If the current block is large enough, reuse it.
    if (block_ptr->size >= size) {
        split_block(block_ptr, size);
        return ptr;
    } else {
        // Allocate a new block, copy the data, and free the old block.
        void *new_ptr = my_malloc(size);
        if (!new_ptr)
            return NULL;
        memcpy(new_ptr, ptr, block_ptr->size);
        my_free(ptr);
        return new_ptr;
    }
}
