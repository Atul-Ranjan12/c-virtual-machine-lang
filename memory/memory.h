#ifndef vm_memory_h
#define vm_memory_h

#include "../commons/common.h"
#include <stddef.h>

// Macro to grow the capacity of an array
#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)


// Macro to grow the size of an array
#define GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type*)reallocate(pointer, sizeof(type) * (oldCount), \
        sizeof(type) * (newCount))


// Macro to free the array
#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type) * (oldCount), 0)


// The reallocate function
void* reallocate(void* pointer, size_t oldSize, size_t newSize);

#endif
