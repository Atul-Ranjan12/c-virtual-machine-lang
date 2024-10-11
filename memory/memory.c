#include <stddef.h>
#include <stdlib.h>
#include "memory.h"

// Reallocate function handles reallocation of
// memory
void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    // If the newsize is 0, we free the pointer
    if (newSize == 0) {
        free(pointer);
        return NULL;
    }
    // Copies the old to the new block after
    // allocating it, and then returns the pointer
    // to the new block
    void* result = realloc(pointer, newSize);

    // Handle if reallocation is not possible
    // due to insufficient memory
    if (result == NULL) exit(1);

    return result;
}
