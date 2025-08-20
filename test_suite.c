#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "my_allocator.h"

#define NUM_ALLOCS 10000

// Basic tests: testing my_malloc, my_calloc, my_realloc, and my_free.
void basic_tests() {
    printf("Starting basic tests...\n");

    char *ptr1 = (char*)my_malloc(128);
    if(ptr1) {
        strcpy(ptr1, "Basic test with custom allocator.");
        printf("ptr1: %s\n", ptr1);
    } else {
        printf("my_malloc failed for ptr1.\n");
    }

    int *ptr2 = (int*)my_calloc(10, sizeof(int));
    if(ptr2) {
        printf("ptr2 allocated with calloc, first element = %d\n", ptr2[0]);
    } else {
        printf("my_calloc failed for ptr2.\n");
    }

    ptr1 = (char*)my_realloc(ptr1, 256);
    if(ptr1) {
        strcat(ptr1, " Reallocation successful.");
        printf("ptr1 after realloc: %s\n", ptr1);
    } else {
        printf("my_realloc failed for ptr1.\n");
    }

    my_free(ptr1);
    my_free(ptr2);
    printf("Basic tests completed.\n\n");
}

// Stress test: allocates many random blocks and frees them randomly.
void stress_test() {
    printf("Starting stress test...\n");
    void *allocations[NUM_ALLOCS];
    for (int i = 0; i < NUM_ALLOCS; i++) {
        size_t size = (rand() % 249) + 8;  // Allocate sizes between 8 and 256 bytes.
        allocations[i] = my_malloc(size);
        if(allocations[i] == NULL) {
            printf("my_malloc failed at iteration %d, size = %zu\n", i, size);
        }
    }

    // Randomly free some of the allocated blocks.
    for (int i = 0; i < NUM_ALLOCS; i++) {
        if(rand() % 2) {
            my_free(allocations[i]);
            allocations[i] = NULL;
        }
    }

    // Free the remaining allocated blocks.
    for (int i = 0; i < NUM_ALLOCS; i++) {
        if(allocations[i])
            my_free(allocations[i]);
    }
    printf("Stress test completed.\n\n");
}

// Performance test: measures the time for a series of malloc/free operations.
void performance_test() {
    printf("Starting performance test...\n");
    int iterations = 10000000;
    void *temp;
    clock_t start, end;
    double cpu_time_used;

    start = clock();
    for (int i = 0; i < iterations; i++) {
        temp = my_malloc(64);
        my_free(temp);
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("%d malloc/free operations took %f seconds\n", iterations, cpu_time_used);
    printf("Performance test completed.\n\n");
}

int main() {
    srand(time(NULL));
    printf("Starting Custom Allocator Test Suite...\n\n");

    basic_tests();
    stress_test();
    performance_test();

    printf("All tests completed.\n");
    return 0;
}
