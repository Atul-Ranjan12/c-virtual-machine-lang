#include <stdio.h>
#include <stdlib.h>
#include "value.h"
#include "../memory/memory.h"

void initValueArray(ValueArray *va) {
    va->capacity = 0;
    va->values = NULL;
    va->count = 0;
}

void writeValueArray(ValueArray *va, Value v) {
    if (va->count + 1 > va->capacity) {
        int oldCapacity = va->capacity;
        va->capacity = GROW_CAPACITY(oldCapacity);
        va->values = GROW_ARRAY(Value, va->values, oldCapacity, va->capacity);
    }

    va->values[va->count] = v;
    va->count++;
}

void freeValueArray(ValueArray *va){
    FREE_ARRAY(Value, va->values, va->capacity);
    initValueArray(va);
}

void printValue(Value value){
    printf("%g", value);
}
