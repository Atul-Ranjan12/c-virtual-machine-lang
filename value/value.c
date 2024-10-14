#include "value.h"
#include "../memory/memory.h"
#include "../object/object.h"
#include <stdio.h>
#include <stdlib.h>

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

void freeValueArray(ValueArray *va) {
  FREE_ARRAY(Value, va->values, va->capacity);
  initValueArray(va);
}

void printValue(Value value) {
  switch (value.type) {
  case (VAL_NIL):
    printf("nil\n");
    break;
  case (VAL_BOOL):
    printf(AS_BOOL(value) ? "true\n" : "false\n");
    break;
  case (VAL_NUMBER):
    printf("%g\n", AS_NUMBER(value));
    break;
  case (VAL_OBJ):
    printObject(value);
    break;
  default: // Unreachable
    return;
  }
}
