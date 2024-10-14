#ifndef vm_value_h
#define vm_value_h

#include "../commons/common.h"

// // Represents a constant value
// typedef double Value;

// ValueType stores the type of the value
typedef enum ValueType {
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER,
  VAL_OBJ,
} ValueType;

// New representation of value
typedef struct Value {
  ValueType type;
  union {
    bool boolean;
    double number;
    // Adding a type for the object
  } as;
} Value;

// Macros to check if the value is of a particular type
#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)

// Uncasting macros
#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)

// Casting definitions
#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})

// Represents an array of value
typedef struct ValueArray {
  int capacity;
  int count;
  Value *values;
} ValueArray;

// Value array functions
void initValueArray(ValueArray *va);
void writeValueArray(ValueArray *va, Value v);
void freeValueArray(ValueArray *va);
void printValue(Value value);

#endif
