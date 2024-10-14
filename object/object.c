#include "object.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../memory/memory.h"
#include "../virtual_machine/vm.h"


static Obj *allocateObj(size_t size, ObjType type) {
  Obj *object = (Obj *)reallocate(NULL, 0, size);
  object->type = type;
  object->next = vm.objects;
  vm.objects = object;
  return object;
}

#define ALLOCATE_OBJ(type, objectType)                                         \
  (type *)allocateObj(sizeof(type), objectType)

static ObjString *allocateString(char *chars, int length) {
  ObjString *objectString = ALLOCATE_OBJ(ObjString, OBJ_STRING);
  objectString->length = length;
  objectString->chars = chars;

  return objectString;
}

// copyString copies the recieved chars into an object
ObjString *copyString(const char *chars, int length) {
  // Allocate memory to the heap
  char *heapChars = ALLOCATE(char, length + 1);
  memcpy(heapChars, chars, length);
  heapChars[length] = '\0';

  return allocateString(heapChars, length);
}

// printObject handles the printing of an object
void printObject(Value value) {
  switch (AS_OBJ(value)->type) {
  case OBJ_STRING:
    printf("%s \n", AS_CSTRING(value));
    break;
  default:
    break;
  }
}

// takeString allocates a string and returns it
ObjString *takeString(char *chars, int length) {
  return allocateString(chars, length);
}
