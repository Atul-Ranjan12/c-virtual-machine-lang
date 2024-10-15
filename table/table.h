#ifndef vm_table_h
#define vm_table_h
#include "../object/object.h"

typedef struct Entry {
  ObjString *key;
  Value value;
} Entry;

typedef struct Table {
  int count;
  int capacity;
  Entry *entries; // Array of entries
} Table;

void initTable(Table *tb);
void freeTable(Table *tb);
bool tableSet(Table *table, ObjString *key, Value value);
bool tableGet(Table *tb, ObjString *key, Value *value);
bool tableDelete(Table *tb, ObjString *key);
ObjString *tableFindString(Table *table, const char *chars, int length,
                           uint32_t hash);

#endif
