#ifndef vm_object_h
#define vm_object_h

#include "../commons/common.h"
#include "../value/value.h"
#include <stdint.h>

typedef enum ObjType { OBJ_STRING } ObjType;

typedef struct Obj {
  ObjType type;
  Obj *next;
} Obj;

typedef struct ObjString {
  Obj obj;
  int length;
  char *chars;
  uint32_t hash;
} ObjString;

// Because obj is of type Obj :: it is safe to cast ObjString*
// to Obj* and then access its type. We can also similarly
// cast Obj* to ObjString* after ensuring the rest of the fields
// are present

#define OBJ_TYPE (value)(AS_OBJ(value)->type)
#define IS_STRING(value) isObjectType(value, OBJ_STRING)

// isObjectType returns if an object is of a particular type
static inline bool isObjectType(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

// Macros to cast an object to a string
#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value))->chars)

ObjString *copyString(const char *chars, int length);
void printObject(Value value);
ObjString *takeString(char *chars, int length);

#endif
