#include <stdio.h>
#include <string.h>
#include "my_allocator.h"

int main() {
    printf("Custom Allocator Test Suite\n");

    // Test my_malloc.
    char *ptr1 = (char*)my_malloc(100);
    if (ptr1) {
        strcpy(ptr1, "Hello, custom allocator!");
        printf("ptr1: %s\n", ptr1);
    } else {
        printf("my_malloc failed to allocate memory.\n");
    }

    // Test my_calloc.
    int *ptr2 = (int*)my_calloc(10, sizeof(int));
    if (ptr2) {
        printf("ptr2: First element = %d\n", ptr2[0]);
    } else {
        printf("my_calloc failed to allocate memory.\n");
    }

    // Test my_realloc.
    ptr1 = (char*)my_realloc(ptr1, 200);
    if (ptr1) {
        strcat(ptr1, " Reallocation worked.");
        printf("ptr1 after realloc: %s\n", ptr1);
    } else {
        printf("my_realloc failed to reallocate memory.\n");
    }

    // Free allocated memory.
    my_free(ptr1);
    my_free(ptr2);
    printf("Memory freed successfully.\n");

    return 0;
}
