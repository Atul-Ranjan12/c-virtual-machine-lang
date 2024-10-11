#ifndef vm_value_h
#define vm_value_h

#include "../commons/common.h"

// Represents a constant value
typedef double Value;

// Represents an array of value
typedef struct ValueArray {
    int capacity;
    int count;
    Value* values;
} ValueArray;

// Value array functions
void initValueArray(ValueArray* va);
void writeValueArray(ValueArray* va, Value v);
void freeValueArray(ValueArray* va);
void printValue(Value value);

#endif
