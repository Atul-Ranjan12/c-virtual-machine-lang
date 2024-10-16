#include "object.h"
#include "../memory/memory.h"
#include "../virtual_machine/vm.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Write a hash function to store a hash
// hashString uses the FNN-1a algorithm to hash
// strings
static uint32_t hashString(const char *key, int length) {
  uint32_t hash = 2166136261u;
  for (int i = 0; i < length; i++) {
    hash ^= (uint8_t)key[i];
    hash *= 16777619;
  }
  return hash;
}

// Used to allocate an object to the memory
static Obj *allocateObj(size_t size, ObjType type) {
  Obj *object = (Obj *)reallocate(NULL, 0, size);
  object->type = type;
  object->next = vm.objects;
  vm.objects = object;
  return object;
}

// Used to allocate an object type to the memory
#define ALLOCATE_OBJ(type, objectType)                                         \
  (type *)allocateObj(sizeof(type), objectType)

// Gets the allocated space as chars and allocates
// it as an object string
static ObjString *allocateString(char *chars, int length, uint32_t hash) {
  ObjString *objectString = ALLOCATE_OBJ(ObjString, OBJ_STRING);
  objectString->length = length;
  objectString->chars = chars;
  objectString->hash = hash;

  // Set the value of the string in the table
  tableSet(&vm.strings, objectString, NIL_VAL);
  return objectString;
}

// copyString copies the recieved chars into an object
ObjString *copyString(const char *chars, int length) {
  // Hash the string into its hash
  uint32_t hash = hashString(chars, length);
  // Check if we are already storing the string
  // in our strings table, if yes, we just return
  // the intered string
  ObjString *interned = tableFindString(&vm.strings, chars, length, hash);
  if (interned != NULL)
    return interned;
  // Allocate memory to the heap and add final characteer
  char *heapChars = ALLOCATE(char, length + 1);
  memcpy(heapChars, chars, length);
  heapChars[length] = '\0';

  // Return the string at the end after allocating
  // it as on object
  return allocateString(heapChars, length, hash);
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
  uint32_t hash = hashString(chars, length);
  ObjString *interned = tableFindString(&vm.strings, chars, length, hash);
  if (interned != NULL) {
    FREE_ARRAY(char, chars, length + 1);
    return interned;
  }
  return allocateString(chars, length, hash);
}
